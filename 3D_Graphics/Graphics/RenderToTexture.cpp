#include "RenderToTexture.h"
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>

#define PI 3.141592

RenderToTexture::RenderToTexture()
{
}

void RenderToTexture::Initialize(D3DGraphics& gfx, int textureWidth, int textureHeight, float screenDepth, float screenNear)
{
	INFOMAN(gfx);

	m_textureWidth = textureWidth;
	m_textureHeight = textureHeight;

	D3D11_TEXTURE2D_DESC textureDesc;
	// 렌더 타겟 텍스쳐 서술자 초기화.
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	// 렌더 타겟 텍스쳐 서술자 내용을 채워줌.
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

	// 렌더 타겟 대상이 될 텍스쳐 생성.
	GFX_THROW_INFO(gfx.GetDevice()->CreateTexture2D(&textureDesc, NULL, &m_pRenderTargetTexture));

	// 렌더 타겟 뷰 서술자 생성 및 설정.
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	// 렌더 타겟 뷰 생성.
	GFX_THROW_INFO(gfx.GetDevice()->CreateRenderTargetView(m_pRenderTargetTexture.Get(), &renderTargetViewDesc, &m_pRenderTargetView));

	// 셰이더 리소스 뷰 서술자 생성 및 설정.
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	// 셰이더 리소스 뷰 생성.
	GFX_THROW_INFO(gfx.GetDevice()->CreateShaderResourceView(m_pRenderTargetTexture.Get(), &shaderResourceViewDesc, &m_pShaderResourceView));

	// 깊이 버퍼 서술자 설정.
	D3D11_TEXTURE2D_DESC depthBufferDesc = {};
	depthBufferDesc.Width = textureWidth;              
	depthBufferDesc.Height = textureHeight;
	depthBufferDesc.MipLevels = 1u;
	depthBufferDesc.ArraySize = 1u;
	depthBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthBufferDesc.SampleDesc.Count = 1u;
	depthBufferDesc.SampleDesc.Quality = 0u;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	// 깊이 스텐실 버퍼 생성.
	GFX_THROW_INFO(gfx.GetDevice()->CreateTexture2D(&depthBufferDesc, nullptr, &m_pDepthStencilBuffer));

	// 깊이 스텐실 뷰 서술자 설정.
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0u;

	// 깊이 스텐실 뷰 생성.
	GFX_THROW_INFO(gfx.GetDevice()->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), &depthStencilViewDesc, &m_pDepthStencilView));

	// 렌더링을 위한 뷰 포트 설정.
	m_viewport.Width = (float)textureWidth;
	m_viewport.Height = (float)textureHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

	// 투영 행렬 설정.
	m_projectionMatrix = dx::XMMatrixPerspectiveFovLH(PI / 4.0f, ((float)textureWidth / (float)textureHeight), screenNear, screenDepth);

	// 2D 렌더링을 위한 직교 투영 행렬 생성.
	m_orthoMatrix = dx::XMMatrixOrthographicLH((float)textureWidth, (float)textureHeight, screenNear, screenDepth);
}

void RenderToTexture::SetRenderTarget(D3DGraphics& gfx)
{
	gfx.GetContext()->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

	// 뷰포트 설정.
	gfx.GetContext()->RSSetViewports(1, &m_viewport);
}

void RenderToTexture::ClearRenderTarget(D3DGraphics& gfx, float red, float green, float blue, float alpha)
{
	float color[4];

	// 백 버퍼 초기화 색상을 지정해줌.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// 백 버퍼 초기화.
	gfx.GetContext()->ClearRenderTargetView(m_pRenderTargetView.Get(), color);

	// 깊이 버퍼 초기화.
	gfx.GetContext()->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}

ID3D11ShaderResourceView* RenderToTexture::GetShaderResourceView()
{
    return m_pShaderResourceView.Get();
}

void RenderToTexture::GetProjectionMatrix(DirectX::XMMATRIX& projection)
{
	projection = m_projectionMatrix;
}

void RenderToTexture::GetOrthoMatrix(DirectX::XMMATRIX& ortho)
{
	ortho = m_orthoMatrix;
}

int RenderToTexture::GetTextureWidth()
{
	return m_textureWidth;
}

int RenderToTexture::GetTextureHeight()
{
	return m_textureHeight;
}
