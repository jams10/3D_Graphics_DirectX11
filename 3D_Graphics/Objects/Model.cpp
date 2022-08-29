#include "Model.h"
#include <stdexcept>
#include <imgui/imgui.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <ErrorHandle/StandardException.h>
#include <Graphics/TextureArray.h>
#include <Utils/StringUtils.h>
#include <Utils/ObjFileLoader.h>
#include <fstream>
#include <sstream>

Model::Model()
    :
    m_vertexCount(0),
    m_indexCount(0)
{
    m_pVertices = nullptr;
    m_pIndices = nullptr;
    m_pTextureArray = nullptr;
    m_pModel = nullptr;
    Reset();
}

Model::~Model()
{
}

void Model::Initialize(D3DGraphics& gfx, std::string modelFilePath, 
                       std::string textureFilePath1, std::string textureFilePath2, std::string textureFilePath3)
{
    LoadModel(modelFilePath);

    InitializeBuffers(gfx);

    LoadTextures(gfx, textureFilePath1, textureFilePath2, textureFilePath3);
}

void Model::Bind(D3DGraphics& gfx)
{
    BindBuffers(gfx);
}

ID3D11ShaderResourceView** Model::GetTextureArray()
{
    return m_pTextureArray->GetTextureArray();
}

void Model::InitializeBuffers(D3DGraphics& gfx)
{
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc; // ����, �ε��� �����Ͱ� �� ����.
    D3D11_SUBRESOURCE_DATA vertexData, indexData;        // ���� ����, �ε��� ���ۿ� �� ������

    // ���� ���� ������ �����ڸ� ����.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;  // ���۰� ��� �������� ����������(written) ����. 
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount; // ������ ũ��(����Ʈ)
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // ���۰� ���������ο� ��� ������(� ���� ����)�� ����.
    vertexBufferDesc.CPUAccessFlags = 0; // CPU ���� �÷���. 0�̸� CPU ������ �ʿ� ���ٴ� ��.
    vertexBufferDesc.MiscFlags = 0; // ��Ÿ �÷���. 0�̸� ������� �ʰڴٴ� ��.
    vertexBufferDesc.StructureByteStride = sizeof(VertexType); // ���ۿ� ����ִ� �� ������ ũ��.

    // ���� ���ۿ� ���� subresource ����ü�� ����.
    vertexData.pSysMem = m_pVertices; // �ʱ�ȭ �����Ϳ� ���� ������. ���⿡ �츮�� ���� �����͸� �־���.
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
    indexData.pSysMem = m_pIndices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // �ε��� ���� ����.
    GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer));

    // �ڿ��� ���� �����Ƿ�, ���� �Ҵ��� �迭�� �ʿ䰡 �����Ƿ� ���� ����.
    delete[] m_pVertices;
    m_pVertices = nullptr;

    delete[] m_pIndices;
    m_pIndices = nullptr;
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

void Model::LoadTextures(D3DGraphics& gfx, std::string filePath1, std::string filePath2, std::string filePath3)
{
    m_pTextureArray = new TextureArray();
    m_pTextureArray->Initialize(gfx, filePath1, filePath2, filePath3);
    ALLOCATE_EXCEPT(m_pTextureArray, "Can't allocate a texture instance!")
}

void Model::ReleaseTexture()
{
    if (m_pTextureArray)
    {
        m_pTextureArray->Shutdown();
        delete m_pTextureArray;
        m_pTextureArray = nullptr;
    }
}

void Model::LoadModel(std::string filePath)
{
    std::vector<std::string> splited = SplitString(filePath, '.');
    FileFormat = splited[1];
    if (FileFormat == "model") LoadCustomFile(filePath);
    else if (FileFormat == "obj") LoadObjFile(filePath);
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

    if (FileFormat == "obj")
    {
        if (ImGui::Begin("ObjModelInfo"))
        {
            ImGui::Text(ToNarrow(FileInfoString).c_str());
        }
        ImGui::End();
    }
}

void Model::Reset()
{
    pos = { 0.0f, 0.0f, 0.0f };
    pitch = 0.f;
    yaw = 0.0f;
}

void Model::LoadCustomFile(std::string filePath)
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
        file >> m_pModel[i].nx >> m_pModel[i].ny >> m_pModel[i].nz;  // �븻 ���� ��ǥ
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
        m_pVertices[i].normal = DirectX::XMFLOAT3(m_pModel[i].nx, m_pModel[i].ny, m_pModel[i].nz);

        m_pIndices[i] = i;
    }

    // ���� �ݾ���.
    file.close();
}

