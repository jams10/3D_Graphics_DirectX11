#include "InstanceModel.h"
#include <Graphics/Texture.h>
#include <ErrorHandle/StandardException.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <fstream>
#include <sstream>

InstanceModel::InstanceModel()
{
    m_vertexCount = 0;
    m_instanceCount = 0;
    m_Texture = nullptr;
}

InstanceModel::~InstanceModel()
{
}

void InstanceModel::Initialize(D3DGraphics& gfx, std::string modelFilePath, std::string textureFilePath)
{
    LoadModel(modelFilePath);
    InitializeBuffers(gfx);
    LoadTexture(gfx, textureFilePath);
}

void InstanceModel::Bind(D3DGraphics& gfx)
{
    BindBuffers(gfx);
}

unsigned int InstanceModel::GetVertexCount()
{
    return m_vertexCount;
}

unsigned int InstanceModel::GetIndexCount()
{
    return m_indexCount;
}

unsigned int InstanceModel::GetInstanceCount()
{
    return m_instanceCount;
}

ID3D11ShaderResourceView* InstanceModel::GetTexture()
{
    return m_Texture->GetTextureView();
}

void InstanceModel::InitializeBuffers(D3DGraphics& gfx)
{
    INFOMAN(gfx)

    // 정점 버퍼의 서술자 생성.
    D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;  // 버퍼가 어떻게 읽혀지고 쓰여지는지(written) 정의. 
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount; // 버퍼의 크기(바이트)
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // 버퍼가 파이프라인에 어떻게 묶일지(어떤 버퍼 인지)를 정의.
    vertexBufferDesc.CPUAccessFlags = 0; // CPU 접근 플래그. 0이면 CPU 접근이 필요 없다는 것.
    vertexBufferDesc.MiscFlags = 0; // 기타 플래그. 0이면 사용하지 않겠다는 것.
    vertexBufferDesc.StructureByteStride = 0;

    // 정점 버퍼에 실제 들어갈 정점 데이터를 정의.
    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = m_pVertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // 정점 버퍼 생성.
    GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_pVertexBuffer));

    // 필요 없어진 정점 배열 해제.
    delete[] m_pVertices;
    m_pVertices = nullptr;

    // 인덱스 버퍼의 서술자를 세팅.
    D3D11_BUFFER_DESC indexBufferDesc;
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = sizeof(int);

    // 인덱스 버퍼에 대한 subresource 구조체를 설정.
    D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = m_pIndices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // 인덱스 버퍼 생성.
    GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer));

    delete[] m_pIndices;
    m_pIndices = nullptr;

    // 인스턴스 배열에 들어갈 인스턴스의 개수를 설정.
    m_instanceCount = 4;

    // 인스턴스 배열 생성.
    InstanceType* instances = nullptr;
    instances = new InstanceType[m_instanceCount];
    ALLOCATE_EXCEPT(instances, "Can't allocate model vertex array!")

    // 인스턴스 배열을 채워줌. 각 인스턴스들의 위치 값.
    instances[0].position = DirectX::XMFLOAT3(-1.5f, -1.5f, 5.0f);
    instances[1].position = DirectX::XMFLOAT3(-1.5f, 1.5f, 5.0f);
    instances[2].position = DirectX::XMFLOAT3(1.5f, -1.5f, 5.0f);
    instances[3].position = DirectX::XMFLOAT3(1.5f, 1.5f, 5.0f);

    // 인스턴스 버퍼 서술자 생성.
    D3D11_BUFFER_DESC instanceBufferDesc;
    instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    instanceBufferDesc.ByteWidth = sizeof(InstanceType) * m_instanceCount;
    instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instanceBufferDesc.CPUAccessFlags = 0;
    instanceBufferDesc.MiscFlags = 0;
    instanceBufferDesc.StructureByteStride = 0;

    // 인스턴스 버퍼에 들어갈 인스턴스 데이터 정의.
    D3D11_SUBRESOURCE_DATA instanceData;
    instanceData.pSysMem = instances;
    instanceData.SysMemPitch = 0;
    instanceData.SysMemSlicePitch = 0;

    // 인스턴스 버퍼 생성.
    GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&instanceBufferDesc, &instanceData, &m_pInstanceBuffer));

    // 필요 없어진 인스턴스 배열 해제.
    delete[] instances;
}

