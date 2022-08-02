#include "LightShader.h"
#include <d3dcompiler.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>

LightShader::LightShader()
{
}

LightShader::~LightShader()
{
}

void LightShader::Initialize(D3DGraphics& gfx)
{
	InitializeShaders(gfx, L"Shaders/SpecularLightingVS.cso", L"Shaders/SpecularLightingPS.cso");
}

void LightShader::Bind(D3DGraphics& gfx, int indexCount, XMMATRIX world, XMMATRIX view, XMMATRIX projection, XMFLOAT3 cameraPosition,
					  ID3D11ShaderResourceView* texture, DirectX::XMFLOAT4 ambientColor, DirectX::XMFLOAT4 diffuseColor, DirectX::XMFLOAT3 lightDirection,
					  XMFLOAT4 specularColor, float specularPower)
{
	// 월드, 뷰, 투영 행렬 설정, 텍스쳐, 광원 색상 및 방향 설정.
	SetShaderParameters(gfx, world, view, projection, cameraPosition, texture, ambientColor, diffuseColor, lightDirection, specularColor, specularPower);

	// 파이프라인에 자원들을 바인딩.
	BindShaders(gfx, indexCount);
}

void LightShader::InitializeShaders(D3DGraphics& gfx, const std::wstring& vsFileName, const std::wstring& psFileName)
{
	INFOMAN(gfx);
	ID3DBlob* vertexShaderBuffer;
	ID3DBlob* pixelShaderBuffer;

	// 셰이더 파일 이름으로 부터 정점 셰이더 생성.
	GFX_THROW_INFO(D3DReadFileToBlob(vsFileName.c_str(), &vertexShaderBuffer));
	GFX_THROW_INFO(gfx.GetDevice()->CreateVertexShader(
		vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(),
		nullptr,
		&m_pVertexShader
	));

	// 셰이더 파일 이름으로 부터 픽셀 셰이더 생성.
	GFX_THROW_INFO(D3DReadFileToBlob(psFileName.c_str(), &pixelShaderBuffer));
	GFX_THROW_INFO(gfx.GetDevice()->CreatePixelShader(
		pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(),
		nullptr,
		&m_pPixelShader));

	// 입력 레이아웃 서술자 설정. 셰이더에서 올바르게 데이터를 읽을 수 있도록 Model 클래스에서 설정해준 정점 구조체 타입과 맞춰줘야 함.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	// 입력 레이아웃에 있는 원소들의 개수를 얻어옴.
	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// 입력 레이아웃 생성.
	GFX_THROW_INFO(gfx.GetDevice()->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_pLayout));

	// 정점 셰이더 버퍼와 픽셀 셰이더 버퍼는 더 이상 필요가 없으므로 해제 해줌.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

	// 정점 셰이더에서 사용할 행렬들을 담은 상수 버퍼에 대한 서술자를 세팅.
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = 0;
	matrixBufferDesc.MiscFlags = 0;
	// 정점 셰이더를 위한 상수 버퍼 생성.
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&matrixBufferDesc, NULL, &m_pMatrixBuffer));

	D3D11_BUFFER_DESC cameraBufferDesc;
	cameraBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = 0;
	cameraBufferDesc.MiscFlags = 0;
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&cameraBufferDesc, NULL, &m_pCameraBuffer));

	// 픽셀 셰이더에서 사용할 행렬들을 담은 상수 버퍼에 대한 서술자를 세팅.
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = 0;
	lightBufferDesc.MiscFlags = 0;
	// 픽셀 셰이더를 위한 상수 버퍼 생성.
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&lightBufferDesc, NULL, &m_pLightBuffer));

	// Sampler State 서술자.
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	// Sampler State 생성.
	GFX_THROW_INFO(gfx.GetDevice()->CreateSamplerState(&samplerDesc, &m_pSamplerState));
}

void LightShader::SetShaderParameters(D3DGraphics& gfx, XMMATRIX world, XMMATRIX view, XMMATRIX projection, XMFLOAT3 cameraPosition,
				                      ID3D11ShaderResourceView* texture, DirectX::XMFLOAT4 ambientColor, DirectX::XMFLOAT4 diffuseColor, DirectX::XMFLOAT3 lightDirection,
									  XMFLOAT4 specularColor, float specularPower)
{
	MatrixBufferType cb1;
	CameraBufferType cb2;
	LightBufferType cb3;
	int bufferNumber = 0;

	cb1.world = DirectX::XMMatrixTranspose(world);
	cb1.view = DirectX::XMMatrixTranspose(view);
	cb1.projection = DirectX::XMMatrixTranspose(projection);

	gfx.GetContext()->UpdateSubresource(m_pMatrixBuffer.Get(), 0, NULL, &cb1, sizeof(cb1), 0);
	gfx.GetContext()->VSSetConstantBuffers(bufferNumber, 1, m_pMatrixBuffer.GetAddressOf());

	cb2.cameraPosition = cameraPosition;
	cb2.padding = 0.0f;

	gfx.GetContext()->UpdateSubresource(m_pCameraBuffer.Get(), 0, NULL, &cb2, sizeof(cb2), 0);
	gfx.GetContext()->VSSetConstantBuffers(bufferNumber + 1, 1, m_pCameraBuffer.GetAddressOf());

	gfx.GetContext()->PSSetShaderResources(0, 1, &texture);

	cb3.ambientColor = ambientColor;
	cb3.diffuseColor = diffuseColor;
	cb3.lightDirection = lightDirection;
	cb3.specularColor = specularColor;
	cb3.specularPower = specularPower;

	gfx.GetContext()->UpdateSubresource(m_pLightBuffer.Get(), 0, NULL, &cb3, 0, 0);
	gfx.GetContext()->PSSetConstantBuffers(bufferNumber, 1, m_pLightBuffer.GetAddressOf());
}

void LightShader::BindShaders(D3DGraphics& gfx, int indexCount)
{
	// 정점 입력 레이아웃을 바인딩 해줌.
	gfx.GetContext()->IASetInputLayout(m_pLayout.Get());

	// 정점, 픽셀 셰이더를 파이프라인에 바인딩 해줌.
	gfx.GetContext()->VSSetShader(m_pVertexShader.Get(), NULL, 0);
	gfx.GetContext()->PSSetShader(m_pPixelShader.Get(), NULL, 0);

	// 픽셀 셰이더에 Sampler State를 설정 해줌.
	gfx.GetContext()->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());

	// 도형을 그려줌.
	gfx.GetContext()->DrawIndexed(indexCount, 0, 0);
}
