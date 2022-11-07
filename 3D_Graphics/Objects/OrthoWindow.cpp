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

	// 창의 왼쪽, 오른쪽, 위쪽, 아래쪽 스크린 좌표를 계산해줌.
	left = (float)((windowWidth / 2) * -1);
	right = left + (float)windowWidth;
	top = (float)(windowHeight / 2);
	bottom = top - (float)windowHeight;

	// 정점의 수와 인덱스 수를 설정함.
	m_vertexCount = 6;
	m_indexCount = m_vertexCount;

	// 정점 배열와 인덱스 배열 동적 할당하여 생성.
	pVertices = new VertexType[m_vertexCount];
	ALLOCATE_EXCEPT(pVertices, "Can't allocate vertex array!")
	pIndices = new unsigned long[m_indexCount];
	ALLOCATE_EXCEPT(pIndices, "Can't allocate index array!")

	// 정점 배열에 두 개의 삼각형을 구성하는 정점들을 넣어줌.
	// 첫 번째 삼각형.
	pVertices[0].position = DirectX::XMFLOAT3(left, top, 0.0f);  // Top left.
	pVertices[0].texture = DirectX::XMFLOAT2(0.0f, 0.0f);

	pVertices[1].position = DirectX::XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	pVertices[1].texture = DirectX::XMFLOAT2(1.0f, 1.0f);

	pVertices[2].position = DirectX::XMFLOAT3(left, bottom, 0.0f);  // Bottom left.
	pVertices[2].texture = DirectX::XMFLOAT2(0.0f, 1.0f);

	// 두 번째 삼각형.
	pVertices[3].position = DirectX::XMFLOAT3(left, top, 0.0f);  // Top left.
	pVertices[3].texture = DirectX::XMFLOAT2(0.0f, 0.0f);

	pVertices[4].position = DirectX::XMFLOAT3(right, top, 0.0f);  // Top right.
	pVertices[4].texture = DirectX::XMFLOAT2(1.0f, 0.0f);

	pVertices[5].position = DirectX::XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	pVertices[5].texture = DirectX::XMFLOAT2(1.0f, 1.0f);

	// 인덱스 버퍼를 채워줌.
	for (int i = 0; i < m_indexCount; i++)
	{
		pIndices[i] = i;
	}

	// 정점 버퍼 서술자를 설정.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// 정점 버퍼에 실제로 들어갈 정점 데이터를 설정.
	vertexData.pSysMem = pVertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 정점 버퍼 생성.
	INFOMAN(gfx);
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_pVertexBuffer));

	// 인덱스 버퍼 서술자를 설정.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// 인덱스 버퍼에 실제로 들어갈 인덱스 데이터를 설정.
	indexData.pSysMem = pIndices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// 인덱스 버퍼 생성.
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer));

	// 더 이상 필요없는 정점, 인덱스 배열을 해제해줌.
	delete[] pVertices;
	pVertices = nullptr;

	delete[] pIndices;
	pIndices = nullptr;
}

void OrthoWindow::BindBuffers(D3DGraphics& gfx)
{
	unsigned int stride; // 정점 버퍼 원소의 크기.
	unsigned int offset; // 정점 버퍼 원소 사이의 거리.

	stride = sizeof(VertexType);
	offset = 0;

	// 파이프라인에 정점 버퍼 바인딩.
	gfx.GetContext()->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);

	// 파이프라인에 인덱스 버퍼 바인딩.
	gfx.GetContext()->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// 정점들을 통해 구성해줄 기본 도형(primitive)을 삼각형으로 정의.
	gfx.GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}