void Model::LoadObjFile(std::string filePath)
{
    FileInfo info;
    std::vector<Float3Type> vertices;
    std::vector<Float3Type> texcoords;
    std::vector<Float3Type> normals;
    std::vector<FaceType> faces;
    info = GetModelInfo(filePath);
    TransformDataToDXFormat(filePath, info, vertices, texcoords, normals, faces);

    std::wstringstream ss;
    std::wstring tri = info.isTriangulated ? L"Yes" : L"No";
    ss << "Vertex Count : " << info.vertexCount << "\n" << "Texture Coord Count : " << info.textureCount << "\n"
        << "Normal Vector Count : " << info.normalCount << "\n" << "Is Triangulated : " << tri;

    FileInfoString = ss.str();

    // ���Ϸκ��� ���� ���� ������ ���� �� ����������������
    int vIdx = 0, tIdx = 0, nIdx = 0, idx = 0;

    // ���� �ͽ���Ʈ �� �� triangulate �ɼ��� ��� ������, �� ���� �ﰢ�� ������ �����Ǿ� ����.
    if (info.isTriangulated)
    {
        m_vertexCount = info.faceCount * 3;
        m_indexCount = m_vertexCount;
    }
    else
    {
        // !!! �̴� ������ü���� �����. �𵨸��� ǥ���� �����ϴ� ������ ������ �ٸ� �� ����.(4���� �ƴ� �� ����.)
        m_vertexCount = info.faceCount * 4;
        m_indexCount = m_vertexCount + info.faceCount * 2;
    }

    m_pModel = new ModelType[m_vertexCount];

    for (int i = 0; i < info.faceCount; ++i)
    {
        vIdx = faces[i].vIndex1 - 1;
        tIdx = faces[i].tIndex1 - 1;
        nIdx = faces[i].nIndex1 - 1;

        m_pModel[idx].x = vertices[vIdx].x;
        m_pModel[idx].y = vertices[vIdx].y;
        m_pModel[idx].z = vertices[vIdx].z;
        m_pModel[idx].tu = texcoords[tIdx].x;
        m_pModel[idx].tv = texcoords[tIdx].y;
        m_pModel[idx].nx = normals[nIdx].x;
        m_pModel[idx].ny = normals[nIdx].y;
        m_pModel[idx].nz = normals[nIdx].z;

        vIdx = faces[i].vIndex2 - 1;
        tIdx = faces[i].tIndex2 - 1;
        nIdx = faces[i].nIndex2 - 1;
        idx++;

        m_pModel[idx].x = vertices[vIdx].x;
        m_pModel[idx].y = vertices[vIdx].y;
        m_pModel[idx].z = vertices[vIdx].z;
        m_pModel[idx].tu = texcoords[tIdx].x;
        m_pModel[idx].tv = texcoords[tIdx].y;
        m_pModel[idx].nx = normals[nIdx].x;
        m_pModel[idx].ny = normals[nIdx].y;
        m_pModel[idx].nz = normals[nIdx].z;

        vIdx = faces[i].vIndex3 - 1;
        tIdx = faces[i].tIndex3 - 1;
        nIdx = faces[i].nIndex3 - 1;
        idx++;

        m_pModel[idx].x = vertices[vIdx].x;
        m_pModel[idx].y = vertices[vIdx].y;
        m_pModel[idx].z = vertices[vIdx].z;
        m_pModel[idx].tu = texcoords[tIdx].x;
        m_pModel[idx].tv = texcoords[tIdx].y;
        m_pModel[idx].nx = normals[nIdx].x;
        m_pModel[idx].ny = normals[nIdx].y;
        m_pModel[idx].nz = normals[nIdx].z;

        if (!info.isTriangulated)
        {
            vIdx = faces[i].vIndex4 - 1;
            tIdx = faces[i].tIndex4 - 1;
            nIdx = faces[i].nIndex4 - 1;
            idx++;

            m_pModel[idx].x = vertices[vIdx].x;
            m_pModel[idx].y = vertices[vIdx].y;
            m_pModel[idx].z = vertices[vIdx].z;
            m_pModel[idx].tu = texcoords[tIdx].x;
            m_pModel[idx].tv = texcoords[tIdx].y;
            m_pModel[idx].nx = normals[nIdx].x;
            m_pModel[idx].ny = normals[nIdx].y;
            m_pModel[idx].nz = normals[nIdx].z;
        }
        
        idx++;
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
        m_pVertices[i].normal = DirectX::XMFLOAT3(m_pModel[i].nx, m_pModel[i].ny, m_pModel[i].nz);
    }

    if (info.isTriangulated)
    {
        for (int i = 0; i < m_indexCount; ++i)
        {
            m_pIndices[i] = i;
        }
    }

    int curface = 0;
    for (int i = 0; i < m_indexCount; i+=6)
    {
        m_pIndices[i]     = curface;
        m_pIndices[i + 1] = curface + 1;
        m_pIndices[i + 2] = curface + 2;

        m_pIndices[i + 3] = curface + 3;
        m_pIndices[i + 4] = curface;
        m_pIndices[i + 5] = curface + 2;
        curface += 4;
    }
}


