#pragma once

#include<Graphics/D3DGraphics.h>
#include<DirectXMath.h>

class TS_TestModel
{
public:
	TS_TestModel();
	~TS_TestModel();

	void Initialize(D3DGraphics& gfx);
	void Bind(D3DGraphics& gfx);
	unsigned int GetIndexCount() { return m_indexCount; }

private:
	void InitializeBuffers(D3DGraphics& gfx);
	void BindBuffers(D3DGraphics& gfx);

private:
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;
	unsigned int m_vertexCount;
	unsigned int m_indexCount;
};