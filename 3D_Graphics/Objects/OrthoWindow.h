#include<Graphics/D3DGraphics.h>
#include <DirectXMath.h>

class OrthoWindow
{
public:
	OrthoWindow();
	OrthoWindow(const OrthoWindow&);
	~OrthoWindow();

	void Initialize(D3DGraphics& gfx, int windowWidth, int windowHeight);
	void Bind(D3DGraphics& gfx);

	int GetIndexCount();

private:
	void InitializeBuffers(D3DGraphics& gfx, int windowWidth, int windowHeight);
	void BindBuffers(D3DGraphics& gfx);

private:
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
	};

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;
	int m_vertexCount;
	int m_indexCount;
};