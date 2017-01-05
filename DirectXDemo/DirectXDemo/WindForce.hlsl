struct Instance
{
	float4 rot;
	float3 pos;
	float3 wind;
	//float2 padding;
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


cbuffer CBChangePerFrame
{
	uint numCuboids;
	uint numSpheres;
	uint numInstances;
};

StructuredBuffer<WindCuboid>	inCuboids	: register(t0);
StructuredBuffer<WindSphere>	inSpheres	: register(t1);
ByteAddressBuffer				inGrass		: register(t2);
RWByteAddressBuffer				outGrass	: register(u0);


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
	//if (DTid.x > numInstances) return;//@Might not need this
	uint address = DTid.x * 48;
	float3 pos	= asfloat(inGrass.Load3(address + 16));
	float3 windVec = windFromCuboids(pos) + windFromSpheres(pos);
	
	outGrass.Store4(address + 0, inGrass.Load4(address));
	outGrass.Store3(address + 16, asuint(pos));
	outGrass.Store3(address + 28, asuint(windVec));
	outGrass.Store(address + 40, asuint(DTid.x));	//for debugging @ remove in release
	outGrass.Store(address + 44, -1);				//for debugging @ remove in release


	//float3 info = float3(inSpheres[0].radius, inSpheres[0].initalStrength, inSpheres[0].fallOffPow);
	//outGrass.Store4(address + 0, uint4(DTid.x, DTid.y, DTid.z, -1));
	//outGrass.Store3(address + 16, inSpheres[0].pos);
	//outGrass.Store3(address + 28, asuint(info));
	//outGrass.Store2(address + 40, uint2(-1, -1));//for debugging

}
