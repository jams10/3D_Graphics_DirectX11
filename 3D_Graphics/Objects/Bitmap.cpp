#include "Bitmap.h"
#include<ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <ErrorHandle/StandardException.h>

Bitmap::Bitmap()
{
	m_pTexture = nullptr;
}

Bitmap::~Bitmap()
{
}

void Bitmap::Initialize(D3DGraphics& gfx, int screenWidth, int screenHeight, 
	                    std::string textureFilePath, int bitmapWidth, int bitmapHeight)
{
	// ȭ�� ũ�� ����.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	
	// Bitmap Ŭ������ ���ǵǴ� 2D �̹��� ��ü�� �׷��� ũ�⸦ ������.
	m_bitmapWidth = bitmapWidth;
	m_bitmapHeight = bitmapHeight;

	// ���� ������ ��ġ�� -1�� �ʱ�ȭ��.
	m_previousPosX = -1;
	m_previousPosY = -1;

	InitializeBuffers(gfx);
	LoadTexture(gfx, textureFilePath);
}

void Bitmap::Release()
{
	ReleaseTexture();
}

void Bitmap::Bind(D3DGraphics& gfx, int positionX, int positionY)
{
	UpdateBuffers(gfx, positionX, positionY);
	BindBuffers(gfx);
}

ID3D11ShaderResourceView* Bitmap::GetTexture()
{
	return m_pTexture->GetTextureView();
}

void Bitmap::InitializeBuffers(D3DGraphics& gfx)
{
	VertexType* vertices;
	unsigned long* indices;
	INFOMAN(gfx)

	// ������ ������ �ε����� ������ ����.
	m_vertexCount = 6;
	m_indexCount = 6;

	// ���� �迭�� �ε��� �迭 ����.
	vertices = new VertexType[m_vertexCount];
	ALLOCATE_EXCEPT(vertices, "Can't Allocate the vertices array.")
	indices = new unsigned long[m_indexCount];
	ALLOCATE_EXCEPT(indices, "Can't Allocate the indices array.")

	// ���� �迭�� �� ������ �ʱ� ���� 0���� �ʱ�ȭ ����.
	memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

	// �ε��� �迭 ä���ֱ�.
	for (int i = 0; i < m_indexCount; ++i) indices[i] = i;

	// ���� ���� ���� ������ ����.
	D3D11_BUFFER_DESC  vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // ��� �뵵�� '����' ���۷� ����.
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// ���� ���� ���ۿ� �� ���� �����͸� ��������.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// �����ڸ� ���� ���۸� ����.
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_pVertexBuffer));

	// �ε��� ���� ������ ����.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// �ε��� ���ۿ� �� ���� �����͸� ��������.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// �����ڸ� ���� ���۸� ����.
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer));

	// �ʿ� ������ ����, �ε��� �迭 ����.
	delete[] vertices;
	vertices = nullptr;
	delete[] indices;
	indices = nullptr;
}

void Bitmap::UpdateBuffers(D3DGraphics& gfx, int positionX, int positionY)
{
	float left, right, top, bottom;

	// ���� bitmap�� ��ġ�� �޶����� �ʾҴٸ�, ���� ���۸� ������Ʈ �� �� �ʿ䰡 ����.
	if ((positionX == m_previousPosX) && (positionY == m_previousPosY)) return;

	// ���� ������ �� üũ�� ���� ���� ��ġ�� �̸� ���� ��ġ�� ������Ʈ �� ����.
	m_previousPosX = positionX;
	m_previousPosY = positionY;

	// x,y ��ġ�� ���� bitmap�� �����¿� ��ǥ�� �����.
	//left = (float)((m_screenWidth / 2) * -1) + (float)positionX;
	left = (float)positionX - (float)(m_bitmapWidth / 2);
	//right = left + (float)m_bitmapWidth;
	right = (float)positionX + (float)(m_bitmapWidth / 2);
	//top = (float)(m_screenHeight / 2) - (float)positionY;
	top = (float)positionY + (float)(m_bitmapHeight / 2);
	//bottom = top - (float)m_bitmapHeight;
	bottom = (float)positionY - (float)(m_bitmapHeight / 2);

	// ���� �迭 ����.
	VertexType* vertices = new VertexType[m_vertexCount];
	ALLOCATE_EXCEPT(vertices, "Can't allocate the vertex array")

	// ���� �迭�� ���� �� ���� ä����.
	vertices[0].pos = DirectX::XMFLOAT3(left, top, 0.0f);
	vertices[0].tex = DirectX::XMFLOAT2(0.0f, 0.0f);
	vertices[1].pos = DirectX::XMFLOAT3(right, bottom, 0.0f);
	vertices[1].tex = DirectX::XMFLOAT2(1.0f, 1.0f);
	vertices[2].pos = DirectX::XMFLOAT3(left, bottom, 0.0f);
	vertices[2].tex = DirectX::XMFLOAT2(0.0f, 1.0f);
	vertices[3].pos = DirectX::XMFLOAT3(left, top, 0.0f);
	vertices[3].tex = DirectX::XMFLOAT2(0.0f, 0.0f);
	vertices[4].pos = DirectX::XMFLOAT3(right, top, 0.0f);
	vertices[4].tex = DirectX::XMFLOAT2(1.0f, 0.0f);
	vertices[5].pos = DirectX::XMFLOAT3(right, bottom, 0.0f);
	vertices[5].tex = DirectX::XMFLOAT2(1.0f, 1.0f);

	// ���� ���ۿ� ���� ���� ���� �ϴ� lock�� �ɾ���.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	gfx.GetContext()->Map(m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// lock�� �ɾ��� ���ҽ��� ���� ���ۿ� ��� ���� �����͸� ������.
	VertexType* verticesPtr = (VertexType*)mappedResource.pData;

	// �츮�� ������ ������� �� ���� �����͸� ���� ���ۿ� ����.
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

	// �ٽ� ���� ���۸� unlock.
	gfx.GetContext()->Unmap(m_pVertexBuffer.Get(), 0);

	// �ʿ� ������ ���� �Ҵ� ���� ���� �迭�� ����.
	delete[] vertices;
	vertices = nullptr;
}

void Bitmap::BindBuffers(D3DGraphics& gfx)
{
	unsigned int stride; // ���� ���� ������ ũ��.
	unsigned int offset; // ���� ���� ���� ������ �Ÿ�.

	stride = sizeof(VertexType);
	offset = 0;

	// ���������ο� ���� ���� ���ε�.
	gfx.GetContext()->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);

	// ���������ο� �ε��� ���� ���ε�.
	gfx.GetContext()->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// �������� ���� �������� �⺻ ����(primitive)�� �ﰢ������ ����.
	gfx.GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

void Bitmap::LoadTexture(D3DGraphics& gfx, std::string filePath)
{
	m_pTexture = new Texture(gfx, filePath);
	ALLOCATE_EXCEPT(m_pTexture, "Can't allocate a texture instance!")
}

void Bitmap::ReleaseTexture()
{
	if (m_pTexture)
	{
		delete m_pTexture;
		m_pTexture = nullptr;
	}
}
