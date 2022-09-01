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

	// ���̴� ���� �̸����� ���� ���� ���̴� ����.
	GFX_THROW_INFO(D3DReadFileToBlob(vsFileName.c_str(), &vertexShaderBuffer));
	GFX_THROW_INFO(gfx.GetDevice()->CreateVertexShader(
		vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(),
		nullptr,
		&m_pVertexShader
	));

	// ���̴� ���� �̸����� ���� �ȼ� ���̴� ����.
	GFX_THROW_INFO(D3DReadFileToBlob(psFileName.c_str(), &pixelShaderBuffer));
	GFX_THROW_INFO(gfx.GetDevice()->CreatePixelShader(
		pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(),
		nullptr,
		&m_pPixelShader));

	// �Է� ���̾ƿ� ������ ����. ���̴����� �ùٸ��� �����͸� ���� �� �ֵ��� Model Ŭ�������� �������� ���� ����ü Ÿ�԰� ������� ��.
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

	// �Է� ���̾ƿ��� �ִ� ���ҵ��� ������ ����.
	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// �Է� ���̾ƿ� ����.
	GFX_THROW_INFO(gfx.GetDevice()->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_pLayout));

	// ���� ���̴� ���ۿ� �ȼ� ���̴� ���۴� �� �̻� �ʿ䰡 �����Ƿ� ���� ����.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

	// ���� ���̴����� ����� ��ĵ��� ���� ��� ���ۿ� ���� �����ڸ� ����.
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// ��� ���� ����.
	gfx.GetDevice()->CreateBuffer(&matrixBufferDesc, NULL, &m_pMatrixBuffer);

	// Sampler State ������.
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
	// Sampler State ����.
	GFX_THROW_INFO(gfx.GetDevice()->CreateSamplerState(&samplerDesc, &m_pSamplerState));

	// LightBuffer ����.
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&lightBufferDesc, NULL, &m_pLightBuffer));

	// CameraBuffer ����.
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

	// GPU�� column���� matrix�� �а�, CPU�� row�� matrix�� �б� ������ ���̴� �ʿ��� ���� �� �ֵ��� ����� ��ġ������.
	world = XMMatrixTranspose(world);
	view = XMMatrixTranspose(view);
	projection = XMMatrixTranspose(projection);

	INFOMAN(gfx)
	// ��� ���ۿ� ���� ���� ���� lock.
	GFX_THROW_INFO(gfx.GetContext()->Map(m_pMatrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	// lock�� ���� ���� ���ε� ���ҽ����� ��� ������ �����Ϳ� ���� �����͸� ����.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// ��� ���ۿ� �Լ��� ���ڷ� ���� ��ĵ��� ������ �־���.
	dataPtr->world = world;
	dataPtr->view = view;
	dataPtr->projection = projection;

	// ���� �� �����Ƿ�, ����� ���� �ٽ� Ǯ����. unlock.
	gfx.GetContext()->Unmap(m_pMatrixBuffer.Get(), 0);

	// ���� ���̴������� ��� ������ ��ġ�� ����.
	bufferNumber = 0;

	// ���� ���̴��� �ִ� ��� ���۸� ���ŵ� ������ ��������.
	gfx.GetContext()->VSSetConstantBuffers(bufferNumber, 1, m_pMatrixBuffer.GetAddressOf());

	// Light ��� ���ۿ� ���� ���� ���� gpu �޸𸮸� cpu �޸𸮿� ���� ������.
	gfx.GetContext()->Map(m_pLightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// ���ε� ��� ���ۿ� ���� ������ ����.
	dataPtr2 = (LightBufferType*)mappedResource.pData;

	// ��� ���ۿ� ���� ���� ������ ������ �־���.
	dataPtr2->diffuseColor = diffuse;
	dataPtr2->lightDirection = lightDir;
	dataPtr2->ambientColor = ambientColor;
	dataPtr2->specularColor = specularColor;
	dataPtr2->specularPower = specularPower;

	// ���� �� �����Ƿ� Unlock.
	gfx.GetContext()->Unmap(m_pLightBuffer.Get(), 0);

	// �ȼ� ���̴����� Light ��� ������ ��ȣ�� ������.
	bufferNumber = 0;

	// �ȼ� ���̴��� Light ��� ���۸� ���ε� ����.
	gfx.GetContext()->PSSetConstantBuffers(bufferNumber, 1, m_pLightBuffer.GetAddressOf());

	// Camera ��� ���ۿ� ���� ���� ���� gpu �޸𸮸� cpu �޸𸮿� ���� ������.
	gfx.GetContext()->Map(m_pCameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// ���ε� ��� ���ۿ� ���� ������ ����.
	dataPtr3 = (CameraBufferType*)mappedResource.pData;

	// ��� ���ۿ� ī�޶� ��ġ �־���.
	dataPtr3->cameraPosition = cameraPosition;

	// ���� �� �����Ƿ� Unlock.
	gfx.GetContext()->Unmap(m_pCameraBuffer.Get(), 0);

	// ���� ���̴����� Camera ��� ������ ��ȣ�� ������. (Light ��� ���� ���� ��ȣ ����.)
	bufferNumber = 1;

	// ���� ���̴��� Camera ��� ���۸� ���ε� ����.
	gfx.GetContext()->VSSetConstantBuffers(bufferNumber, 1, m_pCameraBuffer.GetAddressOf());

	// �ȼ� ���̴����� ����� �ؽ��� �ڿ��� ����.
	gfx.GetContext()->PSSetShaderResources(0, 3, textureArray);
}

void SpecularMapShader::BindShader(D3DGraphics& gfx, int indexCount)
{
	// ���� �Է� ���̾ƿ��� ���ε� ����.
	gfx.GetContext()->IASetInputLayout(m_pLayout.Get());

	// ����, �ȼ� ���̴��� ���������ο� ���ε� ����.
	gfx.GetContext()->VSSetShader(m_pVertexShader.Get(), NULL, 0);
	gfx.GetContext()->PSSetShader(m_pPixelShader.Get(), NULL, 0);

	// �ȼ� ���̴��� Sampler State�� ���� ����.
	gfx.GetContext()->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());

	// ������ �׷���.
	gfx.GetContext()->DrawIndexed(indexCount, 0, 0);
}
