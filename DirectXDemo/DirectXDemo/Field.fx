//CONSTANT BUFFERS-----------------------------------------------------------------

cbuffer CONSTS : register(b0)
{	
	float	halfGrassWidth; //this should be in another buffer that never needs to be updated....
	float	time;
	float	minTessDensity;//this should be in another buffer that never needs to be updated....
	float	maxTessDensity;//this should be in another buffer that never needs to be updated....
	float	nearTess;//this should be in another buffer that never needs to be updated....
	float	farTess;//this should be in another buffer that never needs to be updated....
};

cbuffer CBViewProj : register (b1)
{
	matrix view_proj;
}

cbuffer CBLight : register (b2)
{
	float4	camera; //alignment doesn't play nice with float3s
	float4	lightDir;
	float4	ambient;
	float4  diffuse;
	float4	specular;
	float	shiny;
}
//INPUT/OUTPUT STRUCTS-----------------------------------------------------------------
struct VS_INPUT
{
	//PER_VERTEX
	float3	pos			: SV_POSITION;			//position
	float3	binormal	: BINORMAL;				//Helps place the quad verts
	float3	normal		: NORMAL;
	float	flexibility	: FLEX;					//multiplier for wind deformation
	//PER-INSTANCE
	//matrix	world		: INSTANCE_WORLD;		//world transform matrix - Not in use atm, might be required later...
	float4	rotation	: INSTANCE_ROTATION;	//quaternion rotation
	float3	location	: INSTANCE_LOCATION;	//position of grass in world for lighting calc etc
	float3	wind		: INSTANCE_WIND;
};

struct HS_DS_INPUT
{
	float4	cpoint		: POSITION;
	float4	binormal	: BINORMAL;
	float4	b1			: BINORMAL_WORLDPOS0;
	float4	b2			: BINORMAL_WORLDPOS1;
	float3	normal		: NORMAL;
	float3	basePos		: INSTANCE_LOCATION;
	float	tessDensity : TESS_DENSITY;
};

struct HS_CONSTANT_OUTPUT
{
	float edges[2] : SV_TessFactor;
};

//struct HS_OUTPUT
//{
//	float4	cpoint		: CPOINT;
//	float4	binormal	: BINORMAL;
//	float4	b1			: BINORMAL_WORLDPOS0;
//	float4	b2			: BINORMAL_WORLDPOS1;
//	float3	normal		: NORMAL;
//};

struct GS_INPUT
{
	//float4	position	: SV_Position;
	float4	binormal	: BINORMAL;
	float4	b1			: BINORMAL_WORLDPOS0;
	float4	b2			: BINORMAL_WORLDPOS1;
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

const static float4 translationFrequency = float4(1.975, 0.793, 0.375, 0.193);

//VERTEX SHADER----------------------------------------------------------
//Orthoman & GPU gems
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
	// Compute the phase shift for the position p with respect to
	// the current wind strength and direction
	//float phase = (time * length(windVec)) + dot(windVec, p);*/ //HAD TO MODIFY THIS SO BECAUSE PER_INSTANCE wind vectors can now change suddenly!

	//				@@version below works@@
	float phase = (time *length(windVec)) + dot(normalize(windVec), p);
	//float phase = (time + length(p))  * 0.3f;  //<<<-- This kind of works but you don't get increased frequency /w big winds which looks odd...

	// Compute the four translation strengths.
	float4 ts = smoothf(trianglef(translationFrequency * phase));
	// Compute the mean of the four values and
	// return the translation vector.
	return float4(windVec * dot(ts, 0.25), 0.0f);
}

//http://donw.io/post/dual-quaternion-skinning/
inline float3 quatRotateVector(float4 Qr, float3 v)
{
	return v + 2 * cross(Qr.w * v + cross(v, Qr.xyz), Qr.xyz);
}

