#include "Octree.h"
#include "objLoader.h"
#include <memory>
#include "Shorthand.h"
Octree::Node* Octree::build(const ObjModel& _model, const DirectX::XMFLOAT2 & _minSize, field::Instance * _instances, int _numInstances)
{
	using namespace DirectX;

	/*Calculate the size of the bounding box of the model*/
	float* vtxElePtr = _model.GetVertices();
	//Set the first vertex as the largest and smallest to begin with
	XMFLOAT3 current = { *vtxElePtr, *(vtxElePtr + 1), *(vtxElePtr + 2) };
	XMVECTOR smallest = LF3(&current);
	XMVECTOR largest = LF3 (&current);
	
	for (int i = 0; i < _model.GetTotalVerts(); ++i)
	{
		//Get next three floats, store in XMFLOAT3 current
		memcpy(&current, vtxElePtr, sizeof(float)*3);
		vtxElePtr += 3;

		largest = XMVectorMax(LF3(&current), largest);
		smallest = XMVectorMin(LF3(&current), smallest);
	}

	XMFLOAT3 smallf3;
	XMFLOAT3 largef3;

	XMStoreFloat3(&smallf3, smallest);
	XMStoreFloat3(&largef3, largest);


	return nullptr;
}

void Octree::cleanup(Node * _head)
{
}
