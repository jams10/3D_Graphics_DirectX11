#pragma once

#include <Graphics/D3DGraphics.h>
#include <wrl.h>

class RenderToTexture
{
public:
	RenderToTexture();

	void Initialize(D3DGraphics& gfx, int textureWidth, int textureHeight);

	void SetRenderTarget(D3DGraphics& gfx);
	void ClearRenderTarget(D3DGraphics& gfx, float red, float green, float blue, float alpha);
	ID3D11ShaderResourceView* GetShaderResourceView();

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pRenderTargetTexture;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pShaderResourceView;
};
