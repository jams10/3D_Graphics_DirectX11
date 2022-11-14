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

    // ���� ������ ������ ����.
    D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;  // ���۰� ��� �������� ����������(written) ����. 
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount; // ������ ũ��(����Ʈ)
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // ���۰� ���������ο� ��� ������(� ���� ����)�� ����.
    vertexBufferDesc.CPUAccessFlags = 0; // CPU ���� �÷���. 0�̸� CPU ������ �ʿ� ���ٴ� ��.
    vertexBufferDesc.MiscFlags = 0; // ��Ÿ �÷���. 0�̸� ������� �ʰڴٴ� ��.
    vertexBufferDesc.StructureByteStride = 0;

    // ���� ���ۿ� ���� �� ���� �����͸� ����.
    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = m_pVertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // ���� ���� ����.
    GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_pVertexBuffer));

    // �ʿ� ������ ���� �迭 ����.
    delete[] m_pVertices;
    m_pVertices = nullptr;

    // �ε��� ������ �����ڸ� ����.
    D3D11_BUFFER_DESC indexBufferDesc;
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = sizeof(int);

    // �ε��� ���ۿ� ���� subresource ����ü�� ����.
    D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = m_pIndices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // �ε��� ���� ����.
    GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer));

    delete[] m_pIndices;
    m_pIndices = nullptr;

    // �ν��Ͻ� �迭�� �� �ν��Ͻ��� ������ ����.
    m_instanceCount = 4;

    // �ν��Ͻ� �迭 ����.
    InstanceType* instances = nullptr;
    instances = new InstanceType[m_instanceCount];
    ALLOCATE_EXCEPT(instances, "Can't allocate model vertex array!")

    // �ν��Ͻ� �迭�� ä����. �� �ν��Ͻ����� ��ġ ��.
    instances[0].position = DirectX::XMFLOAT3(-1.5f, -1.5f, 5.0f);
    instances[1].position = DirectX::XMFLOAT3(-1.5f, 1.5f, 5.0f);
    instances[2].position = DirectX::XMFLOAT3(1.5f, -1.5f, 5.0f);
    instances[3].position = DirectX::XMFLOAT3(1.5f, 1.5f, 5.0f);

    // �ν��Ͻ� ���� ������ ����.
    D3D11_BUFFER_DESC instanceBufferDesc;
    instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    instanceBufferDesc.ByteWidth = sizeof(InstanceType) * m_instanceCount;
    instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instanceBufferDesc.CPUAccessFlags = 0;
    instanceBufferDesc.MiscFlags = 0;
    instanceBufferDesc.StructureByteStride = 0;

    // �ν��Ͻ� ���ۿ� �� �ν��Ͻ� ������ ����.
    D3D11_SUBRESOURCE_DATA instanceData;
    instanceData.pSysMem = instances;
    instanceData.SysMemPitch = 0;
    instanceData.SysMemSlicePitch = 0;

    // �ν��Ͻ� ���� ����.
    GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&instanceBufferDesc, &instanceData, &m_pInstanceBuffer));

    // �ʿ� ������ �ν��Ͻ� �迭 ����.
    delete[] instances;
}

void InstanceModel::BindBuffers(D3DGraphics& gfx)
{
    unsigned int strides[2];
    unsigned int offsets[2];
    ID3D11Buffer* bufferPointers[2];

    // ���� stride ����.
    strides[0] = sizeof(VertexType);
    strides[1] = sizeof(InstanceType);

    // ���� offset���� ����.
    offsets[0] = 0;
    offsets[1] = 0;

    // ���� ������ �迭�� ���� ������ �ν��Ͻ� ���۷� ��������.
    bufferPointers[0] = m_pVertexBuffer.Get();
    bufferPointers[1] = m_pInstanceBuffer.Get();

    // ���������ο� ���۵��� ���ε� ����. (���� ���̴��� ���ε�.)
    gfx.GetContext()->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

    // ���������ο� �ε��� ���� ���ε�.
    gfx.GetContext()->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // Primitive Topology�� �ﰢ������ ����.
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

    // �� ������ ������.
    file.open(filePath);
    if (file.fail()) STD_EXCEPT("Can't open a model file!")

    // �� ���Ͽ� �ִ� ���� ���� ���� �о���.
    file.get(input);
    while (input != ':')
    {
        file.get(input);
    }
    file >> m_vertexCount;

    // �ε��� ������ ������ ������ �Ȱ��� ����.
    m_indexCount = m_vertexCount;

    // �о���� ���� ���� ��ŭ ���� Ÿ�� ����ü �迭�� �Ҵ�����.
    m_pModel = new ModelType[m_vertexCount];
    ALLOCATE_EXCEPT(m_pModel, "Can't allocate model vertex array!")

    // ���� ���� ���� �����͸� �о��.
    file.get(input);
    while (input != ':')
    {
        file.get(input);
    }
    file.get(input); // \n
    file.get(input); // \r

    for (int i = 0; i < m_vertexCount; i++)
    {
        file >> m_pModel[i].x >> m_pModel[i].y >> m_pModel[i].z;     // ��ġ
        file >> m_pModel[i].tu >> m_pModel[i].tv;                    // �ؽ��� ��ǥ(UV)
    }

    // ���� �迭 ����.
    m_pVertices = new VertexType[m_vertexCount];
    ALLOCATE_EXCEPT(m_pVertices, "Can't allocate vertex array!")

    // �ε��� �迭 ����.
    m_pIndices = new unsigned long[m_indexCount];
    ALLOCATE_EXCEPT(m_pIndices, "Can't allocate index array!")

    // ���Ϸκ��� �о�� ���� �����͸� ������ �ε��� �迭�� ä����.
    for (int i = 0; i < m_vertexCount; i++)
    {
        m_pVertices[i].position = DirectX::XMFLOAT3(m_pModel[i].x, m_pModel[i].y, m_pModel[i].z);
        m_pVertices[i].texture = DirectX::XMFLOAT2(m_pModel[i].tu, m_pModel[i].tv);
        m_pIndices[i] = i;
    }

    // ���� �ݾ���.
    file.close();
}

