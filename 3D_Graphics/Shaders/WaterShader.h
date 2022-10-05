#pragma once

#include <Graphics/D3DGraphics.h>
#include <DirectXMath.h>
#include <wrl.h>

using namespace DirectX;

class WaterShader
{
public:
	WaterShader();
	~WaterShader();

	void Initialize(D3DGraphics& gfx);
	void Bind(D3DGraphics& gfx, int indexCount, XMMATRIX world, XMMATRIX view, XMMATRIX projection, XMMATRIX reflection,
		ID3D11ShaderResourceView* reflectionTexture, ID3D11ShaderResourceView* refractionTexture,
		ID3D11ShaderResourceView* normalTexture, float waterTranslation, float reflectRefractScale);

private:
	void InitializeShaders(D3DGraphics& gfx, const std::wstring& vsFileName, const std::wstring& psFileName);
	void SetShaderParameters(D3DGraphics& gfx, XMMATRIX world, XMMATRIX view, XMMATRIX projection, XMMATRIX reflection,
		ID3D11ShaderResourceView* reflectionTexture, ID3D11ShaderResourceView* refractionTexture,
		ID3D11ShaderResourceView* normalTexture, float waterTranslation, float reflectRefractScale);
	void BindShaders(D3DGraphics& gfx, int indexCount);

private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};
	struct ReflectionBufferType
	{
		XMMATRIX reflection;
	};
	struct WaterBufferType
	{
		float waterTranslation;
		float reflectRefractionScale;
		DirectX::XMFLOAT2 padding;
	};

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pLayout;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerState;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pMatrixBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pReflectionBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pWaterBuffer;
};