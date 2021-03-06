//CONSTANT BUFFERS-----------------------------------------------------------------
cbuffer CONSTS : register(b0)
{
	float tessDensity;
	float halfGrassWidth;
	//game time
	float time;	
	//wind vector
	float wind_x;
	float wind_y;
	float wind_z;
	//used to compute the wind animation (orthoman style)
	float base_x; 
	float base_y;
	float base_z;
	//bitangent for mesh generation
	float tan_x;
	float tan_y;
	float tan_z;
	float tan_w;
};

cbuffer CBWorldViewProj : register (b1)
{
	matrix world_view_proj;
}

//do this...
cbuffer CBLight : register (b2)
{
	float pos_x;
	float pos_y;
	float pos_z;
	float intensity;
	float cam_x;
	float cam_y;
	float cam_z;
}


//INPUT/OUTPUT STRUCTS-----------------------------------------------------------------
struct VS_INPUT
{
	float3 pos	: SV_POSITION;//position
	float tVal : T_VAL;	//how far along the blade? 
};

struct VS_OUTPUT
{
	float4 pos : POSITION;
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
	float4 pos : SV_POSITION;
};


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
	float4 ts = smoothf(trianglef(float4(1.975, 0.793, 0.375, 0.193) * phase));
	// Compute the mean of the four values and
	// return the translation vector.
	return float4(wind * dot(ts, 0.25), 0.0f);
}

VS_OUTPUT VS_Main(VS_INPUT vertex)
{
	VS_OUTPUT output;
	float4 pos = float4(vertex.pos, 1.f);

	pos += (windForce(float3(base_x, base_y, base_z), float3(wind_x, wind_y, wind_z)) * vertex.tVal * vertex.tVal); //<-orthomans technique.... (also, square tVal to have stiff base, cube for stiffer grass)
	pos = mul(pos, world_view_proj);
	//output.pos = pos +(windForce(float3(base_x, base_y, base_z), float3(wind_x, wind_y, wind_z)) * vertex.tVal * vertex.tVal); //<-orthomans technique.... (also, square tVal to have stiff base, cube for stiffer grass)
	//output.pos = pos + (windForce(pos, float3(wind_x, wind_y, wind_z)) *vertex.tVal* vertex.tVal); //<-is this better?
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

	output.position = pos; // float4(pos, 1.0f);
	output.tVal = t;

	return output;
}


//GEOMETRY SHADER--------------------------------------------------------
[maxvertexcount(4)]
void GS_Main(line DS_OUTPUT input[2], inout TriangleStream<PS_INPUT> output)
{
	for (uint i = 0; i < 2; i++)
	{
		PS_INPUT element;	//vertex for output
		float4 bitangent = float4(tan_x, tan_y, tan_z, tan_w) * (1 - (input[i].tVal * input[i].tVal)); //parabolic curve for slightly more realistic grass :)

		float4 pos = input[i].position + bitangent;
		element.pos = pos;
		output.Append(element);

		pos = input[i].position - bitangent;
		element.pos = pos;
		output.Append(element);
	}
}

//PIXEL SHADER-----------------------------------------------------------
float4 PS_Main(PS_INPUT vertex) : SV_TARGET
{
	return float4(0.0f,1.0f,0.0f,1.0f);
}