//http://stackoverflow.com/questions/1171849/finding-quaternion-representing-the-rotation-from-one-vector-to-another
inline float4 quatFromTwoVec(float3 v0, float3 v1)
{
	float4 q;
	v0 = cross(v0, v1);
	q.xyz = v0;
	q.w = sqrt(pow(length(v0) , 2) * pow(length(v1),2)) + dot(v0, v1);
	return normalize(q);
}

inline float3 project(float3 v0, float3 v1)
{
	return v1* dot(v0, v1) / pow(length(v1), 2);
}


float4 quatMul(float4 q1, float4 q2)
{
	float4 result;
	result.x = q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x;
	result.y = -q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y;
	result.z = q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z;
	result.w = -q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w;
	return result;
}

//https://en.wikipedia.org/wiki/Slerp
float4 slerp(float4 v0, float4 v1, double t) {
	// Only unit quaternions are valid rotations.
	// Normalize to avoid undefined behavior.
	normalize(v0);
	normalize(v1);

	// Compute the cosine of the angle between the two vectors.
	float _dot = dot(v0, v1);

	const float DOT_THRESHOLD = 0.9995;
	if (_dot > DOT_THRESHOLD) {
		// If the inputs are too close for comfort, linearly interpolate
		// and normalize the result.
		//float4 r =v0 + t * (v1 – v0);
		float4 r = v0 + t * (v1 - v0);
		return normalize(r);
	}

	// If the dot product is negative, the quaternions
	// have opposite handed-ness and slerp won't take
	// the shorter path. Fix by reversing one quaternion.
	if (_dot < 0.0f) {
		v1 = -v1;
		_dot = -_dot;
	}

	clamp(_dot, -1, 1);           // Robustness: Stay within domain of acos()
	float theta_0 = acos(_dot);  // theta_0 = angle between input vectors
	float theta = theta_0*t;    // theta = angle between v0 and result 

	float4 v2 = v1-v0*_dot;
	normalize(v2);              // { v0, v2 } is now an orthonormal basis

	return v0*cos(theta) + v2*sin(theta);
}


float4 OCHWind(float3 vertex, float3 pos, float3 wind, float flex)
{

	float phase = (time *length(wind)) + dot(normalize(wind), pos);
	//float phase = (time + length(pos))  * 0.3f;  //<<<-- This kind of works but you don't get increased frequency /w big winds which looks odd...nah doesn't work :/

	// Compute the four translation strengths.
	float4 ts = smoothf(trianglef(translationFrequency * phase));
	// Compute the mean of the four values and
	// return the translation vector.
	float4 displacement = float4(wind * dot(ts, 0.25), 0.0f) ;

	float len = 0.0001;// length(displacement);
	displacement.xy *= flex;
	displacement.xyz = normalize(displacement.xyz) * len;

	//displacement.y = 0;
	//float height =  clamp(-length(displacement), -vertex.y, 0);
	//displacement.y = height;

	return displacement;
}

