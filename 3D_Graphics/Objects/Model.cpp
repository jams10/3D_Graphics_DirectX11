#include "Model.h"
#include <stdexcept>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>

Model::Model()
    :
    m_vertexCount(0),
    m_indexCount(0)
{
}

Model::~Model()
{
}

void Model::Initialize(D3DGraphics& gfx)
{
    InitializeBuffers(gfx);
}

void Model::Bind(D3DGraphics& gfx)
{
    BindBuffers(gfx);
}

void Model::InitializeBuffers(D3DGraphics& gfx)
{
    VertexType* vertices = nullptr;
    unsigned long* indices = nullptr;

    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc; // 정점, 인덱스 데이터가 들어갈 버퍼.
    D3D11_SUBRESOURCE_DATA vertexData, indexData;        // 실제 정점, 인덱스 버퍼에 들어갈 데이터

    // 정점과 인덱스 개수를 설정.
    m_vertexCount = 3;
    m_indexCount = 3;

    // 정점 배열 생성
    vertices = new VertexType[m_vertexCount];
    if (!vertices) throw std::runtime_error("Cannot create the vertex array");
    // 인덱스 배열 생성
    indices = new unsigned long[m_indexCount];
    if (!indices) throw std::runtime_error("Cannot create the index array");

    // 정점 배열에 정점 데이터를 채워줌.
    vertices[0].position = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);
    vertices[0].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

    vertices[1].position = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertices[1].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

    vertices[2].position = DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f);
    vertices[2].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

    // 인덱스 배열 채워주기.
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;

    // 정적 정점 버퍼의 서술자를 설정.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;  // 버퍼가 어떻게 읽혀지고 쓰여지는지(written) 정의. 
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount; // 버퍼의 크기(바이트)
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // 버퍼가 파이프라인에 어떻게 묶일지(어떤 버퍼 인지)를 정의.
    vertexBufferDesc.CPUAccessFlags = 0; // CPU 접근 플래그. 0이면 CPU 접근이 필요 없다는 것.
    vertexBufferDesc.MiscFlags = 0; // 기타 플래그. 0이면 사용하지 않겠다는 것.
    vertexBufferDesc.StructureByteStride = sizeof(VertexType); // 버퍼에 들어있는 각 원소의 크기.

    // 정점 버퍼에 대한 subresource 구조체를 설정.
    vertexData.pSysMem = vertices; // 초기화 데이터에 대한 포인터. 여기에 우리의 정점 데이터를 넣어줌.
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    INFOMAN(gfx)

    // 정점 버퍼 생성.
    GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_pVertexBuffer));

    // 인덱스 버퍼의 서술자를 세팅.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = sizeof(int);

    // 인덱스 버퍼에 대한 subresource 구조체를 설정.
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // 인덱스 버퍼 생성.
    GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer));

    // 자원을 생성 했으므로, 동적 할당한 배열은 필요가 없으므로 해제 해줌.
    delete[] vertices;
    vertices = nullptr;

    delete[] indices;
    indices = nullptr;
}

void Model::BindBuffers(D3DGraphics& gfx)
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
