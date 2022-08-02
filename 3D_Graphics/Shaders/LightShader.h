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
	void Bind(D3DGraphics& gfx, int indexCount, XMMATRIX world, XMMATRIX view, XMMATRIX projection, XMFLOAT3 cameraPosition,
			  ID3D11ShaderResourceView* texture, DirectX::XMFLOAT4 ambientColor, DirectX::XMFLOAT4 diffuseColor, DirectX::XMFLOAT3 lightDirection,
			  XMFLOAT4 specularColor, float specularPower);

private:
	void InitializeShaders(D3DGraphics& gfx, const std::wstring& vsFileName, const std::wstring& psFileName);
	void SetShaderParameters(D3DGraphics& gfx, XMMATRIX world, XMMATRIX view, XMMATRIX projection, XMFLOAT3 cameraPosition,
		                     ID3D11ShaderResourceView* texture, DirectX::XMFLOAT4 ambientColor, DirectX::XMFLOAT4 diffuseColor, DirectX::XMFLOAT3 lightDirection,
						     XMFLOAT4 specularColor, float specularPower);
	void BindShaders(D3DGraphics& gfx, int indexCount);

private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct CameraBufferType
	{
		XMFLOAT3 cameraPosition;
		float    padding;
	};

	struct LightBufferType
	{
		DirectX::XMFLOAT4 ambientColor;
		DirectX::XMFLOAT4 diffuseColor;
		DirectX::XMFLOAT3 lightDirection;
		float			  specularPower;
		DirectX::XMFLOAT4 specularColor;
	};

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pMatrixBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pCameraBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pLightBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerState;
};