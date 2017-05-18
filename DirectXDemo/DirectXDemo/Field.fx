/* This stuff is guarenteed to change every frame */
cbuffer CBChangesPerFrame : register(b0)
{
	matrix	worldTransform;
	matrix	viewProjTransform;
	matrix	worldViewProjTransform;
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
	float3	position		: SV_POSITION;			/* Local-space position */
	float3	bitangent		: TANGENT;				/* Local-space tangent */
	float	flexibility		: FLEX;					/* 0-1 flexibility coefficent */
	/* PER-INSTANCE */
	float4	rotation		: INSTANCE_ROTATION;	/* Quaternion rotation */
	float3	basePos			: INSTANCE_POSITION;	/* Local-space base position */
	float3	wind			: INSTANCE_WIND;		/* Wind vector */
};

struct HS_DS_INPUT
{
	float4	bitangent		: BITANGENT;			/* World-space bitangent */
	float4	quadVert0		: QUAD_VERTEX0;			/* Screen-space quad vertex position */
	float4	quadVert1		: QUAD_VERTEX1;			/* Screen-space quad vertex position */
	float3	normal			: NORMAL;				/* Normalised normal */
	float3	basePos			: INSTANCE_POSITION;	/* World-space instance position */
	float	tessDensity		: TESS_DENSITY;			/* Tesselation density */
};

struct HS_CONSTANT_OUTPUT
{
	float edges[2] : SV_TessFactor;					/* Tesselation variables [0] = Detail, [1] = Density */
};

struct GS_INPUT
{
	float4	bitangent		: BITANGENT;			/* World-space bitangent */
	float4	quadVert0		: QUAD_VERTEX0;			/* Screen-space quad vertex */
	float4	quadVert1		: QUAD_VERTEX1;			/* Screen-space quad vertex */
	float3	normal			: NORMAL;				/* Normalised normal */
	float3	basePos			: INSTANCE_POSITION;	/* World-space instance position */
	float	tVal			: T_VAL;				/* Bezier curve time step */
};

struct PS_INPUT
{
	float4	position		: SV_POSITION;			/* Frag position */
	float2	texcoord		: TEXCOORD0;			/* Texture coordinate */
	float3	normal			: NORMAL;				/* Normalized normal */
	float3	viewVec			: TEXCOORD1;			/* View vector for lighting */
	float3	basePos			: TEXCOORD2;			/* World-space instance position */
};



/***** Functions ******/
/* Smooth a float4 value 
	(from Orthmann & GPU gems) */
inline float4 smoothf(float4 x)
{
	return x * x * (3.0 - 2.0 * x);
}
/* Triangle wave function
	(from Orthmann & GPU gems) */
inline float4 trianglef(float4 x)
{
	return abs(frac(x + 0.5) * 2.0 - 1.0);
}
/* Calculate proc animation vertex displacement under wind force 
	(from Orthmann & GPU gems) */
inline float4 windForce(float3 position, float3 wind)
{
	/* Compute the phase shift for the position p with respect to
	the current wind strength and direction */
	float phase = (time *length(wind)) + dot(wind, position);

	/* Compute the four translation strengths. */
	float4 ts = smoothf(trianglef(translationFrequency * phase));

	/* Compute the mean of the four values and
	return the translation vector.
	[ dot(ts, 0.25) == avg of ts componenets ] Vectorizing optimisation for faster average */
	return float4(wind * dot(ts, 0.25), 0.0f);
}

/* Rotate a vector using a unit quaternion
	(from http://donw.io/post/dual-quaternion-skinning/) */
inline float3 quatRotateVector(float4 Qr, float3 v)
{
	return v + 2 * cross(Qr.w * v + cross(v, Qr.xyz), Qr.xyz);
}

/***** VERTEX SHADER ******/
HS_DS_INPUT VS_Main(VS_INPUT vertex)
{
	HS_DS_INPUT output;

	/* Apply instance rotation to the vertex */
	float4 controlPoint = float4(quatRotateVector(vertex.rotation, vertex.position), 1.0f);

	/* Wind displacement */ 
	float4 windDisplacement = (windForce(vertex.basePos, vertex.wind) * vertex.flexibility);
	controlPoint += windDisplacement;

	/* Bitangents */
	/* Grass instance rotation */
	float3 bitangent = normalize(quatRotateVector(vertex.rotation, vertex.bitangent));
	/* Transformation matrix rotation */
	bitangent = mul(bitangent, (float3x3)worldTransform);
	output.bitangent = float4(bitangent, 0);

	/* Quad verts */
	float4 basePos = float4(vertex.basePos, 0.0f);
	float4 quadBaseVec = output.bitangent * (1 - (pow(vertex.flexibility, 2))) * halfGrassWidth;
	output.quadVert0 = basePos + controlPoint + quadBaseVec;
	output.quadVert1 = basePos + controlPoint - quadBaseVec;
	
	/* View-Proj transformation */
	output.quadVert0 = mul(output.quadVert0, worldViewProjTransform);
	output.quadVert1 = mul(output.quadVert1, worldViewProjTransform);
		
	/* Tess factor - NOTE: This could be calculated once in the compute shader stage and passed as an 
	instance struct member. Comes down to speed vs memory */
	float distance = length(camera.xyz - vertex.basePos);
	distance = clamp((distance- nearTess)/(farTess - nearTess), 0, 1);
	output.tessDensity = maxTessDensity - (distance * (maxTessDensity - minTessDensity));

	output.basePos = vertex.basePos;

	return output;
}

