#include "RenderToTexture.h"
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>

RenderToTexture::RenderToTexture()
{
}

void RenderToTexture::Initialize(D3DGraphics& gfx, int textureWidth, int textureHeight)
{
	INFOMAN(gfx);

	D3D11_TEXTURE2D_DESC textureDesc;
	// ���� Ÿ�� �ؽ��� ������ �ʱ�ȭ.
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	// ���� Ÿ�� �ؽ��� ������ ������ ä����.
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// ���� Ÿ�� ����� �� �ؽ��� ����.
	GFX_THROW_INFO(gfx.GetDevice()->CreateTexture2D(&textureDesc, NULL, &m_pRenderTargetTexture));

	// ���� Ÿ�� �� ������ ���� �� ����.
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	// ���� Ÿ�� �� ����.
	GFX_THROW_INFO(gfx.GetDevice()->CreateRenderTargetView(m_pRenderTargetTexture.Get(), &renderTargetViewDesc, &m_pRenderTargetView));

	// ���̴� ���ҽ� �� ������ ���� �� ����.
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	// ���̴� ���ҽ� �� ����.
	GFX_THROW_INFO(gfx.GetDevice()->CreateShaderResourceView(m_pRenderTargetTexture.Get(), &shaderResourceViewDesc, &m_pShaderResourceView));
}

void RenderToTexture::SetRenderTarget(D3DGraphics& gfx)
{
	gfx.GetContext()->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), gfx.GetDepthStencilView());
}

void RenderToTexture::ClearRenderTarget(D3DGraphics& gfx, float red, float green, float blue, float alpha)
{
	float color[4];

	// �� ���� �ʱ�ȭ ������ ��������.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// �� ���� �ʱ�ȭ.
	gfx.GetContext()->ClearRenderTargetView(m_pRenderTargetView.Get(), color);

	// ���� ���� �ʱ�ȭ.
	gfx.GetContext()->ClearDepthStencilView(gfx.GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}

ID3D11ShaderResourceView* RenderToTexture::GetShaderResourceView()
{
    return m_pShaderResourceView.Get();
}
