/*Global Data*/
cbuffer CBDefaultObject_ChangesPerObject : register(b0)
{
	matrix	wvp;//@test
	float3	worldPos;
	float	specularPow;
}

cbuffer CBDefaultObject_ChangesPerFrame : register (b1)
{
	float3	cameraPos;
	float3	lightPos;
	float	intensity;
}


/*Input structs*/
struct VS_INPUT
{
	float3 pos		: POSITION;
	float3 normal	: NORMAL;
	float2 texCoord	: TEXCOORD0;
};

struct PS_INPUT
{
	float4	pos			: SV_POSITION;
	float3	normal		: NORMAL;
	float3  lightVec	: TEXCOORD2;
	float3	viewVec		: TEXCOORD1;
	float2	texCoord	: TEXCOORD0;
};

PS_INPUT main(VS_INPUT vert)
{
	PS_INPUT output;

	output.pos = mul(float4(vert.pos, 1.0f), wvp);
	output.normal = vert.normal;//uhm...needs to get rotated...!
	output.viewVec = normalize(cameraPos - vert.pos);
	output.lightVec = normalize(lightPos - vert.pos);
	output.texCoord = vert.texCoord;
	return output;
}