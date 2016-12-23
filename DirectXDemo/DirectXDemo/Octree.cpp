#include "Octree.h"
#include "objLoader.h"
#include <memory>
#include <vector>
#include <algorithm>
#include <stack>
#include "Shorthand.h"
Octree::Node* Octree::build(const ObjModel& _model, const DirectX::XMFLOAT3& _position, const DirectX::XMFLOAT3 & _minSize, field::Instance * _instances, int _numInstances)
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

	/*Calculate data for root node*/
	Node* root = new Node();
	XMStoreFloat3(&root->m_halfSize, ( largest - smallest ) / 2 );
	XMStoreFloat3(&root->m_pos, largest - LF3(&root->m_halfSize)  + LF3(&_position));

	/*return if the the bounding box ( size / 2 ) < ( _minSize * 8 / 2 ) 
	because the tree cannot be broken up any further*/
	if (XMVector3Length(LF3(&root->m_halfSize)).m128_f32[0] <= XMVector3Length(LF3(&_minSize) * LF3(&XMFLOAT3(4, 4, 4))).m128_f32[0]) return root;

	/*@Generate children*/


	return root;
}

static int numChildren(Octree::Node* _node)
{
	int result = 0;
	for (int i = 0; i < 8; ++i)
	{
		result += static_cast<int>(static_cast<bool>(_node->m_children[i]));
	}
	return result;
}

void Octree::cleanup(Node* _head)
{
	std::stack<Node*>	tree;
	tree.push(_head);

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
