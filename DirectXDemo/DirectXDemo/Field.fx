//CONSTANT BUFFERS-----------------------------------------------------------------

cbuffer CONSTS : register(b0)
{
	float4	binormal;
	float3	wind;
	float	tessDensity;
	float	halfGrassWidth;
	float	time;
};

cbuffer CBViewProj : register (b1)
{
	matrix view_proj;
}

cbuffer CBLight : register (b2)
{
	float4	camera;//@change to float3s
	float4	light;	//@
	float	intensity;
}
//INPUT/OUTPUT STRUCTS-----------------------------------------------------------------
struct VS_INPUT
{
	//PER_VERTEX
	float3	pos			: SV_POSITION;			//position
	float3	binormal	: BINORMAL;				//@@To be used for per-vertex binormals soon
	float3	normal		: NORMAL;
	float	flexibility	: FLEX;					//multiplier for wind deformation
	//PER-INSTANCE
	matrix	world		: INSTANCE_WORLD;		//world transform matrix
	float3	location	: INSTANCE_LOCATION;	//position of grass in world for lighting calc etc
};

struct VS_OUTPUT
{
	float4	pos			: POSITION;
	float4	binormal	: BINORMAL;
	float3	normal		: NORMAL;
};

struct HS_CONSTANT_OUTPUT
{
	float edges[2] : SV_TessFactor;
};

struct HS_OUTPUT
{
	float4	cpoint		: CPOINT;
	float3	normal		: NORMAL;
};

struct DS_OUTPUT
{
	float4	position	: SV_Position;
	float3	normal		: NORMAL;
	float	tVal		: T_VAL;
};

struct PS_INPUT
{
	float4	pos			: SV_POSITION;
	float2	texcoord	: TEXCOORD0;
	float3	normal		: NORMAL;
	float3  lightVec	: TEXCOORD1;
	float3	viewVec		: TEXCOORD2;
};

const static float4 translationFrequency = float4(1.975, 0.793, 0.375, 0.193);

//VERTEX SHADER----------------------------------------------------------
//Orthoman & GPU gems
inline float4 smoothf(float4 x)
{
	return x * x * (3.0 - 2.0 * x);
}
inline float4 trianglef(float4 x)
{
	return abs(frac(x + 0.5) * 2.0 - 1.0);
}
inline float4 windForce(float3 p, float3 wind)
{
	// Compute the phase shift for the position p with respect to
	// the current wind strength and direction
	float phase = (time * length(wind)) + dot(wind, p);
	// Compute the four translation strengths.
	float4 ts = smoothf(trianglef(translationFrequency * phase));
	// Compute the mean of the four values and
	// return the translation vector.
	return float4(wind * dot(ts, 0.25), 0.0f);
}


VS_OUTPUT VS_Main(VS_INPUT vertex)
{
	VS_OUTPUT output;
	output.pos = float4(vertex.pos, 1.f);

	//Wind displacement [Orthomans technique]
	output.pos += (windForce(vertex.location, wind) * vertex.flexibility);

	//Normal
	output.normal = vertex.normal;//@this is crap
	/*@when better wind simulation is in, use twisting to manipulate normal*/

	/*@this didn't work :c*/
	//output.normal = cross(vertex.binormal, normalize(output.pos));
	//output.normal = mul(output.normal, vertex.world);
	//output.normal = normalize(output.normal);


	//World-View-Proj transformation
	matrix wvp = mul(vertex.world, view_proj);
	output.pos = mul(output.pos, wvp);


	return output;
}


//HULL-DOMAIN SHADERS----------------------------------------------------
//See http://gpuexperiments.blogspot.co.uk/2010/02/tessellation-example.html
HS_CONSTANT_OUTPUT HSConst()
{
	HS_CONSTANT_OUTPUT output;

	output.edges[0] = 1.0f;				// Detail factor
	output.edges[1] = tessDensity;		// tess density
	return output;
}


[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(4)]
[patchconstantfunc("HSConst")]
HS_OUTPUT HS_Main(InputPatch<VS_OUTPUT, 4> input, uint id : SV_OutputControlPointID)
{
	HS_OUTPUT output;
	output.cpoint = input[id].pos;
	output.normal = input[id].normal;
	return output;
}


