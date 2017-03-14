#include "FieldUtilities.h"
#include <random>
#include "Shorthand.h"
void field::generateField(ID3D11Device* _device, ObjModel* _model, float _density, std::vector<field::Instance>& _instances)
{
	float truncationAccumulator = 0;
	float* vertElementPtr = _model->getVertices();
	float* normElementPtr = _model->getNormals();
	unsigned int numVerts = _model->getTotalVerts();

	XMFLOAT3 triVerts[3];
	XMFLOAT3 triNorms[3];

	/*For every triangle, currently testing faster implementation*/
	for (unsigned int i = 0; i < numVerts; i += 3)
	{
		/*Get vert positions*/
		memcpy(triVerts, vertElementPtr, sizeof(float) * 9);
		vertElementPtr += 9;

		/*Get vert normals*/
		if (normElementPtr)
		{
			memcpy(triNorms, normElementPtr, sizeof(float) * 9);
			normElementPtr += 9;
		}

		/*Store vert positions & calc surface area*/
		float sa = triangleSurfaceArea((float*)triVerts);
		/*Calc number of blades*/
		unsigned int numBlades = (int)std::truncf(sa * _density);
		/*Deal with trunc rounding accumulation*/
		truncationAccumulator += std::fmodf(_density, sa);
		if (truncationAccumulator >= 1.0f)
		{
			++numBlades;
			--truncationAccumulator;
		}

		_instances.resize(_instances.size() + numBlades);
		/*Add the patch to the field and octree*/
		addPatch(_instances,(float*)triVerts, numBlades);
	}
}

float field::triangleSurfaceArea(float * verts)
{
	using namespace DirectX;
	/*Calc surface area*/
	XMVECTOR a = VEC3(*(verts), *(verts + 1), *(verts + 2));
	XMVECTOR b = VEC3(*(verts + 3), *(verts + 4), *(verts + 5));
	XMVECTOR c = VEC3(*(verts + 6), *(verts + 7), *(verts + 8));
	XMVECTOR sa = 0.5 * XMVector3Length(XMVector3Cross((a - c), (b - c)));
	float val;
	XMStoreFloat(&val, sa);
	return val;
}

void field::addPatch(std::vector<field::Instance>& _field, float* verts, unsigned int _numBlades)
{
	using namespace DirectX;

	//RNG
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0, 1);
	std::uniform_real_distribution<float> angleDistribution(0, 2 * XM_PI);
	auto rand = std::bind(distribution, generator);
	auto randAngle = std::bind(angleDistribution, generator);

	XMVECTOR a = VEC3(*(verts), *(verts + 1), *(verts + 2));
	XMVECTOR b = VEC3(*(verts + 3), *(verts + 4), *(verts + 5));
	XMVECTOR c = VEC3(*(verts + 6), *(verts + 7), *(verts + 8));

	//Use barycentric coordinates [Orthmann] to place grass
	for (unsigned int i = 0; i < _numBlades; ++i)
	{
		field::Instance instance;

		/*Barycentric coords*/ //See paper and orthman for barycentric sampling
		float u = rand();
		float v = rand();

		//http://cgg.mff.cuni.cz/~jaroslav/papers/2013-meshsampling/2013-meshsampling-paper.pdf
		//https://classes.soe.ucsc.edu/cmps160/Fall10/resources/barycentricInterpolation.pdf
		//http://math.stackexchange.com/questions/18686/uniform-random-point-in-triangle
		//http://chrischoy.github.io/research/barycentric-coordinate-for-mesh-sampling/ 
		XMVECTOR translation = (1 - sqrt(u)) * a +
			sqrt(u) * (1 - v) * b +
			sqrt(u) * v * c;

		/*Rotation*/
		float angle = randAngle();
		XMVECTOR euler = XMLoadFloat3(&XMFLOAT3(0, angle, 0));
		XMVECTOR quat = XMQuaternionRotationRollPitchYawFromVector(euler);
		XMStoreFloat4(&instance.rotation, quat);

		/*Translation*/
		XMStoreFloat3(&instance.location, translation);
		instance.wind = { 0 ,0, 0 };
		_field.push_back(instance);
	}
}
