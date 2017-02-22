#pragma once
#include "RenderInfo.h"
class MeshObject;
class DefaultObjcetRenderInfo final : public RenderInfo
{
public:
	float	m_specularPower = 100.0f;
	int		m_celShaded = 1;
	void updatePerObjectBuffers(const DrawData&, const MeshObject&)	override;
	void updatePerFrameBuffers(const DrawData&)	override;
}; 
