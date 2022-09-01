#include "SpecularMapShader.h"
#include <d3dcompiler.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>

SpecularMapShader::SpecularMapShader()
{
}

void SpecularMapShader::Initialize(D3DGraphics& gfx)
{
    InitializeShader(gfx, L"Shaders/SpecularMapVS.cso", L"Shaders/SpecularMapPS.cso");
}

void SpecularMapShader::Bind(D3DGraphics& gfx, int indexCount, XMMATRIX world, XMMATRIX view, XMMATRIX projection, XMFLOAT4 diffuse, XMFLOAT3 lightDir, XMFLOAT3 cameraPosition, XMFLOAT4 ambientColor, XMFLOAT4 specularColor, float specularPower, ID3D11ShaderResourceView** textureArray)
{
    SetShaderParameters(gfx, world, view, projection, diffuse, lightDir, cameraPosition, ambientColor, specularColor, specularPower, textureArray);

    BindShader(gfx, indexCount);
}

void SpecularMapShader::InitializeShader(D3DGraphics& gfx, const std::wstring& vsFileName, const std::wstring& psFileName)
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
	D3D11_INPUT_ELEMENT_DESC polygonLayout[5];
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

	polygonLayout[3].SemanticName = "TANGENT";
	polygonLayout[3].SemanticIndex = 0;
	polygonLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[3].InputSlot = 0;
	polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[3].InstanceDataStepRate = 0;

	polygonLayout[4].SemanticName = "BINORMAL";
	polygonLayout[4].SemanticIndex = 0;
	polygonLayout[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[4].InputSlot = 0;
	polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[4].InstanceDataStepRate = 0;

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

	// 상수 버퍼 생성.
	gfx.GetDevice()->CreateBuffer(&matrixBufferDesc, NULL, &m_pMatrixBuffer);

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

	// LightBuffer 생성.
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&lightBufferDesc, NULL, &m_pLightBuffer));

	// CameraBuffer 생성.
	D3D11_BUFFER_DESC cameraBufferDesc;
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&cameraBufferDesc, NULL, &m_pCameraBuffer));
}

void SpecularMapShader::SetShaderParameters(D3DGraphics& gfx, XMMATRIX world, XMMATRIX view, XMMATRIX projection, XMFLOAT4 diffuse, XMFLOAT3 lightDir, XMFLOAT3 cameraPosition, XMFLOAT4 ambientColor, XMFLOAT4 specularColor, float specularPower, ID3D11ShaderResourceView** textureArray)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightBufferType* dataPtr2;
	CameraBufferType* dataPtr3;
	unsigned int bufferNumber;

	// GPU는 column으로 matrix를 읽고, CPU는 row로 matrix를 읽기 때문에 셰이더 쪽에서 읽을 수 있도록 행렬을 전치시켜줌.
	world = XMMatrixTranspose(world);
	view = XMMatrixTranspose(view);
	projection = XMMatrixTranspose(projection);

	INFOMAN(gfx)
	// 상수 버퍼에 값을 쓰기 위해 lock.
	GFX_THROW_INFO(gfx.GetContext()->Map(m_pMatrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	// lock을 통해 얻어온 매핑된 리소스에서 상수 버퍼의 데이터에 대한 포인터를 얻어옴.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// 상수 버퍼에 함수의 인자로 들어온 행렬들을 복사해 넣어줌.
	dataPtr->world = world;
	dataPtr->view = view;
	dataPtr->projection = projection;

	// 값을 다 썼으므로, 잠궈준 것을 다시 풀어줌. unlock.
	gfx.GetContext()->Unmap(m_pMatrixBuffer.Get(), 0);

	// 정점 셰이더에서의 상수 버퍼의 위치를 설정.
	bufferNumber = 0;

	// 정점 셰이더에 있는 상수 버퍼를 갱신된 값으로 설정해줌.
	gfx.GetContext()->VSSetConstantBuffers(bufferNumber, 1, m_pMatrixBuffer.GetAddressOf());

	// Light 상수 버퍼에 값을 쓰기 위해 gpu 메모리를 cpu 메모리와 매핑 시켜줌.
	gfx.GetContext()->Map(m_pLightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// 매핑된 상수 버퍼에 대한 포인터 얻어옴.
	dataPtr2 = (LightBufferType*)mappedResource.pData;

	// 상수 버퍼에 광원 변수 값들을 복사해 넣어줌.
	dataPtr2->diffuseColor = diffuse;
	dataPtr2->lightDirection = lightDir;
	dataPtr2->ambientColor = ambientColor;
	dataPtr2->specularColor = specularColor;
	dataPtr2->specularPower = specularPower;

	// 값을 다 썼으므로 Unlock.
	gfx.GetContext()->Unmap(m_pLightBuffer.Get(), 0);

	// 픽셀 셰이더에서 Light 상수 버퍼의 번호를 지정함.
	bufferNumber = 0;

	// 픽셀 셰이더에 Light 상수 버퍼를 바인딩 해줌.
	gfx.GetContext()->PSSetConstantBuffers(bufferNumber, 1, m_pLightBuffer.GetAddressOf());

	// Camera 상수 버퍼에 값을 쓰기 위해 gpu 메모리를 cpu 메모리와 매핑 시켜줌.
	gfx.GetContext()->Map(m_pCameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// 매핑된 상수 버퍼에 대한 포인터 얻어옴.
	dataPtr3 = (CameraBufferType*)mappedResource.pData;

	// 상수 버퍼에 카메라 위치 넣어줌.
	dataPtr3->cameraPosition = cameraPosition;

	// 값을 다 썼으므로 Unlock.
	gfx.GetContext()->Unmap(m_pCameraBuffer.Get(), 0);

	// 정점 셰이더에서 Camera 상수 버퍼의 번호를 지정함. (Light 상수 버퍼 다음 번호 지정.)
	bufferNumber = 1;

	// 정점 셰이더에 Camera 상수 버퍼를 바인딩 해줌.
	gfx.GetContext()->VSSetConstantBuffers(bufferNumber, 1, m_pCameraBuffer.GetAddressOf());

	// 픽셀 셰이더에서 사용할 텍스쳐 자원을 설정.
	gfx.GetContext()->PSSetShaderResources(0, 3, textureArray);
}

void SpecularMapShader::BindShader(D3DGraphics& gfx, int indexCount)
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