HS_DS_INPUT VS_Main(VS_INPUT vertex)
{
	HS_DS_INPUT output;

	//output.cpoint = float4(vertex.pos, 1.f);//@working
	output.cpoint = float4(quatRotateVector(vertex.rotation, vertex.pos), 1.0f);//@new

	/*Wind displacement*/ //[Orthomans technique]
	float4 windDisplacement = (windForce(vertex.location, vertex.wind) * vertex.flexibility);
	output.cpoint += windDisplacement;

	/*Rotation of vectors under wind force*/ //-@->NOT WORKING ATM
	float4 rot = quatFromTwoVec(normalize(vertex.pos), normalize(output.cpoint.xyz));



	/*Binormals*/
	output.binormal = float4(quatRotateVector(vertex.rotation, vertex.binormal),0.f);
	/*/experimental twisting
	float4 twistingQuat = quatFromTwoVec(output.binormal, cross(vertex.wind, float3(0, 1, 0)));
	float phase = (time *length(vertex.wind)) + dot(normalize(vertex.wind), vertex.location);
	float4 ts = smoothf(trianglef(translationFrequency * phase));
	twistingQuat *= vertex.flexibility * ts;
	output.binormal = float4(quatRotateVector(twistingQuat, output.binormal.xyz), 0.f);
	/*/
	output.binormal = float4(normalize(output.binormal.xyz), 0);

	//@when better wind simulation is in, use twisting to manipulate normal
	/*Normals*/
	//output.normal = quatRotateVector(quatMul(rot,vertex.rotation), vertex.normal);
	output.normal = quatRotateVector(vertex.rotation, vertex.normal);
	/*experimental twisting - I think this works but the twisting calc is bad
	output.normal = float4(quatRotateVector(twistingQuat, output.normal.xyz), 0.f);
	//*/
	output.normal = normalize(output.normal);

	///*Quad verts*///@working
	//output.b1 = output.cpoint + binormal * (1 - (pow(vertex.flexibility, 2))) * halfGrassWidth;
	//output.b2 = output.cpoint - binormal * (1 - (pow(vertex.flexibility, 2))) * halfGrassWidth;

	/*Quad verts*///@new
	output.b1 = float4(vertex.location, 0.0f) + output.cpoint + output.binormal * (1 - (pow(vertex.flexibility, 2))) * halfGrassWidth;
	output.b2 = float4(vertex.location, 0.0f) + output.cpoint - output.binormal * (1 - (pow(vertex.flexibility, 2))) * halfGrassWidth;

	/*View-Proj transformation*///@new
	output.cpoint = mul(output.cpoint, view_proj);
	output.b1 = mul(output.b1, view_proj);
	output.b2 = mul(output.b2, view_proj);

	///*World-View-Proj transformation*///@working
	//matrix wvp = mul(vertex.world, view_proj);
	//output.cpoint = mul(output.cpoint, wvp);

	///*Put quad verts into world space*/
	//output.b1 = mul(output.b1, wvp);
	//output.b2 = mul(output.b2, wvp);

	/*Tess factor*/ //@@Could move this out to the compute shader as it shouldn't need to be calculated 4 times!
	float distance = length(camera.xyz - vertex.location);
	distance = clamp((distance- nearTess)/(farTess - nearTess), 0, 1);
	output.tessDensity = maxTessDensity - (distance * (maxTessDensity - minTessDensity));

	output.basePos = vertex.location;

	return output;
}

//HULL-DOMAIN SHADERS----------------------------------------------------
//See http://gpuexperiments.blogspot.co.uk/2010/02/tessellation-example.html
HS_CONSTANT_OUTPUT HSConst(InputPatch<HS_DS_INPUT, 4> input, uint id : SV_PrimitiveID)
{
	HS_CONSTANT_OUTPUT output;

	output.edges[0] = 1.0f;						// Detail factor
	output.edges[1] = input[id].tessDensity;		// tess density
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

//@Need to find some optimisation here...
//can probably vectorise this?
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

	
	/*Position = evaluateBezier(output.tVal)*/
	//output.position = co[0] * op[0].cpoint + co[1] * op[1].cpoint + co[2] * op[2].cpoint + co[3] * op[3].cpoint;

	/*Normal = evaluateBezier(output.tVal)*/
	output.normal = co[0] * op[0].normal + co[1] * op[1].normal + co[2] * op[2].normal + co[3] * op[3].normal;

	/*Biormal = evaluateBezier(output.tVal)*/
	output.binormal = co[0] * op[0].binormal + co[1] * op[1].binormal + co[2] * op[2].binormal + co[3] * op[3].binormal;

	/*Quad verts*/
	output.b1 = co[0] * op[0].b1 + co[1] * op[1].b1 + co[2] * op[2].b1 + co[3] * op[3].b1; //<<@@VECTORIZING potential, that's a 4-vector dot, matrix?
	output.b2 = co[0] * op[0].b2 + co[1] * op[1].b2 + co[2] * op[2].b2 + co[3] * op[3].b2;

	output.basePos = op[0].basePos;

	return output;
}