void InstanceModel::BindBuffers(D3DGraphics& gfx)
{
    unsigned int strides[2];
    unsigned int offsets[2];
    ID3D11Buffer* bufferPointers[2];

    // 버퍼 stride 설정.
    strides[0] = sizeof(VertexType);
    strides[1] = sizeof(InstanceType);

    // 버퍼 offset들을 설정.
    offsets[0] = 0;
    offsets[1] = 0;

    // 버퍼 포인터 배열을 각각 정점과 인스턴스 버퍼로 설정해줌.
    bufferPointers[0] = m_pVertexBuffer.Get();
    bufferPointers[1] = m_pInstanceBuffer.Get();

    // 파이프라인에 버퍼들을 바인딩 해줌. (정점 셰이더에 바인딩.)
    gfx.GetContext()->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

    // 파이프라인에 인덱스 버퍼 바인딩.
    gfx.GetContext()->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // Primitive Topology를 삼각형으로 설정.
    gfx.GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void InstanceModel::LoadTexture(D3DGraphics& gfx, std::string filePath)
{
    m_Texture = new Texture(gfx, filePath);
}

void InstanceModel::ReleaseTexture()
{
    if (m_Texture)
    {
        delete m_Texture;
        m_Texture = nullptr;
    }
}

void InstanceModel::LoadModel(std::string filePath)
{
    LoadCustomFile(filePath);
}

void InstanceModel::ReleaseModel()
{
    if (m_pModel != nullptr)
    {
        delete[]m_pModel;
        m_pModel = nullptr;
    }
}

void InstanceModel::LoadCustomFile(std::string filePath)
{
    std::ifstream file;
    char input;

    // 모델 파일을 열어줌.
    file.open(filePath);
    if (file.fail()) STD_EXCEPT("Can't open a model file!")

    // 모델 파일에 있는 정점 개수 값을 읽어줌.
    file.get(input);
    while (input != ':')
    {
        file.get(input);
    }
    file >> m_vertexCount;

    // 인덱스 개수를 정점의 개수와 똑같이 설정.
    m_indexCount = m_vertexCount;

    // 읽어들인 정점 개수 만큼 정점 타입 구조체 배열을 할당해줌.
    m_pModel = new ModelType[m_vertexCount];
    ALLOCATE_EXCEPT(m_pModel, "Can't allocate model vertex array!")

    // 모델의 개별 정점 데이터를 읽어옴.
    file.get(input);
    while (input != ':')
    {
        file.get(input);
    }
    file.get(input); // \n
    file.get(input); // \r

    for (int i = 0; i < m_vertexCount; i++)
    {
        file >> m_pModel[i].x >> m_pModel[i].y >> m_pModel[i].z;     // 위치
        file >> m_pModel[i].tu >> m_pModel[i].tv;                    // 텍스쳐 좌표(UV)
    }

    // 정점 배열 생성.
    m_pVertices = new VertexType[m_vertexCount];
    ALLOCATE_EXCEPT(m_pVertices, "Can't allocate vertex array!")

    // 인덱스 배열 생성.
    m_pIndices = new unsigned long[m_indexCount];
    ALLOCATE_EXCEPT(m_pIndices, "Can't allocate index array!")

    // 파일로부터 읽어온 정점 데이터를 정점과 인덱스 배열에 채워줌.
    for (int i = 0; i < m_vertexCount; i++)
    {
        m_pVertices[i].position = DirectX::XMFLOAT3(m_pModel[i].x, m_pModel[i].y, m_pModel[i].z);
        m_pVertices[i].texture = DirectX::XMFLOAT2(m_pModel[i].tu, m_pModel[i].tv);
        m_pIndices[i] = i;
    }

    // 파일 닫아줌.
    file.close();
}

