cbuffer CBDefaultObject_ChangesPerObject : register(b0)
{
	matrix	wvp;
	float3	worldPos;
	float	specularPow;
}
cbuffer CBDefaultObject_ChangesPerFrame : register (b1)
{
	float3	cameraPos;
	float3	lightPos;
	float	intensity;
}

struct PS_INPUT
{
	float4	pos			: SV_POSITION;
	float3	normal		: NORMAL;
	float3  lightVec	: TEXCOORD2;
	float3	viewVec		: TEXCOORD1;
	float2  texCoord	: TEXCOORD0;
};

struct PS_OUT
{
	float4	target0	: SV_TARGET;
};

//////For now just draw one colour & don't worry about texcoords
////Texture2D TEX_0;
////SamplerState SAMPLER_STATE;
//float4 defaultShading(PS_INPUT vert)
//{
//	/*Lighting*/
//	float4 diffuse = float4(0.5, 0, 0, 1.0f); //float4(vert.normal * 0.5, 1.0f);
//	float4 ambient = 0.05f;
//	ambient.w = 1.0f;
//
//	float4 diff = saturate(dot(vert.normal, vert.lightVec));
//
//	// R = 2 * (N.L) * N - L
//	float3 Reflect = normalize(2 * diff * vert.normal - vert.lightVec);
//	float4 specular = pow(saturate(dot(Reflect, vert.viewVec)), specularPow); // R.V^n
//	float4 colour = clamp(ambient + diffuse * diff + specular, 0, 1);
//	return colour;
//}


PS_OUT main(PS_INPUT vert)
{
	PS_OUT output;

	//output.target0 = defaultShading(vert);

	/* Just render it with preset colour - not necessary to the project make the 'renderer' any good */
	output.target0 = float4(0.2, 0.2, 0.2, 1);
	return output;
}


