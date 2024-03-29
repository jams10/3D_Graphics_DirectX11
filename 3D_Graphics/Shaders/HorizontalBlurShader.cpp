#include "HorizontalBlurShader.h"
#include <d3dcompiler.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>

HorizontalBlurShader::HorizontalBlurShader()
{
}

HorizontalBlurShader::~HorizontalBlurShader()
{
}

void HorizontalBlurShader::Initialize(D3DGraphics& gfx)
{
	InitializeShaders(gfx, L"Shaders/HorizontalBlurVS.cso", L"Shaders/HorizontalBlurPS.cso");
}

void HorizontalBlurShader::Bind(D3DGraphics& gfx, int indexCount, XMMATRIX world, XMMATRIX view, XMMATRIX projection, ID3D11ShaderResourceView* texture, float screenWidth)
{
	// 월드, 뷰, 투영 행렬 설정해줌.
	SetShaderParameters(gfx, world, view, projection, texture, screenWidth);

	// 파이프라인에 자원들을 바인딩.
	BindShaders(gfx, indexCount);
}

void HorizontalBlurShader::InitializeShaders(D3DGraphics& gfx, const std::wstring& vsFileName, const std::wstring& psFileName)
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
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
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

	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	gfx.GetDevice()->CreateBuffer(&matrixBufferDesc, NULL, &m_pMatrixBuffer);

	D3D11_BUFFER_DESC screenSizeBufferDesc;
	screenSizeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	screenSizeBufferDesc.ByteWidth = sizeof(ScreenSizeBufferType);
	screenSizeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	screenSizeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	screenSizeBufferDesc.MiscFlags = 0;
	screenSizeBufferDesc.StructureByteStride = 0;
	gfx.GetDevice()->CreateBuffer(&matrixBufferDesc, NULL, &m_pScreenSizeBuffer);

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

void HorizontalBlurShader::SetShaderParameters(D3DGraphics& gfx, XMMATRIX world, XMMATRIX view, XMMATRIX projection, ID3D11ShaderResourceView* texture, float screenWidth)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	ScreenSizeBufferType* dataPtr2;
	unsigned int bufferNumber;

	// GPU는 column으로 matrix를 읽고, CPU는 row로 matrix를 읽기 때문에 셰이더 쪽에서 읽을 수 있도록 행렬을 전치시켜줌.
	world = XMMatrixTranspose(world);
	view = XMMatrixTranspose(view);
	projection = XMMatrixTranspose(projection);

	INFOMAN(gfx)
	// 변환 행렬 상수 버퍼.
	GFX_THROW_INFO(gfx.GetContext()->Map(m_pMatrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = world;
	dataPtr->view = view;
	dataPtr->projection = projection;

	gfx.GetContext()->Unmap(m_pMatrixBuffer.Get(), 0);

	bufferNumber = 0;
	gfx.GetContext()->VSSetConstantBuffers(bufferNumber, 1, m_pMatrixBuffer.GetAddressOf());

	// 스크린 너비 상수 버퍼.
	GFX_THROW_INFO(gfx.GetContext()->Map(m_pScreenSizeBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	dataPtr2 = (ScreenSizeBufferType*)mappedResource.pData;

	dataPtr2->screenWidth = screenWidth;
	dataPtr2->padding = XMFLOAT3(0, 0, 0);

	gfx.GetContext()->Unmap(m_pScreenSizeBuffer.Get(), 0);

	bufferNumber = 1;
	gfx.GetContext()->VSSetConstantBuffers(bufferNumber, 1, m_pScreenSizeBuffer.GetAddressOf());

	gfx.GetContext()->PSSetShaderResources(0, 1, &texture);
}

void HorizontalBlurShader::BindShaders(D3DGraphics& gfx, int indexCount)
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
