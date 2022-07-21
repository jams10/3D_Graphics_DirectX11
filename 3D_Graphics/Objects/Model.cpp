#include "Model.h"
#include <stdexcept>
#include <imgui/imgui.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <ErrorHandle/StandardException.h>
#include <Graphics/Texture.h>
#include <fstream>

Model::Model()
    :
    m_vertexCount(0),
    m_indexCount(0)
{
    m_pTexture = nullptr;
    m_pModel = nullptr;
    Reset();
}

Model::~Model()
{
}

void Model::Initialize(D3DGraphics& gfx, std::string modelFilePath, std::string textureFilePath)
{
    LoadModel(modelFilePath);

    InitializeBuffers(gfx);

    LoadTexture(gfx, textureFilePath);
}

void Model::Bind(D3DGraphics& gfx)
{
    BindBuffers(gfx);
}

ID3D11ShaderResourceView* Model::GetTexture()
{
    return m_pTexture->GetTextureView();
}

void Model::InitializeBuffers(D3DGraphics& gfx)
{
    VertexType* vertices = nullptr;
    unsigned long* indices = nullptr;

    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc; // ����, �ε��� �����Ͱ� �� ����.
    D3D11_SUBRESOURCE_DATA vertexData, indexData;        // ���� ����, �ε��� ���ۿ� �� ������

    // ���� �迭 ����.
    vertices = new VertexType[m_vertexCount];
    ALLOCATE_EXCEPT(vertices, "Can't allocate vertex array!")

    // �ε��� �迭 ����.
    indices = new unsigned long[m_indexCount];
    ALLOCATE_EXCEPT(indices, "Can't allocate index array!")

    // ���Ϸκ��� �о�� ���� �����͸� ������ �ε��� �迭�� ä����.
    for (int i = 0; i < m_vertexCount; i++)
    {
        vertices[i].position = DirectX::XMFLOAT3(m_pModel[i].x, m_pModel[i].y, m_pModel[i].z);
        vertices[i].texture = DirectX::XMFLOAT2(m_pModel[i].tu, m_pModel[i].tv);
        vertices[i].normal = DirectX::XMFLOAT3(m_pModel[i].nx, m_pModel[i].ny, m_pModel[i].nz);

        indices[i] = i;
    }

    // ���� ���� ������ �����ڸ� ����.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;  // ���۰� ��� �������� ����������(written) ����. 
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount; // ������ ũ��(����Ʈ)
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // ���۰� ���������ο� ��� ������(� ���� ����)�� ����.
    vertexBufferDesc.CPUAccessFlags = 0; // CPU ���� �÷���. 0�̸� CPU ������ �ʿ� ���ٴ� ��.
    vertexBufferDesc.MiscFlags = 0; // ��Ÿ �÷���. 0�̸� ������� �ʰڴٴ� ��.
    vertexBufferDesc.StructureByteStride = sizeof(VertexType); // ���ۿ� ����ִ� �� ������ ũ��.

    // ���� ���ۿ� ���� subresource ����ü�� ����.
    vertexData.pSysMem = vertices; // �ʱ�ȭ �����Ϳ� ���� ������. ���⿡ �츮�� ���� �����͸� �־���.
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    INFOMAN(gfx)

    // ���� ���� ����.
    GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_pVertexBuffer));

    // �ε��� ������ �����ڸ� ����.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = sizeof(int);

    // �ε��� ���ۿ� ���� subresource ����ü�� ����.
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // �ε��� ���� ����.
    GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer));

    // �ڿ��� ���� �����Ƿ�, ���� �Ҵ��� �迭�� �ʿ䰡 �����Ƿ� ���� ����.
    delete[] vertices;
    vertices = nullptr;

    delete[] indices;
    indices = nullptr;
}

void Model::BindBuffers(D3DGraphics& gfx)
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

void Model::LoadTexture(D3DGraphics& gfx, std::string filePath)
{
    m_pTexture = new Texture(gfx, filePath);
    ALLOCATE_EXCEPT(m_pTexture, "Can't allocate a texture instance!")
}

void Model::ReleaseTexture()
{
    if (m_pTexture)
    {
        delete m_pTexture;
        m_pTexture = nullptr;
    }
}

void Model::LoadModel(std::string filePath)
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
        file >> m_pModel[i].x  >> m_pModel[i].y  >> m_pModel[i].z;   // ��ġ
        file >> m_pModel[i].tu >> m_pModel[i].tv;                    // �ؽ��� ��ǥ(UV)
        file >> m_pModel[i].nx >> m_pModel[i].ny >> m_pModel[i].nz;  // �븻 ���� ��ǥ
    }

    // ���� �ݾ���.
    file.close();
}

void Model::ReleaseModel()
{
    if (m_pModel != nullptr)
    {
        delete[]m_pModel;
        m_pModel = nullptr;
    }
}

DirectX::XMMATRIX Model::GetWorldMatrix() const noexcept
{
    DirectX::XMMATRIX mat = DirectX::XMMatrixIdentity();

    mat *= (DirectX::XMMatrixRotationX(pitch) * DirectX::XMMatrixRotationY(yaw) * DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z));

    return mat;
}

void Model::SpawnControlWindow() noexcept
{
    if (ImGui::Begin("Model"))
    {
        ImGui::Text("Position");
        ImGui::SliderFloat("X", &pos.x, -80.0f, 80.0f, "%.1f");
        ImGui::SliderFloat("Y", &pos.y, -80.0f, 80.0f, "%.1f");
        ImGui::SliderFloat("Z", &pos.z, -80.0f, 80.0f, "%.1f");
        ImGui::Text("Orientation");
        ImGui::SliderAngle("Pitch", &pitch, -180.0f, 180.0f);
        ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f);
        if (ImGui::Button("Reset"))
        {
            Reset();
        }
    }
    ImGui::End();
}

void Model::Reset()
{
    pos = { 0.0f, 0.0f, 0.0f };
    pitch = 0.f;
    yaw = 0.0f;
}
