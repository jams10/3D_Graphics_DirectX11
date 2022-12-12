#pragma once

#include <Graphics/D3DGraphics.h>
#include <DirectXMath.h>
#include <wrl.h>

using namespace DirectX;

class ShadowShader
{
public:
	ShadowShader();
	~ShadowShader();

	void Initialize(D3DGraphics& gfx);
	void Bind(D3DGraphics& gfx, int indexCount, XMMATRIX world, XMMATRIX view, XMMATRIX projection, XMMATRIX lightView, XMMATRIX lightProjection,
		ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthMapTexture,
		DirectX::XMFLOAT3 lightPosition, DirectX::XMFLOAT4 ambientColor, DirectX::XMFLOAT4 diffuseColor);

private:
	void InitializeShaders(D3DGraphics& gfx, const std::wstring& vsFileName, const std::wstring& psFileName);
	void SetShaderParameters(D3DGraphics& gfx, XMMATRIX world, XMMATRIX view, XMMATRIX projection, XMMATRIX lightView, XMMATRIX lightProjection,
		ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthMapTexture,
		DirectX::XMFLOAT3 lightPosition, DirectX::XMFLOAT4 ambientColor, DirectX::XMFLOAT4 diffuseColor);
	void BindShaders(D3DGraphics& gfx, int indexCount);

private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX lightView;
		XMMATRIX lightProjection;
	};

	struct LightBufferType
	{
		DirectX::XMFLOAT4 ambientColor;
		DirectX::XMFLOAT4 diffuseColor;
	};

	struct LightBufferType2
	{
		DirectX::XMFLOAT3 lightPosition;
		float padding;
	};

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pMatrixBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pLightBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pLightBuffer2;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerStateWrap;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerStateClamp;
};