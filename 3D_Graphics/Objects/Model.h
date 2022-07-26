#pragma once

#include<Graphics/D3DGraphics.h>
#include<DirectXMath.h>

class Texture;

class Model
{
public:
	Model();
	~Model();

	void Initialize(D3DGraphics& gfx, std::string modelFilePath, std::string textureFilePath);
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

	void LoadModel(std::string filePath);
	void ReleaseModel();

	void LoadCustomFile(std::string filePath);
	void LoadObjFile(std::string filePath);

	void Reset();

private:
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
		DirectX::XMFLOAT3 normal;
	};

	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;
	DirectX::XMFLOAT3 pos;
	float pitch;
	float yaw;
	unsigned int m_vertexCount;
	unsigned int m_indexCount;
	VertexType* m_pVertices;
	unsigned long* m_pIndices;
	Texture* m_pTexture;
	ModelType* m_pModel;
	std::string FileFormat;
	std::wstring FileInfoString;
};