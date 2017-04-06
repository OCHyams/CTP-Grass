/***** Buffers and global variables ******/
//cbuffer CBChangesPerFrame : register(b0)
//{	
//	float	halfGrassWidth; //this should be in another buffer that never needs to be updated....
//	float	time;
//	float	minTessDensity;//this should be in another buffer that never needs to be updated....
//	float	maxTessDensity;//this should be in another buffer that never needs to be updated....
//	float	nearTess;//this should be in another buffer that never needs to be updated....
//	float	farTess;//this should be in another buffer that never needs to be updated....
//};
//
//cbuffer CBViewProj : register (b1)
//{
//	matrix view_proj;
//}
//
//cbuffer CBLight : register (b2)
//{
//	float4	camera; //Packing doesn't much like float3s
//	float4	lightDir;
//	float4	ambient;
//	float4  diffuse;
//	float4	specular;
//	float	shiny;
//}
/* This stuff is guarenteed to change every frame */
cbuffer CBChangesPerFrame : register(b0)
{
	matrix view_proj;
	float	time;
};

/* This stuff isn't likely to get updated often, if at all.
It's here so the user can customise the grass to some extent -
width & LODing control*/
cbuffer CBRarelyChanges : register (b1)
{
	float	halfGrassWidth;
	float	maxTessDensity;
	float	minTessDensity;
	float	nearTess;
	float	farTess;
}

/* This was added as a buffer for debug access but was kept so that a user
can drastically alter light properties if they so wish */
cbuffer CBLight : register (b2)
{
	float4	camera;
	float4	lightDir;
	float4	ambient;
	float4  diffuse;
	float4	specular;
	float	shiny;
}

const static float4 translationFrequency = float4(1.975, 0.793, 0.375, 0.193);

/***** I/O Structures ******/
struct VS_INPUT
{
	/* PER_VERTEX */
	float3	pos				: SV_POSITION;			/* Local position */
	float3	bitangent		: TANGENT;				/* Local tangent (normal is derived later) */
	float3	normal			: NORMAL;				/* REMOVE THIS */
	float	flexibility		: FLEX;					/* Could remove this?*/
	/* PER-INSTANCE */
	float4	rotation		: INSTANCE_ROTATION;	/* Quaternion rotation */
	float3	worldPosition	: INSTANCE_LOCATION;	/* World space base position */
	float3	wind			: INSTANCE_WIND;		/* Wind vector */
};

struct HS_DS_INPUT
{
	float4	cpoint		: POSITION;
	float4	bitangent	: BITANGENT;				//world space
	float4	b1			: BITANGENT_WORLDPOS0;//screen space
	float4	b2			: BITANGENT_WORLDPOS1;
	float3	normal		: NORMAL;
	float3	basePos		: INSTANCE_LOCATION;
	float	tessDensity : TESS_DENSITY;
};

struct HS_CONSTANT_OUTPUT
{
	float edges[2] : SV_TessFactor;
};

struct GS_INPUT
{
	float4	bitangent	: BITANGENT;
	float4	b1			: BITANGENT_WORLDPOS0;
	float4	b2			: BITANGENT_WORLDPOS1;
	float3	normal		: NORMAL;
	float3	basePos		: INSTANCE_LOCATION;
	float	tVal		: T_VAL;
};

struct PS_INPUT
{
	float4	pos			: SV_POSITION;
	float2	texcoord	: TEXCOORD0;
	float3	normal		: NORMAL;
	float3  lightVec	: TEXCOORD1;
	float3	viewVec		: TEXCOORD2;
	float3	basePos		: TEXCOORD3;
};



/***** Functions ******/
/* Orthoman & GPU gems */
inline float4 smoothf(float4 x)
{
	return x * x * (3.0 - 2.0 * x);
}
/* Orthoman & GPU gems */
inline float4 trianglef(float4 x)
{
	return abs(frac(x + 0.5) * 2.0 - 1.0);
}
/* Orthoman & GPU gems */
inline float4 windForce(float3 p, float3 windVec)
{
	/* Compute the phase shift for the position p with respect to
	the current wind strength and direction */
	float phase = (time *length(windVec)) + dot(windVec, p);

	/* Compute the four translation strengths. */
	float4 ts = smoothf(trianglef(translationFrequency * phase));

	/* Compute the mean of the four values and
	return the translation vector. --> dot(ts, 0.25) Vectorizing optimisation for fast average */
	return float4(windVec * dot(ts, 0.25), 0.0f);
}

/* Rotate a vector using a unit quaternion, taken from http://donw.io/post/dual-quaternion-skinning/ */
inline float3 quatRotateVector(float4 Qr, float3 v)
{
	return v + 2 * cross(Qr.w * v + cross(v, Qr.xyz), Qr.xyz);
}

/* Find a quaternion rotation from two direction vectors, taken from
http://stackoverflow.com/questions/1171849/finding-quaternion-representing-the-rotation-from-one-vector-to-another */
inline float4 quatFromTwoVec(float3 v0, float3 v1)
{
	float4 q;
	v0 = cross(v0, v1);
	q.xyz = v0;
	q.w = sqrt(pow(length(v0), 2) * pow(length(v1), 2)) + dot(v0, v1);
	return normalize(q);
}

