#include "OrthoWindow.h"
#include <stdexcept>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <ErrorHandle/StandardException.h>

OrthoWindow::OrthoWindow()
{
}

OrthoWindow::~OrthoWindow()
{
}

void OrthoWindow::Initialize(D3DGraphics& gfx, int windowWidth, int windowHeight)
{
	InitializeBuffers(gfx, windowWidth, windowHeight);
}

void OrthoWindow::Bind(D3DGraphics& gfx)
{
	BindBuffers(gfx);
}

int OrthoWindow::GetIndexCount()
{
	return m_indexCount;
}

void OrthoWindow::InitializeBuffers(D3DGraphics& gfx, int windowWidth, int windowHeight)
{
	float left, right, top, bottom;
	VertexType* pVertices;
	unsigned long* pIndices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// â�� ����, ������, ����, �Ʒ��� ��ũ�� ��ǥ�� �������.
	left = (float)((windowWidth / 2) * -1);
	right = left + (float)windowWidth;
	top = (float)(windowHeight / 2);
	bottom = top - (float)windowHeight;

	// ������ ���� �ε��� ���� ������.
	m_vertexCount = 6;
	m_indexCount = m_vertexCount;

	// ���� �迭�� �ε��� �迭 ���� �Ҵ��Ͽ� ����.
	pVertices = new VertexType[m_vertexCount];
	ALLOCATE_EXCEPT(pVertices, "Can't allocate vertex array!")
	pIndices = new unsigned long[m_indexCount];
	ALLOCATE_EXCEPT(pIndices, "Can't allocate index array!")

	// ���� �迭�� �� ���� �ﰢ���� �����ϴ� �������� �־���.
	// ù ��° �ﰢ��.
	pVertices[0].position = DirectX::XMFLOAT3(left, top, 0.0f);  // Top left.
	pVertices[0].texture = DirectX::XMFLOAT2(0.0f, 0.0f);

	pVertices[1].position = DirectX::XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	pVertices[1].texture = DirectX::XMFLOAT2(1.0f, 1.0f);

	pVertices[2].position = DirectX::XMFLOAT3(left, bottom, 0.0f);  // Bottom left.
	pVertices[2].texture = DirectX::XMFLOAT2(0.0f, 1.0f);

	// �� ��° �ﰢ��.
	pVertices[3].position = DirectX::XMFLOAT3(left, top, 0.0f);  // Top left.
	pVertices[3].texture = DirectX::XMFLOAT2(0.0f, 0.0f);

	pVertices[4].position = DirectX::XMFLOAT3(right, top, 0.0f);  // Top right.
	pVertices[4].texture = DirectX::XMFLOAT2(1.0f, 0.0f);

	pVertices[5].position = DirectX::XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	pVertices[5].texture = DirectX::XMFLOAT2(1.0f, 1.0f);

	// �ε��� ���۸� ä����.
	for (int i = 0; i < m_indexCount; i++)
	{
		pIndices[i] = i;
	}

	// ���� ���� �����ڸ� ����.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// ���� ���ۿ� ������ �� ���� �����͸� ����.
	vertexData.pSysMem = pVertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// ���� ���� ����.
	INFOMAN(gfx);
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_pVertexBuffer));

	// �ε��� ���� �����ڸ� ����.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// �ε��� ���ۿ� ������ �� �ε��� �����͸� ����.
	indexData.pSysMem = pIndices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// �ε��� ���� ����.
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer));

	// �� �̻� �ʿ���� ����, �ε��� �迭�� ��������.
	delete[] pVertices;
	pVertices = nullptr;

	delete[] pIndices;
	pIndices = nullptr;
}

void OrthoWindow::BindBuffers(D3DGraphics& gfx)
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
