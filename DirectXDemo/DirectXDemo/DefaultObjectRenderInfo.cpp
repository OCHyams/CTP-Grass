#include "DefaultObjectRenderInfo.h"
#include "ConstantBuffers.h"
#include "Camera.h"
#include "Shorthand.h"
#include "Renderer.h"
void DefaultObjcetRenderInfo::updatePerObjectBuffers(const DrawData& _drawData, const MeshObject& _object)
{
	CBDefaultObject_ChangesPerObject cbuffer;
	cbuffer.m_worldPos = _object.getPos();
	cbuffer.m_specularPow = m_specularPower;
	cbuffer.m_worldViewProj = XMLoadFloat4x4(&_object.getWVPTranspose());
	_drawData.m_dc->UpdateSubresource(m_vsBuffers.at(0), 0, 0, &cbuffer, 0 , 0);
}

void DefaultObjcetRenderInfo::updatePerFrameBuffers(const DrawData& _drawData)
{
	CBDefaultObject_ChangesPerFrame cbuffer;
	cbuffer.m_cameraPos = _drawData.m_cam->getPos();
	cbuffer.m_lightPos = DirectX::XMFLOAT3(0, 3, 0);
	cbuffer.m_intensity = 3;
	_drawData.m_dc->UpdateSubresource(m_vsBuffers.at(1), 0, 0, &cbuffer, 0, 0);
}
