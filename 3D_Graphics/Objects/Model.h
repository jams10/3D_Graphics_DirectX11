#pragma once

#include<Graphics/D3DGraphics.h>
#include<DirectXMath.h>

class TextureArray;

class Model
{
private:
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 binormal;
	};

	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
		float tx, ty, tz;
		float bx, by, bz;
	};

	struct TempVertexType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	struct VectorType
	{
		float x, y, z;
	};
public:
	Model();
	~Model();

	void Initialize(D3DGraphics& gfx, std::string modelFilePath, 
		            std::string textureFilePath1, std::string textureFilePath2, std::string textureFilePath3);
	void Bind(D3DGraphics& gfx);
	unsigned int GetIndexCount() { return m_indexCount; }
	ID3D11ShaderResourceView** GetTextureArray();
	DirectX::XMMATRIX GetWorldMatrix() const noexcept;
	void SpawnControlWindow() noexcept;

private:
	void InitializeBuffers(D3DGraphics& gfx);
	void BindBuffers(D3DGraphics& gfx);

	void LoadTextures(D3DGraphics& gfx, std::string filePath1, std::string filePath2, std::string filePath3);
	void ReleaseTexture();

	void LoadModel(std::string filePath);
	void ReleaseModel();

	void LoadCustomFile(std::string filePath);
	void LoadObjFile(std::string filePath);

	void Reset();

	void CalculateModelVectors();
	void CalculateTangentBinormal(TempVertexType v1, TempVertexType v2, TempVertexType v3, VectorType& tangent, VectorType& binormal);
	void CalculateNormal(VectorType tangent, VectorType binormal, VectorType& normal);

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
	TextureArray* m_pTextureArray;
	ModelType* m_pModel;
	std::string FileFormat;
	std::wstring FileInfoString;
};