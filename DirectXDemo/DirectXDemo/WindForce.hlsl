struct Instance
{
	float4 rot;
	float3 pos;
	float3 wind;
	int octreeIdx;
	float padding;
};

struct WindCuboid
{
	float3	pos;
	float3	extents;
	float3	velocity;
};

struct WindSphere
{
	float3	pos;
	float	radius;
	float	initalStrength;
	float	fallOffPow;
};

struct OctreeNode
{
	int childIdx[8];
	int idx;
	int parentIdx;
	int visible; 
	int padding;
};

cbuffer CBChangePerFrame
{
	uint numCuboids;
	uint numSpheres;
	uint numInstances;
	float time;
	float deltaTime;
};

/* Shader resources */
ByteAddressBuffer				inGrass		: register(t0);
StructuredBuffer<WindCuboid>	inCuboids	: register(t1);
StructuredBuffer<WindSphere>	inSpheres	: register(t2);
StructuredBuffer<OctreeNode>	inNodes		: register(t3);

/* Unordered access resources */
RWByteAddressBuffer	outGrass		: register(u0);
RWByteAddressBuffer	outGrassFrustumCulled : register (u1);
RWBuffer<uint>		indirectArgs	: register(u2);

static uint ARGS_VERT_COUNT = 0, 
			ARGS_INST_COUNT = 1, 
			ARGS_VERT_START = 2, 
			ARGS_INST_START = 3;

static uint SIZE_OF_INSTANCE = 48; /* Raw + Padding = 44 + 4 */
static uint ROT_OFFSET = 0;
static uint POS_OFFSET = 16;
static uint WIND_OFFSET = 28;
static uint OCT_IDX_OFFSET = 40;

/* Return wind force at this point
	from all wind cuboids. */
float3 windFromCuboids(float3 pos)
{
	float3 output = 0;
	for (uint i = 0; i < numCuboids; ++i)
	{
		WindCuboid cuboid = inCuboids[i];
		float3 min = cuboid.pos - cuboid.extents;
		float3 max = cuboid.pos + cuboid.extents;

		if ((pos.x >= min.x && pos.x <= max.x) &&
			(pos.y >= min.y && pos.y <= max.y) &&
			(pos.z >= min.z && pos.z <= max.z))
		{
			output += cuboid.velocity;
		}
	}

	return output;
}

/* Return wind force at this point
	from all wind spheres. */
float3 windFromSpheres(float3 pos)
{
	float3 output = 0;
	for (uint i = 0; i < numSpheres; ++i)
	{
		WindSphere sphere = inSpheres[i];
		float3 vec = pos - sphere.pos;
		float distance = length(vec);


		if (distance <= sphere.radius)
		{
			vec = normalize(vec);
			output += (sphere.initalStrength - (pow(distance / sphere.radius, sphere.fallOffPow) * sphere.initalStrength)) * vec;
		}
	}

	return output;
}

[numthreads(256, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	/* This is required because rounding in the num-threads calculation
		often leads to more-than-necessary threads are run.
		Without this check, the append buffer counter can be incremented without
		having added anything! */
	if (DTid.x >= numInstances) return;

	Instance instance;
	uint inAddress = DTid.x * SIZE_OF_INSTANCE;
	uint outAddress = 0;

	instance.rot		= asfloat(inGrass.Load4(inAddress + ROT_OFFSET));
	instance.pos		= asfloat(inGrass.Load3(inAddress + POS_OFFSET));
	instance.wind		= windFromCuboids(instance.pos) + windFromSpheres(instance.pos);
	instance.octreeIdx	= asint(inGrass.Load(inAddress + OCT_IDX_OFFSET));
	instance.padding = 0;


	/* If visble... */
	if (inNodes[instance.octreeIdx].visible > 0)
	{	
		/* ...add instance to the pseudo append buffer 
			which will be used as-is as the instance buffer
			in the next grass rendering stage. */

		/* Increment indrect arguments instance count. */
		InterlockedAdd(indirectArgs[ARGS_INST_COUNT], 1, outAddress);
		/* Add instance to buffer. */
		outAddress *= SIZE_OF_INSTANCE;
		outGrassFrustumCulled.Store4(outAddress + ROT_OFFSET, asuint(instance.rot));
		outGrassFrustumCulled.Store3(outAddress + POS_OFFSET, asuint(instance.pos));
		outGrassFrustumCulled.Store3(outAddress + WIND_OFFSET, asuint(instance.wind));
		outGrassFrustumCulled.Store(outAddress + OCT_IDX_OFFSET, asuint(instance.octreeIdx));
	}

	/* Add all grass to the output buffer */
	outGrass.Store4(inAddress + ROT_OFFSET, asuint(instance.rot));
	outGrass.Store3(inAddress + POS_OFFSET, asuint(instance.pos));
	outGrass.Store3(inAddress + WIND_OFFSET, asuint(instance.wind));
	outGrass.Store(inAddress + OCT_IDX_OFFSET, asuint(instance.octreeIdx));
}
