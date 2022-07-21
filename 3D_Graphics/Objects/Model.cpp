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

    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc; // 정점, 인덱스 데이터가 들어갈 버퍼.
    D3D11_SUBRESOURCE_DATA vertexData, indexData;        // 실제 정점, 인덱스 버퍼에 들어갈 데이터

    // 정점 배열 생성.
    vertices = new VertexType[m_vertexCount];
    ALLOCATE_EXCEPT(vertices, "Can't allocate vertex array!")

    // 인덱스 배열 생성.
    indices = new unsigned long[m_indexCount];
    ALLOCATE_EXCEPT(indices, "Can't allocate index array!")

    // 파일로부터 읽어온 정점 데이터를 정점과 인덱스 배열에 채워줌.
    for (int i = 0; i < m_vertexCount; i++)
    {
        vertices[i].position = DirectX::XMFLOAT3(m_pModel[i].x, m_pModel[i].y, m_pModel[i].z);
        vertices[i].texture = DirectX::XMFLOAT2(m_pModel[i].tu, m_pModel[i].tv);
        vertices[i].normal = DirectX::XMFLOAT3(m_pModel[i].nx, m_pModel[i].ny, m_pModel[i].nz);

        indices[i] = i;
    }

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
        file >> m_pModel[i].x  >> m_pModel[i].y  >> m_pModel[i].z;   // 위치
        file >> m_pModel[i].tu >> m_pModel[i].tv;                    // 텍스쳐 좌표(UV)
        file >> m_pModel[i].nx >> m_pModel[i].ny >> m_pModel[i].nz;  // 노말 벡터 좌표
    }

    // 파일 닫아줌.
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
