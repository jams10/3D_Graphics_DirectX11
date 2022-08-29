#pragma once

#include <Graphics/D3DGraphics.h>

class TextureArray
{
public:
	TextureArray();

	void Initialize(D3DGraphics& gfx, const std::string& file1, const std::string& file2, const std::string& file3);
	void MakeTextureView(D3DGraphics& gfx, const std::string& file, ID3D11ShaderResourceView** pTexture);
	void Shutdown();

	ID3D11ShaderResourceView** GetTextureArray();

private:
	ID3D11ShaderResourceView* m_pTextures[3];
};