[domain("isoline")]
DS_OUTPUT DS_Main(HS_CONSTANT_OUTPUT input, OutputPatch<HS_OUTPUT, 4> op, float2 uv : SV_DomainLocation)
{
	DS_OUTPUT output;

	/*Time*/
	output.tVal = uv.x;

	/*Position = evaluateBezier(output.tVal)*/
	output.position =	pow(1.0f - output.tVal, 3.0f) * op[0].cpoint + 3.0f * pow(1.0f - output.tVal, 2.0f) * 
						output.tVal * op[1].cpoint + 3.0f * (1.0f - output.tVal) * pow(output.tVal, 2.0f) * 
						op[2].cpoint + pow(output.tVal, 3.0f) * op[3].cpoint;

	/*Normal = evaluateBezier(output.tVal)*/
	output.normal =		pow(1.0f - output.tVal, 3.0f) * op[0].normal + 3.0f * pow(1.0f - output.tVal, 2.0f) *
						output.tVal * op[1].normal + 3.0f * (1.0f - output.tVal) * pow(output.tVal, 2.0f) *
						op[2].normal + pow(output.tVal, 3.0f) * op[3].normal;

	return output;
}


//GEOMETRY SHADER--------------------------------------------------------


[maxvertexcount(4)]
void GS_Main(line DS_OUTPUT input[2], inout TriangleStream<PS_INPUT> output)
{	
	/*for each point in the line segment*/
	for (uint i = 0; i < 2; i++)
	{
		/*create a vertex to output*/
		PS_INPUT element;

		/*texture coords*/
		element.texcoord = float2(0, input[i].tVal);

		/*calculate an offset to the input vertex
		that will be used to place the output vertex.*/
									/*reduce the width of the grass towards
									the tip in a parabolic fashion.*/
		float4 offset = binormal * (1 - (input[i].tVal * input[i].tVal)); 
		
		/*apply offset to input vertex position to 
		get output vertex position*/
		element.pos = input[i].position + offset;

		/*Lighting*/
		element.viewVec = normalize(camera - element.pos);
		element.lightVec = normalize(light - element.pos);
		element.normal = input[i].normal;

		/*output the vertex*/
		output.Append(element);
		
		/*texture coords for other side*/
		element.texcoord.x = 1;
		
		/*apply negative offset for vertex on  
		other side of line segment*/
		element.pos = input[i].position - offset;
		
		/*Lighting*/
		element.viewVec = normalize(camera - element.pos);
		element.lightVec = normalize(light - element.pos);
		element.normal = input[i].normal;

		/*output the vertex*/
		output.Append(element);
	}
}




Texture2D TEX_0;
SamplerState SAMPLER_STATE;
//PIXEL SHADER-----------------------------------------------------------
float4 PS_Main(PS_INPUT input) : SV_TARGET
{
	/*Lighting*/
	float3 ambientColour	= float3(0.2f,0.2f,0.2f);
	float3 lightColour		= float3(0.7f, 0.7f, 0.7f);
	float diffuseTerm = clamp(dot(input.normal, input.lightVec), 0.0f, 1.0f);
	float specularTerm = 0;
	//@@for now do this but maybe get rid of the if later!
	if (diffuseTerm > 0.0f)
	{
		specularTerm = pow(saturate(dot(input.normal, normalize(input.lightVec + input.viewVec))), 25);
	}
	float3 final = ambientColour + lightColour * diffuseTerm + lightColour * specularTerm;
	/*NOTE: remove tex coords to test just lighting*/
	return float4(final * TEX_0.Sample(SAMPLER_STATE, input.texcoord), 1.0f);
}

/*technique11 RenderField
{
	pass P0
	{
		SetVertexShader(	CompileShader(vs_5_0, VS_Main())	);
		SetHullShader(		CompileShader(hs_5_0, HS_Main())	);
		SetDomainShader(	CompileShader(ds_5_0, DS_Main())	);
		SetGeometryShader(	CompileShader(gs_5_0, GS_Main())	);
		SetPixelShader(		CompileShader(ps_5_0, PS_Main())	);
	}
}*/
