#include "Texture.h"
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>

Texture::Texture(D3DGraphics& gfx, std::string filePath)
{
	Surface s = Surface::FromFile(filePath);

	// 텍스쳐 리소스 서술자.
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = s.GetWidth();
	textureDesc.Height = s.GetHeight();
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// 텍스쳐 자원 서브 리소스.
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = s.GetBufferPtr();
	sd.SysMemPitch = s.GetWidth() * sizeof(Surface::Color);

	INFOMAN(gfx)
	// 텍스쳐 자원 생성.
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture;
	GFX_THROW_INFO(gfx.GetDevice()->CreateTexture2D(
		&textureDesc, &sd, &pTexture
	));

	// 텍스쳐 리소스 뷰 생성.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	GFX_THROW_INFO(gfx.GetDevice()->CreateShaderResourceView(
		pTexture.Get(), &srvDesc, &m_pTextureView
	));
}


Texture::Texture(const Texture& other)
{
}


Texture::~Texture()
{
}

ID3D11ShaderResourceView* Texture::GetTextureView()
{
	return m_pTextureView.Get();
}