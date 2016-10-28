//CONSTANT BUFFERS-----------------------------------------------------------------
cbuffer CONSTS : register(b0)
{
	float tessDensity;
	float halfGrassWidth;
	float2 PADDING;
};
//
//cbuffer GS_CONSTS : register(b1)
//{
//	float halfBaseWidth;
//	float padding[3];
//}

cbuffer CBWorldViewProj : register (b1)
{
	matrix world_view_proj;
}

cbuffer CBGrassData : register (b2)
{

}

//INPUT/OUTPUT STRUCTS-----------------------------------------------------------------
struct VS_INPUT_OUTPUT
{
	float3 pos : POSITION;
};

struct HS_CONSTANT_OUTPUT
{
	float edges[2] : SV_TessFactor;
};

struct HS_OUTPUT
{
	float3 cpoint : CPOINT;
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
VS_INPUT_OUTPUT VS_Main(VS_INPUT_OUTPUT vertex)
{
	float4 pos = float4(vertex.pos, 1.0f);
	pos = mul(pos, world_view_proj);
	vertex.pos = pos;
	return	vertex;
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
HS_OUTPUT HS_Main(InputPatch<VS_INPUT_OUTPUT, 4> input, uint id : SV_OutputControlPointID)
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

	float3 pos = pow(1.0f - t, 3.0f) * op[0].cpoint + 3.0f * pow(1.0f - t, 2.0f) * t * op[1].cpoint + 3.0f * (1.0f - t) * pow(t, 2.0f) * op[2].cpoint + pow(t, 3.0f) * op[3].cpoint;

	output.position = float4(pos, 1.0f);
	output.tVal = t;

	return output;
}


//GEOMETRY SHADER--------------------------------------------------------
[maxvertexcount(4)]
void GS_Main(line DS_OUTPUT input[2], inout TriangleStream<PS_INPUT> output)
{
	//should get optimised down to a c&p during compilation
	for (uint i = 0; i < 2; i++)
	{
		float halfWidth = halfGrassWidth * (1 - input[i].tVal);//0.05
		PS_INPUT element;				//vertex for output

		float4 pos = input[i].position; //position of node

		pos.x += halfWidth;				//right of node
		element.pos = pos;
		output.Append(element);

		pos.x -= halfWidth * 2;			//left of node
		element.pos = pos;
		output.Append(element);
	}
}

//PIXEL SHADER-----------------------------------------------------------
float4 PS_Main(PS_INPUT vertex) : SV_TARGET
{
	return float4(0.0f,1.0f,0.0f,1.0f);
}