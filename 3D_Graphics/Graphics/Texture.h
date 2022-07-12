#include <Utils/Surface.h>
#include <Graphics/D3DGraphics.h>
#include <string>
#include <wrl.h>

class Texture
{
public:
	Texture(D3DGraphics& gfx, std::string filePath);
	Texture(const Texture&);
	~Texture();

	ID3D11ShaderResourceView* GetTextureView();

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pTextureView;
};