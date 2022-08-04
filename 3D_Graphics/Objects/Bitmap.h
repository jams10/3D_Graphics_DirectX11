#pragma once

#include <Graphics/Texture.h>

class Bitmap
{
public:
	Bitmap();
	~Bitmap();

	void Initialize(D3DGraphics& gfx, int screenWidth, int screenHeight, 
		            std::string textureFilePath, int bitmapWidth, int bitmapHeight);
	void Bind(D3DGraphics& gfx, int positionX, int positionY);
	void Release();

	unsigned int GetIndexCount() { return m_indexCount; }
	ID3D11ShaderResourceView* GetTexture();

private:
	void InitializeBuffers(D3DGraphics& gfx);
	void UpdateBuffers(D3DGraphics& gfx, int positionX, int positionY);
	void BindBuffers(D3DGraphics& gfx);

	void LoadTexture(D3DGraphics& gfx, std::string textureFilePath);
	void ReleaseTexture();

private:
	struct VertexType
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 tex;
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;
	unsigned int m_vertexCount, m_indexCount;
	Texture* m_pTexture;
	int m_screenWidth, m_screenHeight;
	int m_bitmapWidth, m_bitmapHeight;
	int m_previousPosX, m_previousPosY;
};