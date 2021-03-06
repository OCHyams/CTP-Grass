#pragma once
#include "RenderInfo.h"
class MeshObject;
class DefaultObjcetRenderInfo final : public RenderInfo
{
public:
	float	m_specularPower = 25;
	void updatePerObjectBuffers(const DrawData&, const MeshObject&)	override;
	void updatePerFrameBuffers(const DrawData&)	override;
}; 
