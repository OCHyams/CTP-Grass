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
	float4	camera;
	float4	light;
	float	intensity;
}
//INPUT/OUTPUT STRUCTS-----------------------------------------------------------------
struct VS_INPUT
{
	float3	pos			: SV_POSITION;			//position
	float3	binormal	: BINORMAL;				//@@To be used for per-vertex binormals soon
	float	flexibility	: FLEX;					//multiplier for wind deformation
	matrix	world		: INSTANCE_WORLD;
	float3	location	: INSTANCE_LOCATION;
};

struct VS_OUTPUT
{
	float4	pos			: POSITION;
	float4	binormal	: BINORMAL;
};

struct HS_CONSTANT_OUTPUT
{
	float edges[2] : SV_TessFactor;
};

struct HS_OUTPUT
{
	float4 cpoint : CPOINT;
};

struct DS_OUTPUT
{
	float4 position : SV_Position;
	float tVal : T_VAL;
};

struct PS_INPUT
{
	float4	pos			: SV_POSITION;
	float2	texcoord	: TEXCOORD0;
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
	float4 pos = float4(vertex.pos, 1.f);
	////position in world space
	//float3 base_pos = float3(vertex.world[3][0], vertex.world[3][1], vertex.world[3][2]);
	//orthomans technique.... [removed square, this can be hard coded on cpu side for efficiency]
	pos += (windForce(vertex.location, wind) * vertex.flexibility);

	matrix wvp = mul(vertex.world, view_proj);
	pos = mul(pos, wvp); 

	

	output.pos = pos;
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
	return output;
}


[domain("isoline")]
DS_OUTPUT DS_Main(HS_CONSTANT_OUTPUT input, OutputPatch<HS_OUTPUT, 4> op, float2 uv : SV_DomainLocation)
{
	DS_OUTPUT output;

	float t = uv.x;
	//cubic bezier curve
	float4 pos = pow(1.0f - t, 3.0f) * op[0].cpoint + 3.0f * pow(1.0f - t, 2.0f) * t * op[1].cpoint + 3.0f * (1.0f - t) * pow(t, 2.0f) * op[2].cpoint + pow(t, 3.0f) * op[3].cpoint;

	output.position = pos;
	output.tVal = t;

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
		float4 pos = input[i].position + offset;
		element.pos = pos;
		/*output the vertex*/
		output.Append(element);
		
		/*texture coords for other side*/
		element.texcoord.x = 1;
		
		/*apply negative offset for vertex on  
		other side of line segment*/
		pos = input[i].position - offset;
		element.pos = pos;
		
		/*output the vertex*/
		output.Append(element);
	}
}




//@putting in texture stuff
Texture2D TEX_0;
SamplerState SAMPLER_STATE;
//PIXEL SHADER-----------------------------------------------------------
float4 PS_Main(PS_INPUT input) : SV_TARGET
{
	//apply texture col
	float4 col = TEX_0.Sample(SAMPLER_STATE, input.texcoord);
	return col;
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
