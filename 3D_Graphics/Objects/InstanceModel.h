#pragma once

#include<Graphics/D3DGraphics.h>
#include<DirectXMath.h>

class Texture;

class InstanceModel
{
private:
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
	};

	struct InstanceType
	{
		DirectX::XMFLOAT3 position;
	};

	struct ModelType
	{
		float x, y, z;
		float tu, tv;
	};
public:
	InstanceModel();
	~InstanceModel();

	void Initialize(D3DGraphics& gfx, std::string modelFilePath, std::string textureFilePath);
	void Bind(D3DGraphics& gfx);

	unsigned int GetVertexCount();
	unsigned int GetIndexCount();
	unsigned int GetInstanceCount();
	ID3D11ShaderResourceView* GetTexture();

private:
	void InitializeBuffers(D3DGraphics& gfx);
	void BindBuffers(D3DGraphics& gfx);

	void LoadTexture(D3DGraphics& gfx, std::string filePath);
	void ReleaseTexture();

	void LoadModel(std::string filePath);
	void ReleaseModel();

	void LoadCustomFile(std::string filePath);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pInstanceBuffer;
	unsigned int m_vertexCount;
	unsigned int m_indexCount;
	unsigned int m_instanceCount;
	VertexType* m_pVertices;
	unsigned long* m_pIndices;
	ModelType* m_pModel;
	Texture* m_Texture;
	std::string FileFormat;
	std::wstring FileInfoString;
};