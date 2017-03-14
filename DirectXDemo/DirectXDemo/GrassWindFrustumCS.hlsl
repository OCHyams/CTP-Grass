struct Instance
{
	float4 rot;
	float3 pos;
	float3 wind;
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
//To-Do
struct Frustum
{
	

};

cbuffer CBChangesPerFrameRW
{
	uint numInstances;
};

cbuffer CBChangePerFrameRO
{
	uint numCuboids;
	uint numSpheres;
	float time;
	float deltaTime;
	Frustum viewFrustum;
};

cbuffer CBNeverChanges
{
	uint maxInstances;
}

uniform float4 translationFrequency = float4(1.975, 0.793, 0.375, 0.193);
uniform uint SIZE_OF_INSTANCE = 42 /*Raw*/ + 6 /*Padding*/;
uniform uint ROT_OFFSET = 0;
uniform uint POS_OFFSET = 16;
uniform uint WIND_OFFSET = 28;
uniform float LERP_SPEED = 10;
ByteAddressBuffer				inGrass		: register(t0);
StructuredBuffer<WindCuboid>	inCuboids	: register(t1);
StructuredBuffer<WindSphere>	inSpheres	: register(t2);
//Only way I can think of doing this...
uniform uint FRONT_ADDRESS_IDX = 0, BACK_ADDRESS_IDX = 32;
RWStructuredBuffer<uint>		addresses	: register(t3);
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

inline float4 smoothf(float4 x)
{
	return x * x * (3.0 - 2.0 * x);
}
inline float4 trianglef(float4 x)
{
	return abs(frac(x + 0.5) * 2.0 - 1.0);
}
inline float4 windForce(float3 p, float3 windVec)
{
	float phase = (time * length(windVec)) + dot(normalize(windVec), p); 
	float4 ts = smoothf(trianglef(translationFrequency * phase));
	return float4(windVec * dot(ts, 0.25), 0.0f);
}
//To-Do
bool inViewFrustum(float3 pos)
{
	return true;
}

[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{

	uint fromAddress = DTid.x * SIZE_OF_INSTANCE;
	float3 pos = asfloat(inGrass.Load3(fromAddress + POS_OFFSET));
	float3 currentWindVec = asfloat(outGrass.Load3(fromAddress + WIND_OFFSET));
	float3 newWindVec = windFromCuboids(pos) + windFromSpheres(pos);
	newWindVec = windForce(pos, newWindVec);
	newWindVec = lerp(currentWindVec, newWindVec, deltaTime * LERP_SPEED);

	uint toAddress = 0;
	if (inViewFrustum(pos))
	{
		toAddress = addresses[FRONT_ADDRESS_IDX] * SIZE_OF_INSTANCE;
		InterlockedAdd(addresses[FRONT_ADDRESS_IDX], 1);
	}
	else
	{
		toAddress = addresses[BACK_ADDRESS_IDX] * SIZE_OF_INSTANCE;
		InterlockedAdd(addresses[BACK_ADDRESS_IDX], -1);
	}

	outGrass.Store4(toAddress + 0, inGrass.Load4(fromAddress));
	outGrass.Store3(toAddress + 16, asuint(pos));
	outGrass.Store3(toAddress + 28, asuint(newWindVec));
}
