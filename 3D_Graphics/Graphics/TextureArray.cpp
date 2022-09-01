#include "TextureArray.h"
#include <Utils/Surface.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>

TextureArray::TextureArray()
{
    m_pTextures[0] = nullptr;
    m_pTextures[1] = nullptr;
    m_pTextures[2] = nullptr;
}

void TextureArray::Initialize(D3DGraphics& gfx, const std::string& file1, const std::string& file2, const std::string& file3)
{
	MakeTextureView(gfx, file1, &m_pTextures[0]);
	MakeTextureView(gfx, file2, &m_pTextures[1]);
	MakeTextureView(gfx, file2, &m_pTextures[2]);
}

void TextureArray::MakeTextureView(D3DGraphics& gfx, const std::string& file, ID3D11ShaderResourceView** pTextureView)
{
	Surface s = Surface::FromFile(file);

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
		pTexture.Get(), &srvDesc, pTextureView
	));
}

void TextureArray::Shutdown()
{
	if (m_pTextures[0])
	{
		m_pTextures[0]->Release();
		m_pTextures[0] = nullptr;
	}

	if (m_pTextures[1])
	{
		m_pTextures[1]->Release();
		m_pTextures[1] = nullptr;
	}

	if (m_pTextures[2])
	{
		m_pTextures[2]->Release();
		m_pTextures[2] = nullptr;
	}
}

ID3D11ShaderResourceView** TextureArray::GetTextureArray()
{
    return m_pTextures;
}