//GEOMETRY SHADER--------------------------------------------------------
[maxvertexcount(4)]
void GS_Main(line GS_INPUT input[2], inout TriangleStream<PS_INPUT> output)
{	
	/*for each point in the line segment*/
	for (uint i = 0; i < 2; i++)
	{
		/*create a vertex to output*/
		PS_INPUT element;

		element.basePos = input[i].basePos;

		/*texture coords*/
		element.texcoord = float2(0, 1-input[i].tVal);
		
		/*Element position*/
		element.pos = input[i].b1;

		/*Lighting*/
		element.viewVec = normalize(camera.xyz - element.pos.xyz);
		element.lightVec = normalize(lightDir.xyz - element.pos.xyz);
		element.normal = input[i].normal;

		/*output the vertex*/
		output.Append(element);
		
		/*texture coords for other side*/
		element.texcoord.x = 1;
		
		/*Element position*/
		element.pos = input[i].b2;

		/*Lighting*/
		element.viewVec = normalize(camera.xyz - element.pos.xyz);
		element.lightVec = normalize(-lightDir.xyz - element.pos.xyz);

		/*output the vertex*/
		output.Append(element);
	}
}

float3 rand(uint3 seed)
{
	// Xorshift algorithm from George Marsaglia's paper
	seed ^= (seed << 13);
	seed ^= (seed >> 17);
	seed ^= (seed << 5);

	float3 rnd = float3(seed * (1.0 / 4294967296.0));
	return rnd;
}

Texture2D TEX_0;
SamplerState SAMPLER_STATE;
//PIXEL SHADER-----------------------------------------------------------
float4 PS_Main(PS_INPUT input) : SV_TARGET
{
	///*Lighting*/
	//float3 ambientColour = 0.2f;

	////@Trying to do twoside shading
	//float diffuseTerm1 = clamp(dot(input.normal, input.lightVec), 0.0f, 1.0f);
	//float diffuseTerm2 = clamp(dot(-input.normal, input.lightVec), 0.0f, 1.0f);
	//float diffuseTerm = diffuseTerm1;
	//float3 normal = input.normal;

	//if (diffuseTerm1 < diffuseTerm2)
	//{
	//	normal *= -1;
	//	diffuseTerm = diffuseTerm2;
	//	ambientColour = 0.2;
	//}
	//
	//float specularTerm = 0;
	////@@for now do this but maybe get rid of the if later!
	////if (diffuseTerm > 0.0f)
	////{
	//	float specularPow = 4;
	//	specularTerm = pow(saturate(dot(/*input.*/normal, normalize(input.lightVec + input.viewVec))), specularPow);
	////}
	//float3 final = ambientColour + intensity * diffuseTerm + intensity * specularTerm;
	///*NOTE: remove tex coords to test just lighting*/
	//return float4(final * TEX_0.Sample(SAMPLER_STATE, input.texcoord), 1.0f);

	//Hey this looks a bit better... very similar to all the other implementations except i sat down and worked through the Phong model to see what was gonig on.
	float3 LdotN = dot(lightDir.xyz, input.normal);
	float3 R = 2 * LdotN * input.normal - lightDir.xyz;
	float3 highlight = clamp(specular * pow(dot(R, input.viewVec), shiny),0 , 1);
	float3 illumination = ambient + (diffuse * LdotN) + highlight;
	//Back faces
	if (LdotN.x <= 0)
	{
		//*/
		LdotN = dot(lightDir.xyz, -input.normal);
		R = 2 * LdotN * -input.normal - lightDir.xyz;
		highlight = clamp(specular * pow(dot(R, input.viewVec), shiny), 0, 1);
		illumination = ambient + (diffuse * LdotN) + highlight;
		//*/
	}

	/*Add some colour variation*/
	float3 colour = TEX_0.Sample(SAMPLER_STATE, input.texcoord);
	colour += rand(asuint(input.basePos)) * 0.1f;

	return float4(illumination * colour, 1.0f);
}