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
    // ���� ��ƼŬ���� ��� ����.
    KillParticles();

    // ���ο� ��ƼŬ���� ����.
    EmitParticles(deltaTime);

    // ��ƼŬ���� ��ġ�� ����.
    UpdateParticles(deltaTime);

    // ��ƼŬ ��ġ�� ���� �����Ƿ�, �̸� ��� ���ۿ� �ݿ�����.
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
    // ��ƼŬ�� ����� �� ��� ��ġ�� �� �����ϴ� ������ ������ ����.
    m_particleDeviationX = 0.5f;
    m_particleDeviationY = 0.1f;
    m_particleDeviationZ = 2.0f;

    // ��ƼŬ �ӵ��� �ӵ� ���� ���� ����.
    m_particleVelocity = 20.f;
    m_particleVelocityVariation = 0.2f;

    // ��ƼŬ�� ũ�⸦ ����.
    m_particleSize = 0.2f;

    // �ʴ� ������ ��ƼŬ�� ������ ������.
    m_particlesPerSecond = 500.f;

    // ��ƼŬ �ý��ۿ��� ����� ��ƼŬ�� �ִ� ������ ����.
    m_maxParticles = 5000;

    // ��ƼŬ ����Ʈ�� �����ϰ� �ʱ�ȭ.
    m_particleList = new ParticleType[m_maxParticles];
    ALLOCATE_EXCEPT(m_particleList, "Can't allocate a particle list!")
    for (int i = 0; i < m_maxParticles; ++i)
        m_particleList[i].active = false;

    // ���� �����Ǿ� ����ǰ� �ִ� ��ƼŬ�� ������ 0���� �ʱ�ȭ.
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
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc; // ����, �ε��� �����Ͱ� �� ����.
    D3D11_SUBRESOURCE_DATA vertexData, indexData;        // ���� ����, �ε��� ���ۿ� �� ������

    // ���� �迭�� �ִ� �������� �ִ� ������ ������.
    m_vertexCount = m_maxParticles * 6;
    // �ε��� �迭�� �ִ� �ε������� �ִ� ������ ������.
    m_indexCount = m_vertexCount;
    
    // ������ �� �� ��ƼŬ���� ���� �迭�� ����.
    m_pVertices = new VertexType[m_vertexCount];
    ALLOCATE_EXCEPT(m_pVertices, "Can not allocate a Vertex array!")

    // �ε��� �迭 ����.
    indices = new unsigned long[m_indexCount];
    ALLOCATE_EXCEPT(indices, "Can not allocate a Index array!")

    // ���� �迭�� 0���� �ʱ�ȭ.
    memset(m_pVertices, 0, (sizeof(VertexType) * m_vertexCount));

    // �ε��� �迭 �ʱ�ȭ.
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
    // ���� ���� ����.
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

    // ���� �迭 0���� �ʱ�ȭ.
    memset(m_pVertices, 0, (sizeof(VertexType) * m_vertexCount));

    // ��ƼŬ ����Ʈ �迭�� ���� ���� �迭�� �������. �� ��ƼŬ�� �� ���� �ﰢ������ ������ �簢��.
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

    // ���� ���� Lock.
    gfx.GetContext()->Map(m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    // ���� ���ۿ� ����ִ� �����Ϳ� ���� ������ ����.
    verticesPtr = (VertexType*)mappedResource.pData;

    // ���� ���ۿ� ���� �迭 �����͸� ������ �־���.
    memcpy(verticesPtr, (void*)m_pVertices, (sizeof(VertexType) * m_vertexCount));

    // ���� ���� Unlock.
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

    // ������ �ð� ������ ��� �ð��� ����.
    m_accumulatedTime += deltaTime;

    // ��ƼŬ�� ������ �ð��� �Ǹ�, bEmitParticle ���� true�� ��������.
    bEmitParticle = false;
    if (m_accumulatedTime > (1000.0f / m_particlesPerSecond))
    {
        m_accumulatedTime = 0.0f;
        bEmitParticle = true;
    }

    // ������ ��ƼŬ�� �ִٸ�, ������ �� �ϳ��� ������� ��.
    if ((bEmitParticle == true) && (m_currentParticleCount < (m_maxParticles - 1)))
    {
        m_currentParticleCount++;

        // ��ƼŬ �Ӽ����� �������� ��������.
        posX = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationX;
        posY = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationY;
        posZ = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationZ;

        velocity = m_particleVelocity + (((float)rand() - (float)rand()) / RAND_MAX) * m_particleVelocityVariation;

        r = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;
        g = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;
        b = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;

        // ��ƼŬ�� ������ ���� �ڿ��� ���� ������ ������ �Ǿ�� �ϱ� ������ ���� ��(Z ��) �������� �迭�� ���� ���־�� ��.
        // ���� ������ ��ƼŬ�� ���� ������ ����Ʈ�� �ִ� ��ƼŬ�� ���� ��� ���ԵǾ� �� �� ��������.
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

        // ���� ��� ��ġ�� �� ��ƼŬ�� ������ �� �˾ұ� ������ �ش� ��ġ���� ���� �迭�� �ϳ��� �ڷ� �о �� ��ƼŬ�� �� ������ ��������.
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

        // �� ��ƼŬ�� ����Ʈ�� �־���.
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
    // ��ġ, �ӵ�, ������ �ð��� �̿��� ��ƼŬ���� �Ʒ������� �̵� ��Ŵ.(���� ȿ�� ������ ����.)
    for (int i = 0; i < m_currentParticleCount; ++i)
    {
        m_particleList[i].positionY = m_particleList[i].positionY - (m_particleList[i].velocity * deltaTime * 0.001f);
    }
}

void ParticleSystem::KillParticles()
{
    int i, j; // �����غ��� �Ź� �ݺ��� ���� ���� �ε��� ������ �����ϴ� �� ���� �̸� �����ϴ� ���� ���� �� ����.

    // Ư�� ���� ������ �ٴٸ� ��ƼŬ���� ��������.
    for (i = 0; i < m_maxParticles; i++)
    {
        if ((m_particleList[i].active == true) && (m_particleList[i].positionY < -3.0f))
        {
            m_particleList[i].active = false;
            m_currentParticleCount--;

            // ���ŵ� ��ƼŬ �������� �ٽ� ����Ʈ�� ������ �����.
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
