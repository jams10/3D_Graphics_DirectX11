#include "TSColorShader.h"
#include <d3dcompiler.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>

TSColorShader::TSColorShader()
{
}

TSColorShader::~TSColorShader()
{
}

void TSColorShader::Initialize(D3DGraphics& gfx)
{
	InitializeShaders(gfx, L"Shaders/ColorTS_VS.cso", L"Shaders/ColorTS_HS.cso", 
		L"Shaders/ColorTS_DS.cso", L"Shaders/ColorTS_PS.cso");
}

void TSColorShader::Bind(D3DGraphics& gfx, int indexCount, XMMATRIX world, XMMATRIX view, XMMATRIX projection, float tessellationAmount)
{
	SetShaderParameters(gfx, world, view, projection, tessellationAmount);
	BindShaders(gfx, indexCount);
}

void TSColorShader::InitializeShaders(D3DGraphics& gfx, const std::wstring& vsFileName, const std::wstring& hsFileName, const std::wstring& dsFileName, const std::wstring& psFileName)
{
	ID3DBlob* vertexShaderBuffer;
	ID3DBlob* hullShaderBuffer;
	ID3DBlob* domainShaderBuffer;
	ID3DBlob* pixelShaderBuffer;
	INFOMAN(gfx)

	// ���̴� ���� �̸����� ���� ���� ���̴� ����.
	GFX_THROW_INFO(D3DReadFileToBlob(vsFileName.c_str(), &vertexShaderBuffer));
	GFX_THROW_INFO(gfx.GetDevice()->CreateVertexShader(
		vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(),
		nullptr,
		&m_pVertexShader
	));

	// ���̴� ���� �̸����� ���� �� ���̴� ����.
	GFX_THROW_INFO(D3DReadFileToBlob(hsFileName.c_str(), &hullShaderBuffer));
	GFX_THROW_INFO(gfx.GetDevice()->CreateHullShader(
		hullShaderBuffer->GetBufferPointer(),
		hullShaderBuffer->GetBufferSize(),
		nullptr,
		&m_pHullShader
	));

	// ���̴� ���� �̸����� ���� ������ ���̴� ����.
	GFX_THROW_INFO(D3DReadFileToBlob(dsFileName.c_str(), &domainShaderBuffer));
	GFX_THROW_INFO(gfx.GetDevice()->CreateDomainShader(
		domainShaderBuffer->GetBufferPointer(),
		domainShaderBuffer->GetBufferSize(),
		nullptr,
		&m_pDomainShader
	));

	// ���̴� ���� �̸����� ���� �ȼ� ���̴� ����.
	GFX_THROW_INFO(D3DReadFileToBlob(psFileName.c_str(), &pixelShaderBuffer));
	GFX_THROW_INFO(gfx.GetDevice()->CreatePixelShader(
		pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(),
		nullptr,
		&m_pPixelShader));

	// �Է� ���̾ƿ� ������ ����.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// �Է� ���̾ƿ��� �ִ� ���ҵ��� ������ ����.
	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// �Է� ���̾ƿ� ����.
	GFX_THROW_INFO(gfx.GetDevice()->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_pLayout));

	// ���۴� �� �̻� �ʿ䰡 �����Ƿ� ���� ����.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;

	hullShaderBuffer->Release();
	hullShaderBuffer = nullptr;

	domainShaderBuffer->Release();
	domainShaderBuffer = nullptr;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

	// ������ ���̴����� ����� ��ĵ��� ���� ��� ���ۿ� ���� �����ڸ� ����.
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	// ��� ���� ����.
	gfx.GetDevice()->CreateBuffer(&matrixBufferDesc, NULL, &m_pMatrixBuffer);

	D3D11_BUFFER_DESC tessellationBufferDesc;
	tessellationBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tessellationBufferDesc.ByteWidth = sizeof(TessellationBufferType);
	tessellationBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tessellationBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tessellationBufferDesc.MiscFlags = 0;
	tessellationBufferDesc.StructureByteStride = 0;
	gfx.GetDevice()->CreateBuffer(&tessellationBufferDesc, NULL, &m_pTessellationBuffer);
}

void TSColorShader::SetShaderParameters(D3DGraphics& gfx, XMMATRIX world, XMMATRIX view, XMMATRIX projection, float tessellationAmount)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	TessellationBufferType* dataPtr2;
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

	dataPtr->world = world;
	dataPtr->view = view;
	dataPtr->projection = projection;

	gfx.GetContext()->Unmap(m_pMatrixBuffer.Get(), 0);

	bufferNumber = 0;
	// ������ ���̴��� �ִ� ��� ���۸� ���ŵ� ������ ��������.
	gfx.GetContext()->DSSetConstantBuffers(bufferNumber, 1, m_pMatrixBuffer.GetAddressOf());

	GFX_THROW_INFO(gfx.GetContext()->Map(m_pTessellationBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	dataPtr2 = (TessellationBufferType*)mappedResource.pData;

	dataPtr2->tessellationAmount = tessellationAmount;
	dataPtr2->padding = XMFLOAT3(0.0f, 0.0f, 0.0f);

	gfx.GetContext()->Unmap(m_pTessellationBuffer.Get(), 0);

	bufferNumber = 0;
	gfx.GetContext()->HSSetConstantBuffers(bufferNumber, 1, m_pTessellationBuffer.GetAddressOf());
}

void TSColorShader::BindShaders(D3DGraphics& gfx, int indexCount)
{
	// ���� �Է� ���̾ƿ��� ���ε� ����.
	gfx.GetContext()->IASetInputLayout(m_pLayout.Get());

	// ����, ��, ������, �ȼ� ���̴��� ���������ο� ���ε� ����.
	gfx.GetContext()->VSSetShader(m_pVertexShader.Get(), NULL, 0);
	gfx.GetContext()->HSSetShader(m_pHullShader.Get(), NULL, 0);
	gfx.GetContext()->DSSetShader(m_pDomainShader.Get(), NULL, 0);
	gfx.GetContext()->PSSetShader(m_pPixelShader.Get(), NULL, 0);
	
	// ������ �׷���.
	gfx.GetContext()->DrawIndexed(indexCount, 0, 0);
}
