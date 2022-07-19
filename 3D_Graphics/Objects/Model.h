#pragma once

#include<Graphics/D3DGraphics.h>
#include<DirectXMath.h>

class Texture;

class Model
{
public:
	Model();
	~Model();

	void Initialize(D3DGraphics& gfx, std::string filePath);
	void Bind(D3DGraphics& gfx);
	unsigned int GetIndexCount() { return m_indexCount; }
	ID3D11ShaderResourceView* GetTexture();
	DirectX::XMMATRIX GetWorldMatrix() const noexcept;
	void SpawnControlWindow() noexcept;

private:
	void InitializeBuffers(D3DGraphics& gfx);
	void BindBuffers(D3DGraphics& gfx);

	void LoadTexture(D3DGraphics& gfx, std::string filePath);
	void ReleaseTexture();
	void Reset();
private:
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
		DirectX::XMFLOAT3 normal;
	};
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;
	DirectX::XMFLOAT3 pos;
	float pitch;
	float yaw;
	unsigned int m_vertexCount;
	unsigned int m_indexCount;
	Texture* m_pTexture;
};