#include "DebugWindow.h"
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <ErrorHandle/StandardException.h>

DebugWindow::DebugWindow()
{
}

void DebugWindow::Initialize(D3DGraphics& gfx, int screenWidth, int screenHeight, int bitmapWidth, int bitmapHeight)
{
	// 화면 크기 저장.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	// 비트맵 사이즈 저장.
	m_bitmapWidth = bitmapWidth;
	m_bitmapHeight = bitmapHeight;
	// 이전 렌더링 위치를 -1로 초기화.
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

	// 정점 배열, 인덱스 배열의 크기를 지정함.
	m_vertexCount = 6;
	m_indexCount = m_vertexCount;

	// 정점 배열 할당.
	vertices = new VertexType[m_vertexCount];
	ALLOCATE_EXCEPT(vertices, "Can't allocate vertex array");
	// 인덱스 배열 할당.
	indices = new unsigned long[m_indexCount];
	ALLOCATE_EXCEPT(indices, "Can't allocate index array");

	// 정점 배열 내용들을 0으로 초기화 해줌.
	memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));
	// 인덱스 배열에 인덱스를 채워줌.
	for (int i = 0; i < m_indexCount; i++)
	{
		indices[i] = i;
	}

	// 정점 버퍼 서술자 생성 및 설정.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// 정점 버퍼에 들어갈 실제 데이터를 지정해줌.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 정점 버퍼 생성.
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_pVertexBuffer));

	// 인덱스 버퍼 서술자를 생성 및 설정.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// 인덱스 버퍼에 들어갈 실제 데이터를 지정해줌.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// 인덱스 버퍼 생성.
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer));

	// 더 이상 필요 없으므로 할당받은 배열 해제해줌.
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

	// 만약 렌더링할 위치가 전혀 변하지 않았다면 아무 작업도 하지 않고 바로 리턴해줌.
	if ((positionX == m_previousPosX) && (positionY == m_previousPosY))
	{
		return;
	}

	// 다음 업데이트에 사용하기 위해 이전 위치를 현재 위치로 업데이트 해줌.
	m_previousPosX = positionX;
	m_previousPosY = positionY;

	// 비트맵 좌측의 스크린 좌표를 계산.
	left = (float)((m_screenWidth / 2) * -1) + (float)positionX;
	// 비트맵 우측의 스크린 좌표를 계산.
	right = left + (float)m_bitmapWidth;
	// 비트맵 상단의 스크린 좌표를 계산.
	top = (float)(m_screenHeight / 2) - (float)positionY;
	// 비트맵 하단의 스크린 좌표를 계산.
	bottom = top - (float)m_bitmapHeight;

	// 업데이트 된 값을 담을 정점 배열을 하나 만들어줌.
	vertices = new VertexType[m_vertexCount];
	ALLOCATE_EXCEPT(vertices, "Can't allocate vertex array");

	// 새 정점 배열을 업데이트 된 값으로 채워줌.
	// 첫 번째 삼각형.
	vertices[0].position = DirectX::XMFLOAT3(left, top, 0.0f);  // Top left.
	vertices[0].texture = DirectX::XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = DirectX::XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[1].texture = DirectX::XMFLOAT2(1.0f, 1.0f);

	vertices[2].position = DirectX::XMFLOAT3(left, bottom, 0.0f);  // Bottom left.
	vertices[2].texture = DirectX::XMFLOAT2(0.0f, 1.0f);

	// 두 번째 삼각형.
	vertices[3].position = DirectX::XMFLOAT3(left, top, 0.0f);  // Top left.
	vertices[3].texture = DirectX::XMFLOAT2(0.0f, 0.0f);

	vertices[4].position = DirectX::XMFLOAT3(right, top, 0.0f);  // Top right.
	vertices[4].texture = DirectX::XMFLOAT2(1.0f, 0.0f);

	vertices[5].position = DirectX::XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[5].texture = DirectX::XMFLOAT2(1.0f, 1.0f);

	// Lock the vertex buffer so it can be written to.
	// 정점 버퍼의 값을 변경하기 위해 lock.
	GFX_THROW_INFO(gfx.GetContext()->Map(m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	// CPU 메모리에 매핑된 정점 버퍼에 대한 포인터를 얻어옴.
	verticesPtr = (VertexType*)mappedResource.pData;

	// 정점 버퍼에 업데이트 된 값 써주기.
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

	// 데이터를 다 썼으므로 unlock.
	gfx.GetContext()->Unmap(m_pVertexBuffer.Get(), 0);

	// 필요 없는 배열 해제.
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
