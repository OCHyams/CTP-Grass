struct Instance
{
	float4 rot;
	float3 pos;
	float3 wind;
	int octreeIdx;
	float padding;
};

//@For now only deal with AABB wind volumes
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
};


ByteAddressBuffer				inGrass		: register(t0);
StructuredBuffer<WindCuboid>	inCuboids	: register(t1);
StructuredBuffer<WindSphere>	inSpheres	: register(t2);
StructuredBuffer<OctreeNode>	inNodes		: register(t3);

RWByteAddressBuffer				outGrass		: register(u0);
RWByteAddressBuffer				outGrassFrustumCulled : register (u1);
static uint ARGS_VCOUNT = 0, ARGS_INST_COUNT = 1, ARGS_VERT_START = 2, ARGS_INST_START = 3;
RWBuffer<uint>					indirectArgs	: register(u2);

static uint SIZE_OF_INSTANCE = 48; //44 /*Raw*/ +4 /*Padding*/;
static uint ROT_OFFSET = 0;
static uint POS_OFFSET = 16;
static uint WIND_OFFSET = 28;
static uint OCT_IDX_OFFSET = 40;

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
	if (DTid.x > numInstances) return;//This is required because rounding to int means that more - than - necessary number of threads are run! This pushes out the chance for some instances to get put into the append buffer

	Instance instance;
	uint inAddress = DTid.x * SIZE_OF_INSTANCE;
	uint outAddress = 0;

	instance.pos		= asfloat(inGrass.Load3(inAddress + POS_OFFSET));
	instance.wind		= windFromCuboids(instance.pos) + windFromSpheres(instance.pos);
	instance.rot		= asfloat(inGrass.Load4(inAddress + ROT_OFFSET));
	instance.octreeIdx	= asint(inGrass.Load(inAddress + OCT_IDX_OFFSET));
	instance.padding	= 0;

	//emulate append buffer
	//If visble, ~~add to front of output buffer~~ add to the pseudo append buffer
	if (inNodes[instance.octreeIdx].visible > 0)
	{
		InterlockedAdd(indirectArgs[ARGS_INST_COUNT], 1, outAddress);
		outAddress *= SIZE_OF_INSTANCE;
		outGrassFrustumCulled.Store4(outAddress + ROT_OFFSET, asuint(instance.rot));
		outGrassFrustumCulled.Store3(outAddress + POS_OFFSET, asuint(instance.pos));
		outGrassFrustumCulled.Store3(outAddress + WIND_OFFSET, asuint(instance.wind));
		outGrassFrustumCulled.Store(outAddress + OCT_IDX_OFFSET, asuint(instance.octreeIdx));
	}
	else //else add to back of buffer
	{}

	//add all grass to outbuff
	outGrass.Store4(inAddress + ROT_OFFSET, asuint(instance.rot));
	outGrass.Store3(inAddress + POS_OFFSET, asuint(instance.pos));
	outGrass.Store3(inAddress + WIND_OFFSET, asuint(instance.wind));
	outGrass.Store(inAddress + OCT_IDX_OFFSET, asuint(instance.octreeIdx));
}
