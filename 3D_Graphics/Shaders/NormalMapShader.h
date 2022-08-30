#pragma once

#include <Graphics/D3DGraphics.h>
#include <DirectXMath.h>
#include <wrl.h>

using namespace DirectX;

class NormalMapShader
{
public:
	NormalMapShader();

	void Initialize(D3DGraphics& gfx);
	void Bind(D3DGraphics& gfx, int indexCount, XMMATRIX world, XMMATRIX view, XMMATRIX projection,
		XMFLOAT4 diffuse, XMFLOAT3 lightDir, ID3D11ShaderResourceView** textureArray);

private:
	void InitializeShaders(D3DGraphics& gfx, const std::wstring& vsFileName, const std::wstring& psFileName);

	void SetShaderParameters(D3DGraphics& gfx, XMMATRIX world, XMMATRIX view, XMMATRIX projection,
		XMFLOAT4 diffuse, XMFLOAT3 lightDir, ID3D11ShaderResourceView** textureArray);
	void BindShader(D3DGraphics& gfx, int indexCount);

private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct LightBufferType
	{
		XMFLOAT4 diffuseColor;
		XMFLOAT3 lightDirection;
		float padding;
	};

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pMatrixBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pLightBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerState;
};