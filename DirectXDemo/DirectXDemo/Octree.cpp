#include "Octree.h"
#include "objLoader.h"
#include <memory>
#include <vector>
#include <algorithm>
#include <stack>
#include "Shorthand.h"
Octree::Node* Octree::build(const ObjModel& _model, const DirectX::XMFLOAT3& _position, const DirectX::XMFLOAT3 & _minSize, field::Instance* _instances, int _numInstances, float _minGrassLength)
{
	using namespace DirectX;

	/*Calculate the size of the bounding box of the model*/
	float* vtxElePtr	= _model.GetVertices();
	//Set the first vertex as the largest and smallest to begin with
	XMFLOAT3 current	= { *vtxElePtr, *(vtxElePtr + 1), *(vtxElePtr + 2) };
	XMVECTOR smallest	= LF3(&current);
	XMVECTOR largest	= LF3(&current);
	
	for (int i = 0; i < _model.GetTotalVerts(); ++i)
	{
		//Get next three floats, store in XMFLOAT3 current
		memcpy(&current, vtxElePtr, sizeof(float)*3);
		vtxElePtr += 3;
		//Component-wise compare and store
		largest = XMVectorMax(LF3(&current), largest);
		smallest = XMVectorMin(LF3(&current), smallest);
	}

	//Increase height of bounding box to encompass all the grass
	largest += VEC3(0, _minGrassLength, 0);

	XMFLOAT3 min, max;
	XMStoreFloat3(&min, smallest);
	XMStoreFloat3(&max, largest);

	/*Root starts as a leaf*/
	Node* root = new Node(min, max, nullptr, 0);

	/*return if the the bounding box ( size ) < ( _minSize * 8 ) 
	because the tree cannot be broken up any further*/
	if (XMVector3Length(largest - smallest).m128_f32[0] <= XMVector3Length(LF3(&_minSize) * LF3(&XMFLOAT3(8, 8, 8))).m128_f32[0]) return root;

	/*Octree segments*/
	XMFLOAT3 oct[8] =
	{
		{  0.5 ,  0.5 ,  0.5 },
		{ -0.5 ,  0.5 ,  0.5 },
		{ -0.5 ,  0.5 , -0.5 },
		{  0.5 ,  0.5 , -0.5 },
		{  0.5 , -0.5 ,  0.5 },
		{ -0.5 , -0.5 ,  0.5 },
		{ -0.5 , -0.5 , -0.5 },
		{ 0.5 ,  -0.5 , -0.5 }
	};
	
	/*Build child nodes*/


	return root;
}

void Octree::cleanup(Node* _root)
{
	/*Early out if _root ptr is null*/
	if (_root == nullptr) return;

	std::stack<Node*> tree;
	tree.push(_root);

	/*Depth first traversal of nodes*/
	while (tree.size())
	{
		//Pop top node
		Node* current = tree.top();
		tree.pop();

		//Store child nodes
		for (int i = 0; i < 8; ++i)
		{
			Node* child = current->m_children[i];
			if (child) tree.push(child);
		}

		//Delete the current node
		delete current;
	}
}

void Octree::prune(Node* _root)
{
}

bool Octree::addGrass(Node* _root, const field::Instance& _instance)
{
	std::stack<Node*> tree;
	tree.push(_root);

	/*Depth first traversal of nodes*/
	while (tree.size())
	{
		//Pop top node
		Node* current = tree.top();
		tree.pop();

		//If this node contains the grass
		if (current->m_AABB.Contains(LF3(&_instance.location)))
		{
			//If this is a leaf node
			if (current->m_leaf)
			{
				//Add the grass instance
				current->m_instances.push_back(_instance);
				return true;
			}
			//Else push child nodes
			for (int i = 0; i < 8; ++i)
			{
				Node* child = current->m_children[i];
				if (child) tree.push(child);
			}
		}
	}

	return false;
}
