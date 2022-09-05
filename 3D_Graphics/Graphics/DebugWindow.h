#pragma once

#include <Graphics/D3DGraphics.h>
#include <wrl.h>
#include <DirectXMath.h>

class DebugWindow
{
private:
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
	};

public:
	DebugWindow();

	void Initialize(D3DGraphics& gfx, int screenWidth, int screenHeight, int bitmapWidth, int bitmapHeight);
	void Render(D3DGraphics& gfx, int positionX, int positionY);

	int GetIndexCount();

private:
	void InitializeBuffers(D3DGraphics& gfx);
	void UpdateBuffers(D3DGraphics& gfx, int positionX, int positionY);
	void RenderBuffers(D3DGraphics& gfx);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;
	int m_vertexCount, m_indexCount;
	int m_screenWidth, m_screenHeight;
	int m_bitmapWidth, m_bitmapHeight;
	int m_previousPosX, m_previousPosY;
};