struct Instance
{
	float4 rot;
	float3 pos;
	float3 wind;
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
	uint numSpheres;
	uint numCuboids;
};

StructuredBuffer<Instance>		inGrass		: register(t0);
StructuredBuffer<WindCuboid>	inCuboids	: register(t1);
StructuredBuffer<WindSphere>	inSpheres	: register(t2);
RWStructuredBuffer<Instance>	outGrass	: register(u0);

Instance applyCuboids(Instance input)
{
	for (uint i = 0; i < numCuboids; ++i)
	{
		WindCuboid cuboid = inCuboids[i];
		float3 min = cuboid.pos - cuboid.extents;
		float3 max = cuboid.pos + cuboid.extents;

		if ((input.pos.x >= min.x && input.pos.x <= max.x) &&
			(input.pos.y >= min.y && input.pos.y <= max.y) &&
			(input.pos.z >= min.z && input.pos.z <= max.z))
		{
			input.wind += cuboid.velocity;
		}
	}

	return input;
}

Instance applySpheres(Instance input)
{
	for (uint i = 0; i < numSpheres; ++i)
	{
		WindSphere sphere = inSpheres[i];
		float3 vec = input.pos - sphere.pos;
		float distance = length(vec);

		if (distance <= sphere.radius)
		{
			vec = normalize(vec);
			input.wind += (sphere.initalStrength - (pow(distance / sphere.radius, sphere.fallOffPow) * sphere.initalStrength)) * vec;
		}
	}

	return input;
}

[numthreads(768, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	//Copy instance
	Instance output = inGrass[DTid.x];
	output.wind = 0;

	output = applyCuboids(output);
	output = applySpheres(output);

	outGrass[DTid.x] = output;
}


