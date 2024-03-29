#include "RefractionShader.h"
#include <d3dcompiler.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>

RefractionShader::RefractionShader()
{
}

RefractionShader::~RefractionShader()
{
}

void RefractionShader::Initialize(D3DGraphics& gfx)
{
	InitializeShaders(gfx, L"Shaders/RefractionVS.cso", L"Shaders/RefractionPS.cso");
}

void RefractionShader::Bind(D3DGraphics& gfx, int indexCount, XMMATRIX world, XMMATRIX view, XMMATRIX projection, XMFLOAT3 lightDirection, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor, XMFLOAT4 clipPlane, ID3D11ShaderResourceView* texture)
{
	SetShaderParameters(gfx, world, view, projection, lightDirection, ambientColor, diffuseColor, clipPlane, texture);

	BindShaders(gfx, indexCount);
}

void RefractionShader::InitializeShaders(D3DGraphics& gfx, const std::wstring& vsFileName, const std::wstring& psFileName)
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
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	// 정점 셰이더를 위한 상수 버퍼 생성.
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&matrixBufferDesc, NULL, &m_pMatrixBuffer));

	// 픽셀 셰이더에서 사용할 행렬들을 담은 상수 버퍼에 대한 서술자를 세팅.
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	// 픽셀 셰이더를 위한 상수 버퍼 생성.
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&lightBufferDesc, NULL, &m_pLightBuffer));

	D3D11_BUFFER_DESC clipBufferDesc;
	clipBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	clipBufferDesc.ByteWidth = sizeof(ClipPlaneBufferType);
	clipBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	clipBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	clipBufferDesc.MiscFlags = 0;
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&clipBufferDesc, NULL, &m_pClipPlaneBuffer));

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

void RefractionShader::SetShaderParameters(D3DGraphics& gfx, XMMATRIX world, XMMATRIX view, XMMATRIX projection, XMFLOAT3 lightDirection, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor, XMFLOAT4 clipPlane, ID3D11ShaderResourceView* texture)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightBufferType* dataPtr2;
	ClipPlaneBufferType* dataPtr3;
	unsigned int bufferNumber;

	world = XMMatrixTranspose(world);
	view = XMMatrixTranspose(view);
	projection = XMMatrixTranspose(projection);

	/* Matrix buffer */
	INFOMAN(gfx)
	GFX_THROW_INFO(gfx.GetContext()->Map(m_pMatrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	dataPtr->world = world;
	dataPtr->view = view;
	dataPtr->projection = projection;
	gfx.GetContext()->Unmap(m_pMatrixBuffer.Get(), 0);

	bufferNumber = 0;
	gfx.GetContext()->VSSetConstantBuffers(bufferNumber, 1, m_pMatrixBuffer.GetAddressOf());
	/* Matrix buffer */

	/* Light buffer */
	GFX_THROW_INFO(gfx.GetContext()->Map(m_pLightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	dataPtr2 = (LightBufferType*)mappedResource.pData;

	dataPtr2->ambientColor = ambientColor;
	dataPtr2->diffuseColor = diffuseColor;
	dataPtr2->lightDirection = lightDirection;
	gfx.GetContext()->Unmap(m_pLightBuffer.Get(), 0);

	bufferNumber = 0;
	gfx.GetContext()->PSSetConstantBuffers(bufferNumber, 1, m_pLightBuffer.GetAddressOf());
	/* Reflection buffer */

	/* Clip buffer */
	GFX_THROW_INFO(gfx.GetContext()->Map(m_pClipPlaneBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	dataPtr3 = (ClipPlaneBufferType*)mappedResource.pData;

	dataPtr3->clipPlane = clipPlane;
	gfx.GetContext()->Unmap(m_pClipPlaneBuffer.Get(), 0);

	bufferNumber = 1;
	gfx.GetContext()->VSSetConstantBuffers(bufferNumber, 1, m_pClipPlaneBuffer.GetAddressOf());
	/* Clip buffer */

	gfx.GetContext()->PSSetShaderResources(0, 1, &texture);
}

void RefractionShader::BindShaders(D3DGraphics& gfx, int indexCount)
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
