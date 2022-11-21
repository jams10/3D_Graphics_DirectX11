#include "TS_TestModel.h"
#include <stdexcept>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>

TS_TestModel::TS_TestModel()
    :
    m_vertexCount(0),
    m_indexCount(0)
{
}

TS_TestModel::~TS_TestModel()
{
}

void TS_TestModel::Initialize(D3DGraphics& gfx)
{
    InitializeBuffers(gfx);
}

void TS_TestModel::Bind(D3DGraphics& gfx)
{
    BindBuffers(gfx);
}

void TS_TestModel::InitializeBuffers(D3DGraphics& gfx)
{
    VertexType* vertices = nullptr;
    unsigned long* indices = nullptr;

    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;       

    m_vertexCount = 3;
    m_indexCount = 3;

    vertices = new VertexType[m_vertexCount];
    if (!vertices) throw std::runtime_error("Cannot create the vertex array");

    indices = new unsigned long[m_indexCount];
    if (!indices) throw std::runtime_error("Cannot create the index array");


    vertices[0].position = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);
    vertices[0].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

    vertices[1].position = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertices[1].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

    vertices[2].position = DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f);
    vertices[2].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);


    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;


    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;  
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount; 
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; 
    vertexBufferDesc.CPUAccessFlags = 0; 
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = sizeof(VertexType); 

    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    INFOMAN(gfx)

    GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_pVertexBuffer));

    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = sizeof(int);

    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer));

    delete[] vertices;
    vertices = nullptr;

    delete[] indices;
    indices = nullptr;
}

void TS_TestModel::BindBuffers(D3DGraphics& gfx)
{
    unsigned int stride; 
    unsigned int offset; 

    stride = sizeof(VertexType);
    offset = 0;

    gfx.GetContext()->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);

    gfx.GetContext()->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // 기존에는 삼각형 리스트를 그려줬다면, 이제는 테셀레이션을 사용하기 위해 세 개의 제어점을 갖는 패치 리스트를 기본 도형으로 설정함.
    gfx.GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

    return;
}