#pragma once

#include <Graphics/D3DGraphics.h>
#include <wrl.h>

class RenderToTexture
{
public:
	RenderToTexture();

	void Initialize(D3DGraphics& gfx, int textureWidth, int textureHeight, float screenDepth, float screenNear);

	void SetRenderTarget(D3DGraphics& gfx);
	void ClearRenderTarget(D3DGraphics& gfx, float red, float green, float blue, float alpha);
	ID3D11ShaderResourceView* GetShaderResourceView();

	void GetProjectionMatrix(DirectX::XMMATRIX& projection);
	void GetOrthoMatrix(DirectX::XMMATRIX& ortho);

	int GetTextureWidth();
	int GetTextureHeight();

private:
	int m_textureWidth, m_textureHeight;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pRenderTargetTexture;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pShaderResourceView;
	Microsoft::WRL::ComPtr <ID3D11Texture2D> m_pDepthStencilBuffer;
	Microsoft::WRL::ComPtr <ID3D11DepthStencilView> m_pDepthStencilView;
	D3D11_VIEWPORT m_viewport;
	DirectX::XMMATRIX m_projectionMatrix;
	DirectX::XMMATRIX m_orthoMatrix;
};
