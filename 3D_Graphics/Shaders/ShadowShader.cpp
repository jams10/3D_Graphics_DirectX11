#include "ShadowShader.h"
#include <d3dcompiler.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>

ShadowShader::ShadowShader()
{
}

ShadowShader::~ShadowShader()
{
}

void ShadowShader::Initialize(D3DGraphics& gfx)
{
	InitializeShaders(gfx, L"Shaders/ShadowVS.cso", L"Shaders/ShadowPS.cso");
}

void ShadowShader::Bind(D3DGraphics& gfx, int indexCount, XMMATRIX world, XMMATRIX view, XMMATRIX projection, XMMATRIX lightView, XMMATRIX lightProjection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthMapTexture, DirectX::XMFLOAT3 lightPosition, DirectX::XMFLOAT4 ambientColor, DirectX::XMFLOAT4 diffuseColor)
{
	SetShaderParameters(gfx, world, view, projection, lightView, lightProjection, texture, depthMapTexture, lightPosition, ambientColor, diffuseColor);
	BindShaders(gfx, indexCount);
}

void ShadowShader::InitializeShaders(D3DGraphics& gfx, const std::wstring& vsFileName, const std::wstring& psFileName)
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
	matrixBufferDesc.StructureByteStride = 0;
	// 정점 셰이더를 위한 상수 버퍼 생성.
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&matrixBufferDesc, NULL, &m_pMatrixBuffer));

	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;	
	lightBufferDesc.StructureByteStride = 0;
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&lightBufferDesc, NULL, &m_pLightBuffer));

	D3D11_BUFFER_DESC lightBufferDesc2;
	lightBufferDesc2.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc2.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc2.MiscFlags = 0;
	lightBufferDesc2.StructureByteStride = 0;
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&lightBufferDesc2, NULL, &m_pLightBuffer2));

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
	GFX_THROW_INFO(gfx.GetDevice()->CreateSamplerState(&samplerDesc, &m_pSamplerStateWrap));

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	GFX_THROW_INFO(gfx.GetDevice()->CreateSamplerState(&samplerDesc, &m_pSamplerStateClamp));
}

void ShadowShader::SetShaderParameters(D3DGraphics& gfx, XMMATRIX world, XMMATRIX view, XMMATRIX projection, XMMATRIX lightView, XMMATRIX lightProjection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthMapTexture, DirectX::XMFLOAT3 lightPosition, DirectX::XMFLOAT4 ambientColor, DirectX::XMFLOAT4 diffuseColor)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightBufferType* dataPtr2;
	LightBufferType2* dataPtr3;
	int bufferNumber = 0;

	world = DirectX::XMMatrixTranspose(world);
	view = DirectX::XMMatrixTranspose(view);
	projection = DirectX::XMMatrixTranspose(projection);
	lightView = DirectX::XMMatrixTranspose(lightView);
	lightProjection = DirectX::XMMatrixTranspose(lightProjection);

	INFOMAN(gfx)
	GFX_THROW_INFO(gfx.GetContext()->Map(m_pMatrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	// lock을 통해 얻어온 매핑된 리소스에서 상수 버퍼의 데이터에 대한 포인터를 얻어옴.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// 상수 버퍼에 함수의 인자로 들어온 행렬들을 복사해 넣어줌.
	dataPtr->world = world;
	dataPtr->view = view;
	dataPtr->projection = projection;
	dataPtr->lightView = lightView;
	dataPtr->lightProjection = lightProjection;

	// 값을 다 썼으므로, 잠궈준 것을 다시 풀어줌. unlock.
	gfx.GetContext()->Unmap(m_pMatrixBuffer.Get(), 0);

	// 정점 셰이더에서의 상수 버퍼의 위치를 설정.
	bufferNumber = 0;

	// 정점 셰이더에 있는 상수 버퍼를 갱신된 값으로 설정해줌.
	gfx.GetContext()->VSSetConstantBuffers(bufferNumber, 1, m_pMatrixBuffer.GetAddressOf());

	// 픽셀 셰이더에서 사용할 텍스쳐 자원을 설정.
	gfx.GetContext()->PSSetShaderResources(0, 1, &texture);
	gfx.GetContext()->PSSetShaderResources(1, 1, &depthMapTexture);

	// Light 상수 버퍼에 값을 쓰기 위해 gpu 메모리를 cpu 메모리와 매핑 시켜줌.
	gfx.GetContext()->Map(m_pLightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// 매핑된 상수 버퍼에 대한 포인터 얻어옴.
	dataPtr2 = (LightBufferType*)mappedResource.pData;

	// 상수 버퍼에 광원 변수 값들을 복사해 넣어줌.
	dataPtr2->ambientColor = ambientColor;
	dataPtr2->diffuseColor = diffuseColor;

	// 값을 다 썼으므로 Unlock.
	gfx.GetContext()->Unmap(m_pLightBuffer.Get(), 0);

	// 픽셀 셰이더에서 Light 상수 버퍼의 번호를 지정함.
	bufferNumber = 0;

	// 픽셀 셰이더에 Light 상수 버퍼를 바인딩 해줌.
	gfx.GetContext()->PSSetConstantBuffers(bufferNumber, 1, m_pLightBuffer.GetAddressOf());

	gfx.GetContext()->Map(m_pLightBuffer2.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr3 = (LightBufferType2*)mappedResource.pData;
	dataPtr3->lightPosition = lightPosition;
	dataPtr3->padding = 0.f;

	gfx.GetContext()->Unmap(m_pLightBuffer2.Get(), 0);
	bufferNumber = 1;

	gfx.GetContext()->VSSetConstantBuffers(bufferNumber, 1, m_pLightBuffer2.GetAddressOf());
}

void ShadowShader::BindShaders(D3DGraphics& gfx, int indexCount)
{
	// 정점 입력 레이아웃을 바인딩 해줌.
	gfx.GetContext()->IASetInputLayout(m_pLayout.Get());

	// 정점, 픽셀 셰이더를 파이프라인에 바인딩 해줌.
	gfx.GetContext()->VSSetShader(m_pVertexShader.Get(), NULL, 0);
	gfx.GetContext()->PSSetShader(m_pPixelShader.Get(), NULL, 0);

	// 픽셀 셰이더에 Sampler State를 설정 해줌.
	gfx.GetContext()->PSSetSamplers(0, 1, m_pSamplerStateWrap.GetAddressOf());
	gfx.GetContext()->PSSetSamplers(1, 1, m_pSamplerStateClamp.GetAddressOf());

	// 도형을 그려줌.
	gfx.GetContext()->DrawIndexed(indexCount, 0, 0);
}
