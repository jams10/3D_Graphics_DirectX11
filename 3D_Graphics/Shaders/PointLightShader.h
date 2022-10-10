#pragma once

#include <Graphics/D3DGraphics.h>
#include <DirectXMath.h>
#include <wrl.h>

using namespace DirectX;

const int NUM_LIGHTS = 4;

class PointLightShader
{
public:
	PointLightShader();
	~PointLightShader();

	void Initialize(D3DGraphics& gfx);
	void Bind(D3DGraphics& gfx, int indexCount, XMMATRIX world, XMMATRIX view, XMMATRIX projection,
		ID3D11ShaderResourceView* texture, XMFLOAT4 diffuseColors[4], XMFLOAT4 lightPositions[4]);

private:
	void InitializeShaders(D3DGraphics& gfx, const std::wstring& vsFileName, const std::wstring& psFileName);
	void SetShaderParameters(D3DGraphics& gfx, XMMATRIX world, XMMATRIX view, XMMATRIX projection,
		ID3D11ShaderResourceView* texture, XMFLOAT4 diffuseColors[4], XMFLOAT4 lightPositions[4]);
	void BindShaders(D3DGraphics& gfx, int indexCount);

private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct LightColorBufferType
	{
		DirectX::XMFLOAT4 diffuseColor[NUM_LIGHTS];
	};

	struct LightPositionBufferType
	{
		DirectX::XMFLOAT4 lightPosition[NUM_LIGHTS];
	};

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pLayout;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerState;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pMatrixBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pLightColorBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pLightPositionBuffer;
};