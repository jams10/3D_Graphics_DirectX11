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
	// 화면 크기 저장.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	
	// Bitmap 클래스로 정의되는 2D 이미지 객체가 그려질 크기를 저장함.
	m_bitmapWidth = bitmapWidth;
	m_bitmapHeight = bitmapHeight;

	// 이전 렌더링 위치를 -1로 초기화함.
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

	// 정점의 개수와 인덱스의 개수를 설정.
	m_vertexCount = 6;
	m_indexCount = 6;

	// 정점 배열과 인덱스 배열 생성.
	vertices = new VertexType[m_vertexCount];
	ALLOCATE_EXCEPT(vertices, "Can't Allocate the vertices array.")
	indices = new unsigned long[m_indexCount];
	ALLOCATE_EXCEPT(indices, "Can't Allocate the indices array.")

	// 정점 배열의 각 정점의 초기 값을 0으로 초기화 해줌.
	memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

	// 인덱스 배열 채워주기.
	for (int i = 0; i < m_indexCount; ++i) indices[i] = i;

	// 동적 정점 버퍼 서술자 생성.
	D3D11_BUFFER_DESC  vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // 사용 용도를 '동적' 버퍼로 지정.
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// 동적 정점 버퍼에 들어갈 실제 데이터를 지정해줌.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 서술자를 통해 버퍼를 생성.
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_pVertexBuffer));

	// 인덱스 버퍼 서술자 생성.
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

	// 서술자를 통해 버퍼를 생성.
	GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer));

	// 필요 없어진 정점, 인덱스 배열 해제.
	delete[] vertices;
	vertices = nullptr;
	delete[] indices;
	indices = nullptr;
}

void Bitmap::UpdateBuffers(D3DGraphics& gfx, int positionX, int positionY)
{
	float left, right, top, bottom;

	// 만약 bitmap의 위치가 달라지지 않았다면, 정점 버퍼를 업데이트 해 줄 필요가 없다.
	if ((positionX == m_previousPosX) && (positionY == m_previousPosY)) return;

	// 다음 프레임 때 체크를 위해 이전 위치를 미리 현재 위치로 업데이트 해 놓음.
	m_previousPosX = positionX;
	m_previousPosY = positionY;

	// x,y 위치에 따라 bitmap의 상하좌우 좌표를 계산함.
	//left = (float)((m_screenWidth / 2) * -1) + (float)positionX;
	left = (float)positionX - (float)(m_bitmapWidth / 2);
	//right = left + (float)m_bitmapWidth;
	right = (float)positionX + (float)(m_bitmapWidth / 2);
	//top = (float)(m_screenHeight / 2) - (float)positionY;
	top = (float)positionY + (float)(m_bitmapHeight / 2);
	//bottom = top - (float)m_bitmapHeight;
	bottom = (float)positionY - (float)(m_bitmapHeight / 2);

	// 정점 배열 생성.
	VertexType* vertices = new VertexType[m_vertexCount];
	ALLOCATE_EXCEPT(vertices, "Can't allocate the vertex array")

	// 정점 배열에 정점 값 들을 채워줌.
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

	// 정점 버퍼에 값을 쓰기 위해 일단 lock을 걸어줌.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	gfx.GetContext()->Map(m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// lock을 걸어준 리소스로 부터 버퍼에 담긴 실제 데이터를 가져옴.
	VertexType* verticesPtr = (VertexType*)mappedResource.pData;

	// 우리가 위에서 만들어준 각 정점 데이터를 정점 버퍼에 써줌.
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

	// 다시 정점 버퍼를 unlock.
	gfx.GetContext()->Unmap(m_pVertexBuffer.Get(), 0);

	// 필요 없어진 동적 할당 받은 정점 배열을 해제.
	delete[] vertices;
	vertices = nullptr;
}

void Bitmap::BindBuffers(D3DGraphics& gfx)
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
