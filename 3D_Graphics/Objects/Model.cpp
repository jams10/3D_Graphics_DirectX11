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

    CalculateModelVectors();

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
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc; // 정점, 인덱스 데이터가 들어갈 버퍼.
    D3D11_SUBRESOURCE_DATA vertexData, indexData;        // 실제 정점, 인덱스 버퍼에 들어갈 데이터

    // 정적 정점 버퍼의 서술자를 설정.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;  // 버퍼가 어떻게 읽혀지고 쓰여지는지(written) 정의. 
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount; // 버퍼의 크기(바이트)
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // 버퍼가 파이프라인에 어떻게 묶일지(어떤 버퍼 인지)를 정의.
    vertexBufferDesc.CPUAccessFlags = 0; // CPU 접근 플래그. 0이면 CPU 접근이 필요 없다는 것.
    vertexBufferDesc.MiscFlags = 0; // 기타 플래그. 0이면 사용하지 않겠다는 것.
    vertexBufferDesc.StructureByteStride = sizeof(VertexType); // 버퍼에 들어있는 각 원소의 크기.

    // 정점 버퍼에 대한 subresource 구조체를 설정.
    vertexData.pSysMem = m_pVertices; // 초기화 데이터에 대한 포인터. 여기에 우리의 정점 데이터를 넣어줌.
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
    indexData.pSysMem = m_pIndices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // 인덱스 버퍼 생성.
    GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer));

    // 자원을 생성 했으므로, 동적 할당한 배열은 필요가 없으므로 해제 해줌.
    delete[] m_pVertices;
    m_pVertices = nullptr;

    delete[] m_pIndices;
    m_pIndices = nullptr;
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
        file >> m_pModel[i].nx >> m_pModel[i].ny >> m_pModel[i].nz;  // 노말 벡터 좌표
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
        m_pVertices[i].normal = DirectX::XMFLOAT3(m_pModel[i].nx, m_pModel[i].ny, m_pModel[i].nz);

        m_pIndices[i] = i;
    }

    // 파일 닫아줌.
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

    // 파일로부터 읽은 정점 구성을 토대로 모델 ㄴㅇㄹㄴㅇㄹㅇㄹ
    int vIdx = 0, tIdx = 0, nIdx = 0, idx = 0;

    // 모델을 익스포트 할 때 triangulate 옵션을 사용 했으면, 각 면은 삼각형 단위로 정리되어 나옴.
    if (info.isTriangulated)
    {
        m_vertexCount = info.faceCount * 3;
        m_indexCount = m_vertexCount;
    }
    else
    {
        // !!! 이는 정육면체에만 적용됨. 모델마다 표면을 구성하는 정점의 개수가 다를 수 있음.(4개가 아닐 수 있음.)
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

void Model::CalculateModelVectors()
{
    int faceCount, i, index;
    TempVertexType vertex1, vertex2, vertex3;
    VectorType tangent, binormal, normal;

    // 삼각형을 구성하는 정점의 개수는 3개. 따라서 총 정점의 개수를 3으로 나눈 것이 표면의 개수.
    faceCount = m_vertexCount / 3;

    index = 0;

    // 모든 표면을 순회하면서 해당 표면의 tangent, binormal, normal 벡터들을 계산해줌.
    for (i = 0; i < faceCount; i++)
    {
        vertex1.x = m_pModel[index].x;
        vertex1.y = m_pModel[index].y;
        vertex1.z = m_pModel[index].z;
        vertex1.tu = m_pModel[index].tu;
        vertex1.tv = m_pModel[index].tv;
        vertex1.nx = m_pModel[index].nx;
        vertex1.ny = m_pModel[index].ny;
        vertex1.nz = m_pModel[index].nz;
        index++;

        vertex2.x = m_pModel[index].x;
        vertex2.y = m_pModel[index].y;
        vertex2.z = m_pModel[index].z;
        vertex2.tu = m_pModel[index].tu;
        vertex2.tv = m_pModel[index].tv;
        vertex2.nx = m_pModel[index].nx;
        vertex2.ny = m_pModel[index].ny;
        vertex2.nz = m_pModel[index].nz;
        index++;

        vertex3.x = m_pModel[index].x;
        vertex3.y = m_pModel[index].y;
        vertex3.z = m_pModel[index].z;
        vertex3.tu = m_pModel[index].tu;
        vertex3.tv = m_pModel[index].tv;
        vertex3.nx = m_pModel[index].nx;
        vertex3.ny = m_pModel[index].ny;
        vertex3.nz = m_pModel[index].nz;
        index++;

        // 해당 표면의 tangent와 binormal 값을 계산.
        CalculateTangentBinormal(vertex1, vertex2, vertex3, tangent, binormal);

        // 위에서 계산한 tangent, binormal 값을 통해 새 normal 값을 계산.
        CalculateNormal(tangent, binormal, normal);

        // 계산한 normal, tangent, binormal 값을 저장해줌.
        m_pModel[index - 1].nx = normal.x;
        m_pModel[index - 1].ny = normal.y;
        m_pModel[index - 1].nz = normal.z;
        m_pModel[index - 1].tx = tangent.x;
        m_pModel[index - 1].ty = tangent.y;
        m_pModel[index - 1].tz = tangent.z;
        m_pModel[index - 1].bx = binormal.x;
        m_pModel[index - 1].by = binormal.y;
        m_pModel[index - 1].bz = binormal.z;

        m_pModel[index - 2].nx = normal.x;
        m_pModel[index - 2].ny = normal.y;
        m_pModel[index - 2].nz = normal.z;
        m_pModel[index - 2].tx = tangent.x;
        m_pModel[index - 2].ty = tangent.y;
        m_pModel[index - 2].tz = tangent.z;
        m_pModel[index - 2].bx = binormal.x;
        m_pModel[index - 2].by = binormal.y;
        m_pModel[index - 2].bz = binormal.z;

        m_pModel[index - 3].nx = normal.x;
        m_pModel[index - 3].ny = normal.y;
        m_pModel[index - 3].nz = normal.z;
        m_pModel[index - 3].tx = tangent.x;
        m_pModel[index - 3].ty = tangent.y;
        m_pModel[index - 3].tz = tangent.z;
        m_pModel[index - 3].bx = binormal.x;
        m_pModel[index - 3].by = binormal.y;
        m_pModel[index - 3].bz = binormal.z;
    }

    return;
}

void Model::CalculateTangentBinormal(TempVertexType vertex1, TempVertexType vertex2, TempVertexType vertex3, VectorType& tangent, VectorType& binormal)
{
    float vector1[3], vector2[3];
    float tuVector[2], tvVector[2];
    float den;
    float length;

    // 정점 vertex1에서 vertex2로 가는 벡터를 계산.
    vector1[0] = vertex2.x - vertex1.x;
    vector1[1] = vertex2.y - vertex1.y;
    vector1[2] = vertex2.z - vertex1.z;
    // 정점 vertex1에서 vertex3로 가는 벡터를 계산.
    vector2[0] = vertex3.x - vertex1.x;
    vector2[1] = vertex3.y - vertex1.y;
    vector2[2] = vertex3.z - vertex1.z;

    // 텍스쳐 공간의 벡터들을 계산해줌.
    tuVector[0] = vertex2.tu - vertex1.tu;
    tvVector[0] = vertex2.tv - vertex1.tv;

    tuVector[1] = vertex3.tu - vertex1.tu;
    tvVector[1] = vertex3.tv - vertex1.tv;

    // 행렬로 유도할 수 있는 방정식을 통해 tangent 벡터와 binormal 벡터를 계산해줌.
    den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);
    tangent.x = (tvVector[1] * vector1[0] - tvVector[0] * vector2[0]) * den;
    tangent.y = (tvVector[1] * vector1[1] - tvVector[0] * vector2[1]) * den;
    tangent.z = (tvVector[1] * vector1[2] - tvVector[0] * vector2[2]) * den;

    binormal.x = (tuVector[0] * vector2[0] - tuVector[1] * vector1[0]) * den;
    binormal.y = (tuVector[0] * vector2[1] - tuVector[1] * vector1[1]) * den;
    binormal.z = (tuVector[0] * vector2[2] - tuVector[1] * vector1[2]) * den;

    // tangent 벡터 길이 계산.
    length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

    // 정규화.
    tangent.x = tangent.x / length;
    tangent.y = tangent.y / length;
    tangent.z = tangent.z / length;

    // binormal 벡터 길이 계산.
    length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

    // 정규화.
    binormal.x = binormal.x / length;
    binormal.y = binormal.y / length;
    binormal.z = binormal.z / length;

    return;
}

void Model::CalculateNormal(VectorType tangent, VectorType binormal, VectorType& normal) 
{
    float length;

    // tangent 벡터와 binormal 벡터의 외적을 통해 새 normal 값을 구해줌.
    normal.x = (tangent.y * binormal.z) - (tangent.z * binormal.y);
    normal.y = (tangent.z * binormal.x) - (tangent.x * binormal.z);
    normal.z = (tangent.x * binormal.y) - (tangent.y * binormal.x);

    // 노말 벡터의 길이를 계산.
    length = sqrt((normal.x * normal.x) + (normal.y * normal.y) + (normal.z * normal.z));

    // 정규화.
    normal.x = normal.x / length;
    normal.y = normal.y / length;
    normal.z = normal.z / length;

    return;
}
