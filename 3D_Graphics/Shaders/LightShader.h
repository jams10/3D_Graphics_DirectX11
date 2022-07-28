#pragma once

#include <Graphics/D3DGraphics.h>
#include <DirectXMath.h>
#include <wrl.h>

using namespace DirectX;

class LightShader
{
public:
	LightShader();
	~LightShader();

	void Initialize(D3DGraphics& gfx);
	void Bind(D3DGraphics& gfx, int indexCount, XMMATRIX world, XMMATRIX view, XMMATRIX projection, 
			  ID3D11ShaderResourceView* texture, DirectX::XMFLOAT4 ambientColor, DirectX::XMFLOAT4 diffuseColor, DirectX::XMFLOAT3 lightDirection);

private:
	void InitializeShaders(D3DGraphics& gfx, const std::wstring& vsFileName, const std::wstring& psFileName);
	void SetShaderParameters(D3DGraphics& gfx, XMMATRIX world, XMMATRIX view, XMMATRIX projection,
		                     ID3D11ShaderResourceView* texture, DirectX::XMFLOAT4 ambientColor, DirectX::XMFLOAT4 diffuseColor, DirectX::XMFLOAT3 lightDirection);
	void BindShaders(D3DGraphics& gfx, int indexCount);

private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};
	struct LightBufferType
	{
		DirectX::XMFLOAT4 ambientColor;
		DirectX::XMFLOAT4 diffuseColor;
		DirectX::XMFLOAT3 lightDirection;
		float padding; // 16 바이트 정렬을 위한 패딩 값.
	};

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pMatrixBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pLightBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerState;
};