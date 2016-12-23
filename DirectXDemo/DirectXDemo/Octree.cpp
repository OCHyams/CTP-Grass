#include "Octree.h"
#include "objLoader.h"
#include <memory>
#include <vector>
#include <algorithm>
#include <stack>
#include "Shorthand.h"

Octree::Node* Octree::build(const ObjModel& _model, DirectX::XMVECTOR _position, const DirectX::XMFLOAT3 & _minSize, field::Instance* _instances, int _numInstances, float _minGrassLength)
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

	/*Root starts as a leaf with no children. Translate AABB points.*/
	Node* root = new Node(smallest + _position, largest + _position, nullptr);

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
	std::stack<Node*> stack;
	stack.push(root);
	
	XMVECTOR minSize = LF3(&_minSize); 
	while (!stack.empty())
	{
		//Pop top node
		Node* current = stack.top();
		stack.pop();

		//Copy AABB
		BoundingBox childSize = current->m_AABB;
		//Move to origin
		childSize.Center = { 0, 0, 0 };
		//If node is large enough to contain children
		if (childSize.Contains(minSize))
		{
			//Create children

		}
	}

	return root;
}

void Octree::cleanup(Node* _root)
{
	/*Early out if _root ptr is null*/
	if (_root == nullptr) return;

	std::stack<Node*> tree;
	tree.push(_root);

	/*Depth first traversal of nodes*/
	while (!tree.empty())
	{
		//Pop top node
		Node* current = tree.top();
		tree.pop();

		//Store child nodes
		for (Node* child : current->m_children)
		{
			tree.push(child);
		}

		//Delete the current node
		delete current;
	}
}

void Octree::prune(Node* _root)
{
	/*Early out if _root ptr is null*/
	if (_root == nullptr) return;

	std::stack<Node*> tree;
	tree.push(_root);

	bool complete = false;

	/*Iterate until comepletly pruned*/
	while (!complete)
	{
		complete = true;
		/*Depth first traversal of nodes*/
		while (!tree.empty())
		{
			//Pop top node
			Node* current = tree.top();
			tree.pop();

			//If node is a leaf
			if (current->m_children.empty())
			{
				//If there are no instances of grass
				if (current->m_instances.empty())
				{
					//Iterate at least once more
					complete = false;
					//Remove node from parent, delete it
					current->m_parent->m_children.erase(std::remove_if(
									current->m_parent->m_children.begin(),
									current->m_parent->m_children.end(),
									[current](Node* node)
									{
										if (node == current)
										{
											delete current;
											return true;
										}
										return false;
									}));
				}
			}
			else //If node is not leaf
			{
				//Push children
				for (Node* child : current->m_children)
				{
					tree.push(child);
				}
			}
		}
	}
}

bool Octree::addGrass(Node* _root, const field::Instance& _instance)
{
	/*Early out if _root ptr is null*/
	if (_root == nullptr) return false;

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
			if (current->m_children.empty())
			{
				//Add the grass instance
				current->m_instances.push_back(_instance);
				return true;
			}
			//Else push child nodes
			for (Node* child : current->m_children)
			{
				tree.push(child);
			}
		}
	}

	return false;
}
