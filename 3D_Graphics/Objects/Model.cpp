#include "Model.h"
#include <stdexcept>
#include <imgui/imgui.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <Graphics/Texture.h>

Model::Model()
    :
    m_vertexCount(0),
    m_indexCount(0)
{
    Reset();
}

Model::~Model()
{
}

void Model::Initialize(D3DGraphics& gfx, std::string filePath)
{
    InitializeBuffers(gfx);

    LoadTexture(gfx, filePath);
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

    // ������ �ε��� ������ ����.
    m_vertexCount = 4;
    m_indexCount = 6;

    // ���� �迭 ����
    vertices = new VertexType[m_vertexCount];
    if (!vertices) throw std::runtime_error("Cannot create the vertex array");
    // �ε��� �迭 ����
    indices = new unsigned long[m_indexCount];
    if (!indices) throw std::runtime_error("Cannot create the index array");

    // ���� �迭�� ���� �����͸� ä����.
    vertices[0].position = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);
    vertices[0].texture = DirectX::XMFLOAT2(0.0f, 1.0f);
    vertices[0].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

    vertices[1].position = DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f);
    vertices[1].texture = DirectX::XMFLOAT2(0.0f, 0.0f);
    vertices[1].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

    vertices[2].position = DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f);
    vertices[2].texture = DirectX::XMFLOAT2(1.0f, 0.0f);
    vertices[2].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

    vertices[3].position = DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f);
    vertices[3].texture = DirectX::XMFLOAT2(1.0f, 1.0f);
    vertices[3].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

    // �ε��� �迭 ä���ֱ�.
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 2;
    indices[4] = 3;
    indices[5] = 0;

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
}

void Model::ReleaseTexture()
{
    if (m_pTexture)
    {
        delete m_pTexture;
        m_pTexture = nullptr;
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
