#pragma once
#include <SimpleMath.h>

class RenderData;
/////////////////////////////////////////////////
/// Base class for all game objects used by simulation.
/////////////////////////////////////////////////
class Object
{
public:

	DirectX::XMFLOAT3 m_position;


	//may need for debugging
	//virtual void render(const RenderData&);

private:
protected:

};
