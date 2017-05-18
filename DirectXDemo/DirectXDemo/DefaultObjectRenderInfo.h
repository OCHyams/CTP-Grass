/*----------------------------------------------------------------
Author:			Orlando Cazalet-Hyams
Description :	Render info for default objects.
----------------------------------------------------------------*/
#pragma once
#include "RenderInfo.h"
class MeshObject;
class DefaultObjcetRenderInfo final : public RenderInfo
{
public:
	float	m_specularPower = 0;
	int		m_lit = 0;
	void updatePerObjectBuffers(const DrawData&, const MeshObject&)	override;
	void updatePerFrameBuffers(const DrawData&)	override;
}; 
