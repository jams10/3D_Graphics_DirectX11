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
	// ���� ���̴��� ���� ��� ���� ����.
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

	// lock�� ���� ���� ���ε� ���ҽ����� ��� ������ �����Ϳ� ���� �����͸� ����.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// ��� ���ۿ� �Լ��� ���ڷ� ���� ��ĵ��� ������ �־���.
	dataPtr->world = world;
	dataPtr->view = view;
	dataPtr->projection = projection;
	dataPtr->lightView = lightView;
	dataPtr->lightProjection = lightProjection;

	// ���� �� �����Ƿ�, ����� ���� �ٽ� Ǯ����. unlock.
	gfx.GetContext()->Unmap(m_pMatrixBuffer.Get(), 0);

	// ���� ���̴������� ��� ������ ��ġ�� ����.
	bufferNumber = 0;

	// ���� ���̴��� �ִ� ��� ���۸� ���ŵ� ������ ��������.
	gfx.GetContext()->VSSetConstantBuffers(bufferNumber, 1, m_pMatrixBuffer.GetAddressOf());

	// �ȼ� ���̴����� ����� �ؽ��� �ڿ��� ����.
	gfx.GetContext()->PSSetShaderResources(0, 1, &texture);
	gfx.GetContext()->PSSetShaderResources(1, 1, &depthMapTexture);

	// Light ��� ���ۿ� ���� ���� ���� gpu �޸𸮸� cpu �޸𸮿� ���� ������.
	gfx.GetContext()->Map(m_pLightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// ���ε� ��� ���ۿ� ���� ������ ����.
	dataPtr2 = (LightBufferType*)mappedResource.pData;

	// ��� ���ۿ� ���� ���� ������ ������ �־���.
	dataPtr2->ambientColor = ambientColor;
	dataPtr2->diffuseColor = diffuseColor;

	// ���� �� �����Ƿ� Unlock.
	gfx.GetContext()->Unmap(m_pLightBuffer.Get(), 0);

	// �ȼ� ���̴����� Light ��� ������ ��ȣ�� ������.
	bufferNumber = 0;

	// �ȼ� ���̴��� Light ��� ���۸� ���ε� ����.
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
	// ���� �Է� ���̾ƿ��� ���ε� ����.
	gfx.GetContext()->IASetInputLayout(m_pLayout.Get());

	// ����, �ȼ� ���̴��� ���������ο� ���ε� ����.
	gfx.GetContext()->VSSetShader(m_pVertexShader.Get(), NULL, 0);
	gfx.GetContext()->PSSetShader(m_pPixelShader.Get(), NULL, 0);

	// �ȼ� ���̴��� Sampler State�� ���� ����.
	gfx.GetContext()->PSSetSamplers(0, 1, m_pSamplerStateWrap.GetAddressOf());
	gfx.GetContext()->PSSetSamplers(1, 1, m_pSamplerStateClamp.GetAddressOf());

	// ������ �׷���.
	gfx.GetContext()->DrawIndexed(indexCount, 0, 0);
}
