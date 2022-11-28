#include "ParticleSystem.h"
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <ErrorHandle/StandardException.h>
#include <Graphics/Texture.h>

ParticleSystem::ParticleSystem()
{
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::Initialize(D3DGraphics& gfx, std::string textureFilePath)
{
    LoadTexture(gfx, textureFilePath);
    InitializeParticleSystem();
    InitializeBuffers(gfx);
}

void ParticleSystem::Shutdown()
{
    ShutdownParticleSystem();
    ReleaseTexture();
}

void ParticleSystem::Frame(float deltaTime, D3DGraphics& gfx)
{
    // 이전 파티클들을 모두 제거.
    KillParticles();

    // 새로운 파티클들을 생성.
    EmitParticles(deltaTime);

    // 파티클들의 위치를 갱신.
    UpdateParticles(deltaTime);

    // 파티클 위치를 갱신 했으므로, 이를 상수 버퍼에 반영해줌.
    UpdateBuffers(gfx);
}

void ParticleSystem::Bind(D3DGraphics& gfx)
{
    BindBuffers(gfx);
}

unsigned int ParticleSystem::GetIndexCount()
{
    return m_indexCount;
}

ID3D11ShaderResourceView* ParticleSystem::GetTexture()
{
    return m_Texture->GetTextureView();
}

void ParticleSystem::InitializeParticleSystem()
{
    // 파티클이 방출될 때 어디에 위치할 지 결정하는 무작위 편차를 설정.
    m_particleDeviationX = 0.5f;
    m_particleDeviationY = 0.1f;
    m_particleDeviationZ = 2.0f;

    // 파티클 속도와 속도 차이 값을 설정.
    m_particleVelocity = 20.f;
    m_particleVelocityVariation = 0.2f;

    // 파티클의 크기를 설정.
    m_particleSize = 0.2f;

    // 초당 방출할 파티클의 개수를 설정함.
    m_particlesPerSecond = 500.f;

    // 파티클 시스템에서 허용할 파티클의 최대 개수를 설정.
    m_maxParticles = 5000;

    // 파티클 리스트를 생성하고 초기화.
    m_particleList = new ParticleType[m_maxParticles];
    ALLOCATE_EXCEPT(m_particleList, "Can't allocate a particle list!")
    for (int i = 0; i < m_maxParticles; ++i)
        m_particleList[i].active = false;

    // 현재 생성되어 방출되고 있는 파티클의 개수를 0으로 초기화.
    m_currentParticleCount = 0;

    m_accumulatedTime = 0.0f;
}

void ParticleSystem::ShutdownParticleSystem()
{
    if (m_pVertices)
    {
        delete[] m_pVertices;
        m_pVertices = nullptr;
    }
    if (m_particleList)
    {
        delete[] m_particleList;
        m_particleList = nullptr;
    }
}

void ParticleSystem::InitializeBuffers(D3DGraphics& gfx)
{
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc; // 정점, 인덱스 데이터가 들어갈 버퍼.
    D3D11_SUBRESOURCE_DATA vertexData, indexData;        // 실제 정점, 인덱스 버퍼에 들어갈 데이터

    // 정점 배열에 있는 정점들의 최대 개수를 설정함.
    m_vertexCount = m_maxParticles * 6;
    // 인덱스 배열에 있는 인덱스들의 최대 개수를 설정함.
    m_indexCount = m_vertexCount;
    
    // 렌더링 해 줄 파티클들의 정점 배열을 생성.
    m_pVertices = new VertexType[m_vertexCount];
    ALLOCATE_EXCEPT(m_pVertices, "Can not allocate a Vertex array!")

    // 인덱스 배열 생성.
    indices = new unsigned long[m_indexCount];
    ALLOCATE_EXCEPT(indices, "Can not allocate a Index array!")

    // 정점 배열을 0으로 초기화.
    memset(m_pVertices, 0, (sizeof(VertexType) * m_vertexCount));

    // 인덱스 배열 초기화.
    for (int i = 0; i < m_indexCount; ++i)
        indices[i] = i;

    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    vertexData.pSysMem = m_pVertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    INFOMAN(gfx)
    // 정점 버퍼 생성.
    GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_pVertexBuffer));

    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    GFX_THROW_INFO(gfx.GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer));

    delete[] indices;
    indices = nullptr;
}

