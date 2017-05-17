

struct VS_INPUT
{
	float4 pos : POSITION;
};
struct PS_INPUT
{
	float4 pos : SV_POSITION;
};

cbuffer CBWorldViewProj : register (b0)
{
	matrix world_view_proj;
}


PS_INPUT VS_Main(VS_INPUT vert)
{
	PS_INPUT output ;
	output.pos = mul(vert.pos, world_view_proj);
	return output;
}

float4 PS_Main(PS_INPUT vert) : SV_TARGET
{
	return float4(0.5f,0.5f,0.5f,1.0f);
}