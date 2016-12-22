//CONSTANT BUFFERS-----------------------------------------------------------------

cbuffer CONSTS : register(b0)
{
	float3	wind;		
	float	halfGrassWidth; //this should be in another buffer that never needs to be updated....
	float	time;
	float	minTessDensity;//this should be in another buffer that never needs to be updated....
	float	maxTessDensity;//this should be in another buffer that never needs to be updated....
	float	nearTess;//this should be in another buffer that never needs to be updated....
	float	farTess;//this should be in another buffer that never needs to be updated....
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
	float3	binormal	: BINORMAL;				//Helps place the quad verts
	float3	normal		: NORMAL;
	float	flexibility	: FLEX;					//multiplier for wind deformation
	//PER-INSTANCE
	//matrix	world		: INSTANCE_WORLD;		//world transform matrix - Not in use atm, might be required later...
	float4	rotation	: INSTANCE_ROTATION;	//quaternion rotation
	float3	location	: INSTANCE_LOCATION;	//position of grass in world for lighting calc etc
};

struct HS_DS_INPUT
{
	float4	cpoint		: POSITION;
	float4	binormal	: BINORMAL;
	float4	b1			: BINORMAL_WORLDPOS0;
	float4	b2			: BINORMAL_WORLDPOS1;
	float3	normal		: NORMAL;
	float	tessDensity : TESS_DENSITY;
};

struct HS_CONSTANT_OUTPUT
{
	float edges[2] : SV_TessFactor;
};

//struct HS_OUTPUT
//{
//	float4	cpoint		: CPOINT;
//	float4	binormal	: BINORMAL;
//	float4	b1			: BINORMAL_WORLDPOS0;
//	float4	b2			: BINORMAL_WORLDPOS1;
//	float3	normal		: NORMAL;
//};