void ParticleSystem::UpdateBuffers(D3DGraphics& gfx)
{
    int index, i;
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    VertexType* verticesPtr;

    // 정점 배열 0으로 초기화.
    memset(m_pVertices, 0, (sizeof(VertexType) * m_vertexCount));

    // 파티클 리스트 배열을 통해 정점 배열을 만들어줌. 각 파티클은 두 개의 삼각형으로 구성된 사각형.
    index = 0;

    for (i = 0; i < m_currentParticleCount; i++)
    {
        // Bottom left.
        m_pVertices[index].position = DirectX::XMFLOAT3(m_particleList[i].positionX - m_particleSize, m_particleList[i].positionY - m_particleSize, m_particleList[i].positionZ);
        m_pVertices[index].texture = DirectX::XMFLOAT2(0.0f, 1.0f);
        m_pVertices[index].color = DirectX::XMFLOAT4(m_particleList[i].red, m_particleList[i].green, m_particleList[i].blue, 1.0f);
        index++;

        // Top left.
        m_pVertices[index].position = DirectX::XMFLOAT3(m_particleList[i].positionX - m_particleSize, m_particleList[i].positionY + m_particleSize, m_particleList[i].positionZ);
        m_pVertices[index].texture = DirectX::XMFLOAT2(0.0f, 0.0f);
        m_pVertices[index].color = DirectX::XMFLOAT4(m_particleList[i].red, m_particleList[i].green, m_particleList[i].blue, 1.0f);
        index++;

        // Bottom right.
        m_pVertices[index].position = DirectX::XMFLOAT3(m_particleList[i].positionX + m_particleSize, m_particleList[i].positionY - m_particleSize, m_particleList[i].positionZ);
        m_pVertices[index].texture = DirectX::XMFLOAT2(1.0f, 1.0f);
        m_pVertices[index].color = DirectX::XMFLOAT4(m_particleList[i].red, m_particleList[i].green, m_particleList[i].blue, 1.0f);
        index++;

        // Bottom right.
        m_pVertices[index].position = DirectX::XMFLOAT3(m_particleList[i].positionX + m_particleSize, m_particleList[i].positionY - m_particleSize, m_particleList[i].positionZ);
        m_pVertices[index].texture = DirectX::XMFLOAT2(1.0f, 1.0f);
        m_pVertices[index].color = DirectX::XMFLOAT4(m_particleList[i].red, m_particleList[i].green, m_particleList[i].blue, 1.0f);
        index++;

        // Top left.
        m_pVertices[index].position = DirectX::XMFLOAT3(m_particleList[i].positionX - m_particleSize, m_particleList[i].positionY + m_particleSize, m_particleList[i].positionZ);
        m_pVertices[index].texture = DirectX::XMFLOAT2(0.0f, 0.0f);
        m_pVertices[index].color = DirectX::XMFLOAT4(m_particleList[i].red, m_particleList[i].green, m_particleList[i].blue, 1.0f);
        index++;

        // Top right.
        m_pVertices[index].position = DirectX::XMFLOAT3(m_particleList[i].positionX + m_particleSize, m_particleList[i].positionY + m_particleSize, m_particleList[i].positionZ);
        m_pVertices[index].texture = DirectX::XMFLOAT2(1.0f, 0.0f);
        m_pVertices[index].color = DirectX::XMFLOAT4(m_particleList[i].red, m_particleList[i].green, m_particleList[i].blue, 1.0f);
        index++;
    }

    // 정점 버퍼 Lock.
    gfx.GetContext()->Map(m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    // 정점 버퍼에 들어있는 데이터에 대한 포인터 얻어옴.
    verticesPtr = (VertexType*)mappedResource.pData;

    // 정점 버퍼에 정점 배열 데이터를 복사해 넣어줌.
    memcpy(verticesPtr, (void*)m_pVertices, (sizeof(VertexType) * m_vertexCount));

    // 정점 버퍼 Unlock.
    gfx.GetContext()->Unmap(m_pVertexBuffer.Get(), 0);

}

void ParticleSystem::BindBuffers(D3DGraphics& gfx)
{
    unsigned int stride;
    unsigned int offset;

    stride = sizeof(VertexType);
    offset = 0;

    gfx.GetContext()->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    gfx.GetContext()->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    gfx.GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ParticleSystem::LoadTexture(D3DGraphics& gfx, std::string filePath)
{
    m_Texture = new Texture(gfx, filePath);
}

void ParticleSystem::ReleaseTexture()
{
    if (m_Texture)
    {
        delete m_Texture;
        m_Texture = nullptr;
    }
}

void ParticleSystem::EmitParticles(float deltaTime)
{
    bool bEmitParticle, bFound;
    float posX, posY, posZ, velocity, r, g, b;
    int index, i, j;

    // 프레임 시간 누적해 경과 시간을 구함.
    m_accumulatedTime += deltaTime;

    // 파티클을 방출할 시간이 되면, bEmitParticle 값을 true로 변경해줌.
    bEmitParticle = false;
    if (m_accumulatedTime > (1000.0f / m_particlesPerSecond))
    {
        m_accumulatedTime = 0.0f;
        bEmitParticle = true;
    }

    // 방출할 파티클이 있다면, 프레임 당 하나를 방출시켜 줌.
    if ((bEmitParticle == true) && (m_currentParticleCount < (m_maxParticles - 1)))
    {
        m_currentParticleCount++;

        // 파티클 속성들을 무작위로 설정해줌.
        posX = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationX;
        posY = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationY;
        posZ = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationZ;

        velocity = m_particleVelocity + (((float)rand() - (float)rand()) / RAND_MAX) * m_particleVelocityVariation;

        r = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;
        g = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;
        b = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;

        // 파티클은 블렌딩을 위해 뒤에서 부터 앞으로 렌더링 되어야 하기 때문에 깊이 값(Z 값) 기준으로 배열을 정렬 해주어야 함.
        // 새로 생성된 파티클에 대해 기존에 리스트에 있는 파티클과 비교해 어디에 삽입되야 할 지 결정해줌.
        index = 0;
        bFound = false;
        while (!bFound)
        {
            if ((m_particleList[index].active == false) || (m_particleList[index].positionZ < posZ))
            {
                bFound = true;
            }
            else
            {
                index++;
            }
        }

        // 이제 어느 위치에 새 파티클을 삽입할 지 알았기 때문에 해당 위치에서 부터 배열을 하나씩 뒤로 밀어서 새 파티클이 들어갈 공간을 마련해줌.
        i = m_currentParticleCount;
        j = i - 1;

        while (i != index)
        {
            m_particleList[i].positionX = m_particleList[j].positionX;
            m_particleList[i].positionY = m_particleList[j].positionY;
            m_particleList[i].positionZ = m_particleList[j].positionZ;
            m_particleList[i].red = m_particleList[j].red;
            m_particleList[i].green = m_particleList[j].green;
            m_particleList[i].blue = m_particleList[j].blue;
            m_particleList[i].velocity = m_particleList[j].velocity;
            m_particleList[i].active = m_particleList[j].active;
            i--;
            j--;
        }

        // 새 파티클을 리스트에 넣어줌.
        m_particleList[index].positionX = posX;
        m_particleList[index].positionY = posY;
        m_particleList[index].positionZ = posZ;
        m_particleList[index].red = r;
        m_particleList[index].green = g;
        m_particleList[index].blue = b;
        m_particleList[index].velocity = velocity;
        m_particleList[index].active = true;
    }
}

void ParticleSystem::UpdateParticles(float deltaTime)
{
    // 위치, 속도, 프레임 시간을 이용해 파티클들을 아래쪽으로 이동 시킴.(폭포 효과 구현을 위함.)
    for (int i = 0; i < m_currentParticleCount; ++i)
    {
        m_particleList[i].positionY = m_particleList[i].positionY - (m_particleList[i].velocity * deltaTime * 0.001f);
    }
}

void ParticleSystem::KillParticles()
{
    int i, j; // 생각해보니 매번 반복문 실행 마다 인덱스 변수를 생성하는 것 보다 미리 선언하는 것이 나을 것 같음.

    // 특정 높이 범위에 다다른 파티클들을 제거해줌.
    for (i = 0; i < m_maxParticles; i++)
    {
        if ((m_particleList[i].active == true) && (m_particleList[i].positionY < -3.0f))
        {
            m_particleList[i].active = false;
            m_currentParticleCount--;

            // 제거된 파티클 기준으로 다시 리스트를 앞으로 당겨줌.
            for (j = i; j < m_maxParticles - 1; j++)
            {
                m_particleList[j].positionX = m_particleList[j + 1].positionX;
                m_particleList[j].positionY = m_particleList[j + 1].positionY;
                m_particleList[j].positionZ = m_particleList[j + 1].positionZ;
                m_particleList[j].red = m_particleList[j + 1].red;
                m_particleList[j].green = m_particleList[j + 1].green;
                m_particleList[j].blue = m_particleList[j + 1].blue;
                m_particleList[j].velocity = m_particleList[j + 1].velocity;
                m_particleList[j].active = m_particleList[j + 1].active;
            }
        }
    }
}
