float4 VS_Main(float4 pos : POSITION) : SV_POSITION
{
	return pos;
}

float4 PS_Main(float4 pos : SV_POSITION) : SV_TARGET
{
	return float4(0.0f,1.0f,0.0f,1.0f);
}

//need a struct even for just one member because the position needs to have a semantic tag thing
struct GS_OUT
{
	float4 pos : SV_POSITION; 
};

[maxvertexcount(9)]
void GS_Main(triangle GS_OUT input[3], inout TriangleStream<GS_OUT> triStream)
{
	GS_OUT output;
	for (int i = 0; i < 3; ++i)
	{
		output.pos = input[i].pos + float4(-0.25, 0.25, 0.f, 0.f);
		triStream.Append(output);

		output.pos = input[i].pos + float4(0.25f, -0.25f, 0.5f, 0.f);
		triStream.Append(output);

		output.pos = input[i].pos + float4(-0.25f, -0.25f, 0.5f, 0.f);
		triStream.Append(output);

		triStream.RestartStrip();
	}
	
}