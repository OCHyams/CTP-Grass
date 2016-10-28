#pragma once
#include "GameObject.h"
#include "basicVertex.h"
#include <d3d11_2.h>
class ShaderDemoObject : public GameObject
{
public:
	ShaderDemoObject();
	virtual ~ShaderDemoObject();

	virtual bool load(ID3D11Device* _device) override;
	virtual void unload() override;

	virtual void update() override;
	virtual void draw(ID3D11DeviceContext* _dc) override;

protected:
	ID3D11VertexShader* m_vs;
	ID3D11PixelShader* m_ps;
	ID3D11GeometryShader* m_gs;

	ID3D11InputLayout* m_inputLayout;
	ID3D11Buffer* m_vertexBuffer;
};