struct GS_INPUT
{
	//float4	position	: SV_Position;
	float4	binormal	: BINORMAL;
	float4	b1			: BINORMAL_WORLDPOS0;
	float4	b2			: BINORMAL_WORLDPOS1;
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

//http://donw.io/post/dual-quaternion-skinning/
inline float3 quatRotateVector(float4 Qr, float3 v)
{
	return v + 2 * cross(Qr.w * v + cross(v, Qr.xyz), Qr.xyz);
}

//http://stackoverflow.com/questions/1171849/finding-quaternion-representing-the-rotation-from-one-vector-to-another
inline float4 quatFromTwoVec(float3 v0, float3 v1)
{
	float4 q;
	v0 = cross(v0, v1);
	q.xyz = v0;
	q.w = sqrt(pow(length(v0) , 2) * pow(length(v1),2)) + dot(v0, v1);
	return normalize(q);
}

inline float3 project(float3 v0, float3 v1)
{
	return v1* dot(v0, v1) / pow(length(v1), 2);
}

float4 quatMul(float4 q1, float4 q2)
{
	float4 result;
	result.x = q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x;
	result.y = -q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y;
	result.z = q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z;
	result.w = -q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w;
	return result;
}

HS_DS_INPUT VS_Main(VS_INPUT vertex)
{
	HS_DS_INPUT output;

	//output.cpoint = float4(vertex.pos, 1.f);//@working
	output.cpoint = float4(quatRotateVector(vertex.rotation, vertex.pos), 1.0f);//@new

	/*Wind displacement*/ //[Orthomans technique]
	output.cpoint += (windForce(vertex.location, wind) * vertex.flexibility);

	/*Rotation of vectors under wind force*/ //-@->NOT WORKING ATM
	float4 rot = quatFromTwoVec(vertex.pos, output.cpoint);

	//@when better wind simulation is in, use twisting to manipulate normal
	/*Normals*/
	output.normal = quatRotateVector(/*@quatMul(rot, */vertex.rotation/*)*/, vertex.normal);
	output.normal = normalize(output.normal);

	/*Binormals*/
	output.binormal = float4(quatRotateVector(vertex.rotation, vertex.binormal),0.f);
	output.binormal = normalize(output.binormal);

	///*Quad verts*///@working
	//output.b1 = output.cpoint + binormal * (1 - (pow(vertex.flexibility, 2))) * halfGrassWidth;
	//output.b2 = output.cpoint - binormal * (1 - (pow(vertex.flexibility, 2))) * halfGrassWidth;

	/*Quad verts*///@new
	output.b1 = float4(vertex.location, 0.0f) + output.cpoint + output.binormal * (1 - (pow(vertex.flexibility, 2))) * halfGrassWidth;
	output.b2 = float4(vertex.location, 0.0f) + output.cpoint - output.binormal * (1 - (pow(vertex.flexibility, 2))) * halfGrassWidth;

	/*View-Proj transformation*///@new
	output.cpoint = mul(output.cpoint, view_proj);
	output.b1 = mul(output.b1, view_proj);
	output.b2 = mul(output.b2, view_proj);

	///*World-View-Proj transformation*///@working
	//matrix wvp = mul(vertex.world, view_proj);
	//output.cpoint = mul(output.cpoint, wvp);

	///*Put quad verts into world space*/
	//output.b1 = mul(output.b1, wvp);
	//output.b2 = mul(output.b2, wvp);

	/*Tess factor*/
	float distance = length(camera - vertex.location);
	distance = (distance- nearTess)/(farTess - nearTess);
	output.tessDensity = maxTessDensity - distance * (maxTessDensity - minTessDensity);

	return output;
}

//HULL-DOMAIN SHADERS----------------------------------------------------
//See http://gpuexperiments.blogspot.co.uk/2010/02/tessellation-example.html
HS_CONSTANT_OUTPUT HSConst(InputPatch<HS_DS_INPUT, 4> input, uint id : SV_PrimitiveID)
{
	HS_CONSTANT_OUTPUT output;

	output.edges[0] = 1.0f;				// Detail factor
	output.edges[1] = input[id].tessDensity;		// tess density
	return output;
}


[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(4)]
[patchconstantfunc("HSConst")]
HS_DS_INPUT HS_Main(InputPatch<HS_DS_INPUT, 4> input, uint id : SV_OutputControlPointID)
{
	return input[id];
}

//@Need to find some optimisation here...
//can probably vectorise this?
[domain("isoline")]
GS_INPUT DS_Main(HS_CONSTANT_OUTPUT input, OutputPatch<HS_DS_INPUT, 4> op, float2 uv : SV_DomainLocation)
{
	GS_INPUT output;

	/*Time*/
	output.tVal = uv.x;

	/*Coefficents*/
	float4 co = float4(	pow(1.0f - output.tVal, 3.0f),
						3.0f * pow(1.0f - output.tVal, 2.0f) * output.tVal,
						3.0f * (1.0f - output.tVal) * pow(output.tVal, 2.0f),
						pow(output.tVal, 3.0f));

	
	/*Position = evaluateBezier(output.tVal)*/
	//output.position = co[0] * op[0].cpoint + co[1] * op[1].cpoint + co[2] * op[2].cpoint + co[3] * op[3].cpoint;

	/*Normal = evaluateBezier(output.tVal)*/
	output.normal = co[0] * op[0].normal + co[1] * op[1].normal + co[2] * op[2].normal + co[3] * op[3].normal;

	/*Biormal = evaluateBezier(output.tVal)*/
	output.binormal = co[0] * op[0].binormal + co[1] * op[1].binormal + co[2] * op[2].binormal + co[3] * op[3].binormal;

	/*Quad verts*/
	output.b1 = co[0] * op[0].b1 + co[1] * op[1].b1 + co[2] * op[2].b1 + co[3] * op[3].b1; //<<@@VECTORIZING potential, that's a 4-vector dot, matrix?
	output.b2 = co[0] * op[0].b2 + co[1] * op[1].b2 + co[2] * op[2].b2 + co[3] * op[3].b2;


	return output;
}


//GEOMETRY SHADER--------------------------------------------------------
[maxvertexcount(4)]
void GS_Main(line GS_INPUT input[2], inout TriangleStream<PS_INPUT> output)
{	
	/*for each point in the line segment*/
	for (uint i = 0; i < 2; i++)
	{
		/*create a vertex to output*/
		PS_INPUT element;

		/*texture coords*/
		element.texcoord = float2(0, 1-input[i].tVal);
		
		/*Element position*/
		element.pos = input[i].b1;

		/*Lighting*/
		element.viewVec = normalize(camera - element.pos);
		element.lightVec = normalize(light - element.pos);
		element.normal = input[i].normal;

		/*output the vertex*/
		output.Append(element);
		
		/*texture coords for other side*/
		element.texcoord.x = 1;
		
		/*Element position*/
		element.pos = input[i].b2;

		/*Lighting*/
		element.viewVec = normalize(camera - element.pos);
		element.lightVec = normalize(light - element.pos);

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
	float3 ambientColour = 0.2f;

	//@Trying to do twoside shading
	float diffuseTerm1 = clamp(dot(input.normal, input.lightVec), 0.0f, 1.0f);
	float diffuseTerm2 = clamp(dot(-input.normal, input.lightVec), 0.0f, 1.0f);
	float diffuseTerm = diffuseTerm1;
	float3 normal = input.normal;

	if (diffuseTerm1 < diffuseTerm2)
	{
		normal *= -1;
		diffuseTerm = diffuseTerm2;
		ambientColour = 0.2;
	}
	
	float specularTerm = 0;
	//@@for now do this but maybe get rid of the if later!
	//if (diffuseTerm > 0.0f)
	//{
		specularTerm = pow(saturate(dot(/*input.*/normal, normalize(input.lightVec + input.viewVec))), 25);
	//}
	float3 final = ambientColour + intensity * diffuseTerm + intensity * specularTerm;
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
