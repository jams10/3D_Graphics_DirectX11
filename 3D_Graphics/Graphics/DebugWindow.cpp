#include "DebugWindow.h"
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <ErrorHandle/StandardException.h>

DebugWindow::DebugWindow()
{
}

void DebugWindow::Initialize(D3DGraphics& gfx, int screenWidth, int screenHeight, int bitmapWidth, int bitmapHeight)
{
	// ȭ�� ũ�� ����.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	// ��Ʈ�� ������ ����.
	m_bitmapWidth = bitmapWidth;
	m_bitmapHeight = bitmapHeight;
	// ���� ������ ��ġ�� -1�� �ʱ�ȭ.
	m_previousPosX = -1;
	m_previousPosY = -1;

	InitializeBuffers(gfx);
}

void DebugWindow::Render(D3DGraphics& gfx, int positionX, int positionY)
{
	UpdateBuffers(gfx, positionX, positionY);
	RenderBuffers(gfx);
}

int DebugWindow::GetIndexCount()
{
	return m_indexCount;
}

void DebugWindow::InitializeBuffers(D3DGraphics& gfx)
{
	INFOMAN(gfx);
	VertexType* vertices;
	unsigned long* indices;

	// ���� �迭, �ε��� �迭�� ũ�⸦ ������.
	m_vertexCount = 6;
	m_indexCount = m_vertexCount;

	// ���� �迭 �Ҵ�.
	vertices = new VertexType[m_vertexCount];
	ALLOCATE_EXCEPT(vertices, "Can't allocate vertex array");
	// �ε��� �迭 �Ҵ�.
	indices = new unsigned long[m_indexCount];
	ALLOCATE_EXCEPT(indices, "Can't allocate index array");

	// ���� �迭 ������� 0���� �ʱ�ȭ ����.
	memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));
	// �ε��� �迭�� �ε����� ä����.
	for (int i = 0; i < m_indexCount; i++)
	{
		indices[i] = i;
	}

	// ���� ���� ������ ���� �� ����.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// ���� ���ۿ� �� ���� �����͸� ��������.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// ���� ���� ����.
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_pVertexBuffer));

	// �ε��� ���� �����ڸ� ���� �� ����.
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

	// �ε��� ���� ����.
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer));

	// �� �̻� �ʿ� �����Ƿ� �Ҵ���� �迭 ��������.
	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;
}

void DebugWindow::UpdateBuffers(D3DGraphics& gfx, int positionX, int positionY)
{
	INFOMAN(gfx)
	float left, right, top, bottom;
	VertexType* vertices;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;
	HRESULT result;

	// ���� �������� ��ġ�� ���� ������ �ʾҴٸ� �ƹ� �۾��� ���� �ʰ� �ٷ� ��������.
	if ((positionX == m_previousPosX) && (positionY == m_previousPosY))
	{
		return;
	}

	// ���� ������Ʈ�� ����ϱ� ���� ���� ��ġ�� ���� ��ġ�� ������Ʈ ����.
	m_previousPosX = positionX;
	m_previousPosY = positionY;

	// ��Ʈ�� ������ ��ũ�� ��ǥ�� ���.
	left = (float)((m_screenWidth / 2) * -1) + (float)positionX;
	// ��Ʈ�� ������ ��ũ�� ��ǥ�� ���.
	right = left + (float)m_bitmapWidth;
	// ��Ʈ�� ����� ��ũ�� ��ǥ�� ���.
	top = (float)(m_screenHeight / 2) - (float)positionY;
	// ��Ʈ�� �ϴ��� ��ũ�� ��ǥ�� ���.
	bottom = top - (float)m_bitmapHeight;

	// ������Ʈ �� ���� ���� ���� �迭�� �ϳ� �������.
	vertices = new VertexType[m_vertexCount];
	ALLOCATE_EXCEPT(vertices, "Can't allocate vertex array");

	// �� ���� �迭�� ������Ʈ �� ������ ä����.
	// ù ��° �ﰢ��.
	vertices[0].position = DirectX::XMFLOAT3(left, top, 0.0f);  // Top left.
	vertices[0].texture = DirectX::XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = DirectX::XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[1].texture = DirectX::XMFLOAT2(1.0f, 1.0f);

	vertices[2].position = DirectX::XMFLOAT3(left, bottom, 0.0f);  // Bottom left.
	vertices[2].texture = DirectX::XMFLOAT2(0.0f, 1.0f);

	// �� ��° �ﰢ��.
	vertices[3].position = DirectX::XMFLOAT3(left, top, 0.0f);  // Top left.
	vertices[3].texture = DirectX::XMFLOAT2(0.0f, 0.0f);

	vertices[4].position = DirectX::XMFLOAT3(right, top, 0.0f);  // Top right.
	vertices[4].texture = DirectX::XMFLOAT2(1.0f, 0.0f);

	vertices[5].position = DirectX::XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[5].texture = DirectX::XMFLOAT2(1.0f, 1.0f);

	// Lock the vertex buffer so it can be written to.
	// ���� ������ ���� �����ϱ� ���� lock.
	GFX_THROW_INFO(gfx.GetContext()->Map(m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	// CPU �޸𸮿� ���ε� ���� ���ۿ� ���� �����͸� ����.
	verticesPtr = (VertexType*)mappedResource.pData;

	// ���� ���ۿ� ������Ʈ �� �� ���ֱ�.
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

	// �����͸� �� �����Ƿ� unlock.
	gfx.GetContext()->Unmap(m_pVertexBuffer.Get(), 0);

	// �ʿ� ���� �迭 ����.
	delete[] vertices;
	vertices = 0;

}

void DebugWindow::RenderBuffers(D3DGraphics& gfx)
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(VertexType);
	offset = 0;

	gfx.GetContext()->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);

	gfx.GetContext()->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	gfx.GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}
