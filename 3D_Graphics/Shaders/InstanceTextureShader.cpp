#include "InstanceTextureShader.h"
#include <d3dcompiler.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>

InstanceTextureShader::InstanceTextureShader()
{
}

InstanceTextureShader::~InstanceTextureShader()
{
}

void InstanceTextureShader::Initialize(D3DGraphics& gfx)
{
	InitializeShaders(gfx, L"Shaders/InstanceTextureVS.cso", L"Shaders/InstanceTexturePS.cso");
}

void InstanceTextureShader::Bind(D3DGraphics& gfx, int vertexCount, int indexCount, int instanceCount, XMMATRIX world, XMMATRIX view, XMMATRIX projection, ID3D11ShaderResourceView* texture)
{
	SetShaderParameters(gfx, world, view, projection, texture);

	// �ε��� ��� �ν��Ͻ� ������ �Է� ����.
	BindShaders(gfx, vertexCount, indexCount, instanceCount);
}

void InstanceTextureShader::InitializeShaders(D3DGraphics& gfx, const std::wstring& vsFileName, const std::wstring& psFileName)
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

	// �Է� ���̾ƿ� ������ ����.
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

	polygonLayout[2].SemanticName = "TEXCOORD"; // �ø�ƽ �̸��� ���� �ø�ƽ�� TEXCOORD ����.
	polygonLayout[2].SemanticIndex = 1; // ������ �ؽ��� ��ǥ�� ����ϴ� TEXCOORD �ø�ƽ�� 0�� ������ ����ϹǷ� ���⿡���� 1�� �������� ������.
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 1;
	polygonLayout[2].AlignedByteOffset = 0; // position�� �ν��Ͻ� ������ ù �������̸�, ���� ���ۿ� �ν��Ͻ� �����͸� �������� ���� ���̱� ������ ���� �ɼ��� 0���� ����.
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA; // �� �����Ͱ� �ν��Ͻ� ���������� ǥ��.
	polygonLayout[2].InstanceDataStepRate = 1; // �ν��Ͻ� ������ �� �ϳ��� �ν��Ͻ��� �׸����� ��.

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
}

void InstanceTextureShader::SetShaderParameters(D3DGraphics& gfx, XMMATRIX world, XMMATRIX view, XMMATRIX projection, ID3D11ShaderResourceView* texture)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
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

	// �ȼ� ���̴����� ����� �ؽ��� �ڿ��� ����.
	gfx.GetContext()->PSSetShaderResources(0, 1, &texture);
}

void InstanceTextureShader::BindShaders(D3DGraphics& gfx, int vertexCount, int indexCount, int instanceCount)
{
	// ���� �Է� ���̾ƿ��� ���ε� ����.
	gfx.GetContext()->IASetInputLayout(m_pLayout.Get());

	// ����, �ȼ� ���̴��� ���������ο� ���ε� ����.
	gfx.GetContext()->VSSetShader(m_pVertexShader.Get(), NULL, 0);
	gfx.GetContext()->PSSetShader(m_pPixelShader.Get(), NULL, 0);

	// �ȼ� ���̴��� Sampler State�� ���� ����.
	gfx.GetContext()->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());

	// ������ �׷���.
	gfx.GetContext()->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
}