/***** VERTEX SHADER ******/
HS_DS_INPUT VS_Main(VS_INPUT vertex)
{
	HS_DS_INPUT output;

	/* Apply instance rotation to the vertex */
	output.cpoint = float4(quatRotateVector(vertex.rotation, vertex.pos), 1.0f);

	/* Wind displacement */ 
	float4 windDisplacement = (windForce(vertex.worldPosition, vertex.wind) * vertex.flexibility);
	output.cpoint += windDisplacement;

	output.normal = float3(0, 0, 0);//@REMOVE

	/* Binormals */
	output.bitangent = float4(quatRotateVector(vertex.rotation, vertex.bitangent),0.f);
	output.bitangent = float4(normalize(output.bitangent.xyz), 0);

	/* Quad verts */
	float4 binormal = output.bitangent * (1 - (pow(vertex.flexibility, 2))) * halfGrassWidth;
	output.b1 = float4(vertex.worldPosition, 0.0f) + output.cpoint + binormal;
	output.b2 = float4(vertex.worldPosition, 0.0f) + output.cpoint - binormal;

	/* View-Proj transformation */
	output.cpoint = mul(output.cpoint, view_proj);
	output.b1 = mul(output.b1, view_proj);
	output.b2 = mul(output.b2, view_proj);
		
	/* Tess factor - NOTE: This could be calculated once in the compute shader stage and passed as an 
	instance struct member. Comes down to speed vs memory consumption */
	float distance = length(camera.xyz - vertex.worldPosition);
	distance = clamp((distance- nearTess)/(farTess - nearTess), 0, 1);
	output.tessDensity = maxTessDensity - (distance * (maxTessDensity - minTessDensity));

	output.basePos = vertex.worldPosition;

	return output;
}

/***** HULL-DOMAIN SHADERS ******/
/* Good online resource about tesselation http://gpuexperiments.blogspot.co.uk/2010/02/tessellation-example.html */
HS_CONSTANT_OUTPUT HSConst(InputPatch<HS_DS_INPUT, 4> input, uint id : SV_PrimitiveID)
{
	HS_CONSTANT_OUTPUT output;
	output.edges[0] = 1.0f;						/* Detail factor - Always 1 for our "flat" grass blade*/
	output.edges[1] = input[id].tessDensity;	/* Tess density - controls the LOD of the grass */
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


	/* Bitangent = evaluateBezier(output.tVal)*/
	output.bitangent = co[0] * op[0].bitangent + co[1] * op[1].bitangent + co[2] * op[2].bitangent + co[3] * op[3].bitangent;

	/* Quad verts */
	output.b1 = co[0] * op[0].b1 + co[1] * op[1].b1 + co[2] * op[2].b1 + co[3] * op[3].b1; 
	output.b2 = co[0] * op[0].b2 + co[1] * op[1].b2 + co[2] * op[2].b2 + co[3] * op[3].b2;

	output.basePos = op[0].basePos;

	return output;
}


/***** GEOMETRY SHADER ******/
[maxvertexcount(4)]
void GS_Main(line GS_INPUT input[2], inout TriangleStream<PS_INPUT> output)
{	
	/* Generate normal vector for this quad */
	float3 normal = cross(normalize(input[1].b2 - input[0].b2), normalize(input[0].bitangent));

	/* For each point in the line segment */
	for (uint i = 0; i < 2; i++)
	{
		/* Create a vertex to output */
		PS_INPUT element;

		element.basePos = input[i].basePos;

		/* Generate texture coords*/
		element.texcoord = float2(0, 1-input[i].tVal);
		
		/* Element position */
		element.pos = input[i].b1;

		/* Lighting stuff - @NOT REQUIRED? */
		element.viewVec = normalize(camera.xyz - element.pos.xyz);
		element.lightVec = normalize(lightDir.xyz - element.pos.xyz);
		
		element.normal = normal;

		/* Output the vertex */
		output.Append(element);

		/* Texture coords for other side */
		element.texcoord.x = 1;
		
		/* Element position */
		element.pos = input[i].b2;

		/* Lighting - @ AGAIN?*/
		element.viewVec = normalize(camera.xyz - element.pos.xyz);
		element.lightVec = normalize(-lightDir.xyz - element.pos.xyz);

		/* Output the vertex */
		output.Append(element);
	}
}

/* Lightweight vectorized RNG */
float3 rand(uint3 seed)
{
	/* Xorshift algorithm from George Marsaglia's paper */
	seed ^= (seed << 13);
	seed ^= (seed >> 17);
	seed ^= (seed << 5);

	return float3(seed * (1.0 / 4294967296.0));
}


/***** PIXEL SHADER ******/
Texture2D TEX_0;
SamplerState SAMPLER_STATE;
float4 PS_Main(PS_INPUT input) : SV_TARGET
{
	/* Apply somewhat home-brew Phong implementation /w an attempt at two sided lighting */
	//REMOVE THE LLIGHTDIR FROM THE OTHER STAGES!!!!!!@@??
	float3 LdotN = dot(lightDir.xyz, input.normal);
	float3 R;
	float3 highlight;
	float3 illumination;
	/* Back face adjustments */ 
	if (LdotN.x < 0)
	{
		LdotN = dot(lightDir.xyz, -input.normal);
		R = 2 * LdotN * -input.normal - lightDir.xyz;
		highlight = clamp(specular * pow(dot(R, input.viewVec), shiny), 0, 1);
	}
	else
	{
		R = 2 * LdotN * input.normal - lightDir.xyz;
		highlight = clamp(specular * pow(dot(R, input.viewVec), shiny), 0, 1);
	}

	/* Add some colour variation */
	float3 colour = TEX_0.Sample(SAMPLER_STATE, input.texcoord);
	colour += rand(asuint(input.basePos)) * 0.1f;

	/* Deal with some directional light */
	float directionalLightVal = clamp(-dot(input.viewVec.xyz, lightDir.xyz), 0, 1);
	illumination = ambient + diffuse * LdotN * directionalLightVal + highlight *directionalLightVal;

	return float4(illumination * colour, 1.0f);
}