/***** HULL-DOMAIN SHADERS ******/
/* NOTE: Good online resource about tesselation http://gpuexperiments.blogspot.co.uk/2010/02/tessellation-example.html */
HS_CONSTANT_OUTPUT HSConst(InputPatch<HS_DS_INPUT, 4> input, uint id : SV_PrimitiveID)
{
	HS_CONSTANT_OUTPUT output;
	output.edges[0] = 1.0f;						/* Detail factor - Always 1 for our "flat" grass blade */
	output.edges[1] = input[id].tessDensity;	/* Density - the LOD of the grass */
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

	/* Bezier curve time step */
	output.tVal = uv.x;

	/* Bezier coefficents */
	float4 co = float4(	pow(1.0f - output.tVal, 3.0f),
						3.0f * pow(1.0f - output.tVal, 2.0f) * output.tVal,
						3.0f * (1.0f - output.tVal) * pow(output.tVal, 2.0f),
						pow(output.tVal, 3.0f));


	/* Bitangent interpolation */
	output.bitangent = co[0] * op[0].bitangent + co[1] * op[1].bitangent + co[2] * op[2].bitangent + co[3] * op[3].bitangent;

	/* Quad verts interpolation */
	output.quadVert0 = co[0] * op[0].quadVert0 + co[1] * op[1].quadVert0 + co[2] * op[2].quadVert0 + co[3] * op[3].quadVert0; 
	output.quadVert1 = co[0] * op[0].quadVert1 + co[1] * op[1].quadVert1 + co[2] * op[2].quadVert1 + co[3] * op[3].quadVert1;

	/* Base pos does not change */
	output.basePos = op[0].basePos;

	return output;
}


/***** GEOMETRY SHADER ******/
[maxvertexcount(4)]
void GS_Main(line GS_INPUT input[2], inout TriangleStream<PS_INPUT> output)
{	
	/* Generate normal vector for this quad 
		y = quad.verts[0] -  quad.verts[2]; 
		x = bitangent;
		z = cross(y, x); */
	float3 normal = cross(normalize(input[1].quadVert1 - input[0].quadVert1), normalize(input[0].bitangent.xyz));

	/* For each point in the line segment */
	for (uint i = 0; i < 2; i++)
	{
		/* Create a vertex to output */
		PS_INPUT element;

		element.basePos = input[i].basePos;
		/* Generate texture coords*/
		element.texcoord = float2(0, 1-input[i].tVal);
		/* Element position */
		element.position = input[i].quadVert0;
		/* Lighting stuff */
		element.viewVec = normalize(camera.xyz - element.position.xyz);
		element.normal = normal;
		/* Output the vertex */
		output.Append(element);

		/* Texture coords for other side */
		element.texcoord.x = 1;
		/* Element position */
		element.position = input[i].quadVert1;
		/* Lighting stuff */
		element.viewVec = normalize(camera.xyz - element.position.xyz);
		/* Output the vertex */
		output.Append(element);
	}
}

/* Lightweight RNG */
float3 rand(uint3 seed)
{
	/* Xorshift algorithm from George Marsaglia's paper */
	seed ^= (seed << 13);
	seed ^= (seed >> 17);
	seed ^= (seed << 5);
	const float UINT_MAX_VALUE_AS_FLOAT = 4294967296.0;
	return float3(seed * (1.0 / UINT_MAX_VALUE_AS_FLOAT));
}


/***** PIXEL SHADER ******/
Texture2D TEX_0;
SamplerState SAMPLER_STATE;
float4 PS_Main(PS_INPUT input) : SV_TARGET
{
	/* Apply somewhat home-brew Phong implementation /w an attempt at two sided lighting */
	/* Improving this would be one of the first changes... */
	float  lightDotNormal = dot(lightDir.xyz, input.normal);
	float3 relfect;
	float3 view;
	float3 highlight;
	float3 illumination;
	/* Back face adjustments */ 
	if (lightDotNormal < 0)
	{
		view = input.viewVec;
		lightDotNormal = dot(lightDir.xyz, -input.normal);
		relfect = 2 * lightDotNormal * -input.normal - lightDir.xyz;
		highlight = clamp(specular * pow(dot(relfect, input.viewVec), shiny), 0, 1);
	}
	else
	{
		view = -input.viewVec;
		relfect = 2 * lightDotNormal * input.normal - lightDir.xyz;
		highlight = clamp(specular * pow(dot(relfect, input.viewVec), shiny), 0, 1);
	}
	illumination = clamp(ambient + diffuse * lightDotNormal + highlight, 0, 1);

	/* Add some colour variation */
	float3 colour = TEX_0.Sample(SAMPLER_STATE, input.texcoord);
	colour += rand(asuint(input.basePos)) * 0.1f;

	return float4(illumination * colour, 1.0f);
}