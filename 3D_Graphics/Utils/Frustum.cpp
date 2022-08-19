#include "Frustum.h"

Frustum::Frustum()
{
    m_frustumPlanes.resize(6);
}

void Frustum::ConstructFrustum(float screenDepth, DirectX::XMMATRIX& projMatrix, DirectX::XMMATRIX& viewMatrix)
{
    float near, ratio;
    DirectX::XMMATRIX matrix;
    DirectX::XMFLOAT4X4 fmatrix;
    DirectX::XMStoreFloat4x4(&fmatrix, projMatrix); // xmmatrix를 xmfloat4x4에 저장.

    // near 평면의 거리와 인자로 들어온 far 평면의 거리를 가지고 새로 투영 행렬의 값을 업데이트함.
    near = -fmatrix._43 / fmatrix._33;
    ratio = screenDepth / (screenDepth - near);
    fmatrix._33 = ratio;
    fmatrix._43 = -ratio * near;
    projMatrix = DirectX::XMLoadFloat4x4(&fmatrix); // xmfloat4x4를 xmmatrix에 적재.

    matrix = DirectX::XMMatrixMultiply(viewMatrix, projMatrix);
    DirectX::XMStoreFloat4x4(&fmatrix, matrix);

    // 근평면의 절두체 평면 벡터를 계산.
    m_frustumPlanes[0].x = fmatrix._14 + fmatrix._13;
    m_frustumPlanes[0].y = fmatrix._24 + fmatrix._23;
    m_frustumPlanes[0].z = fmatrix._34 + fmatrix._33;
    m_frustumPlanes[0].w = fmatrix._44 + fmatrix._43;
    
    // 원평면의 절두체 평면 벡터를 계산.
    m_frustumPlanes[1].x = fmatrix._14 - fmatrix._13;
    m_frustumPlanes[1].y = fmatrix._24 - fmatrix._23;
    m_frustumPlanes[1].z = fmatrix._34 - fmatrix._33;
    m_frustumPlanes[1].w = fmatrix._44 - fmatrix._43;

    // 좌측 평면의 절두체 평면 벡터를 계산.
    m_frustumPlanes[2].x = fmatrix._14 + fmatrix._11;
    m_frustumPlanes[2].y = fmatrix._24 + fmatrix._21;
    m_frustumPlanes[2].z = fmatrix._34 + fmatrix._31;
    m_frustumPlanes[2].w = fmatrix._44 + fmatrix._41;

    // 우측 평면의 절두체 평면 벡터를 계산.
    m_frustumPlanes[3].x = fmatrix._14 - fmatrix._11;
    m_frustumPlanes[3].y = fmatrix._24 - fmatrix._21;
    m_frustumPlanes[3].z = fmatrix._34 - fmatrix._31;
    m_frustumPlanes[3].w = fmatrix._44 - fmatrix._41;

    // 상단 평면의 절두체 평면 벡터를 계산.
    m_frustumPlanes[4].x = fmatrix._14 - fmatrix._12;
    m_frustumPlanes[4].y = fmatrix._24 - fmatrix._22;
    m_frustumPlanes[4].z = fmatrix._34 - fmatrix._32;
    m_frustumPlanes[4].w = fmatrix._44 - fmatrix._42;

    // 하단 평면의 절두체 평면 벡터를 계산.
    m_frustumPlanes[5].x = fmatrix._14 + fmatrix._12;
    m_frustumPlanes[5].y = fmatrix._24 + fmatrix._22;
    m_frustumPlanes[5].z = fmatrix._34 + fmatrix._32;
    m_frustumPlanes[5].w = fmatrix._44 + fmatrix._42;

    // 정규화된 평면의 방정식을 얻기 위해 정규화.
    // 이 때, 해당 평면의 방정식의 법선 벡터는 절두체 안쪽을 향하고 있음에 유의.
    for (int i = 0; i < 6; ++i)
    {
        float length = sqrt((m_frustumPlanes[i].x * m_frustumPlanes[i].x) + (m_frustumPlanes[i].y * m_frustumPlanes[i].y) + (m_frustumPlanes[i].z * m_frustumPlanes[i].z));
        m_frustumPlanes[i].x /= length;
        m_frustumPlanes[i].y /= length;
        m_frustumPlanes[i].z /= length;
        m_frustumPlanes[i].w /= length;
    }
}

float Frustum::GetDotResult(DirectX::XMFLOAT4 plane, DirectX::XMVECTOR v)
{
    DirectX::XMVECTOR result = DirectX::XMPlaneDot(DirectX::XMLoadFloat4(&plane), v);
    return DirectX::XMVectorGetX(result);
}

bool Frustum::CheckPoint(float x, float y, float z)
{
    // 인자로 주어진 점이 절두체 안에 있는지 체크.
    for (int i = 0; i < 6; i++)
    {
        if (GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet(x, y, z, 1)) < 0.0f)
        {
            return false;
        }
    }

    return true;
}

bool Frustum::CheckCube(float xCenter, float yCenter, float zCenter, float radius)
{
    // 인자로 주어진 정육면체의 중심 위치와 중심으로부터의 반경으로 절두체 영역안에 들어오는지 체크한다.
    // 중심 위치로부터 8개의 꼭짓점을 체크해서 꼭짓점이 하나라도 절두체 안에 들어오는 경우 그려줄 수 있도록 true를 리턴함.
    for (int i = 0; i < 6; i++)
    {
        if(GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet((xCenter - radius), (yCenter - radius), (zCenter - radius), 1)) >= 0.0f)
        {
            continue;
        }
        if(GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet((xCenter + radius), (yCenter - radius), (zCenter - radius), 1)) >= 0.0f)
        {
            continue;
        }
        if (GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet((xCenter - radius), (yCenter + radius), (zCenter - radius), 1)) >= 0.0f)
        {
            continue;
        }
        if (GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet((xCenter + radius), (yCenter + radius), (zCenter - radius), 1)) >= 0.0f)
        {
            continue;
        }
        if (GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet((xCenter - radius), (yCenter - radius), (zCenter + radius), 1)) >= 0.0f)
        {
            continue;
        }
        if (GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet((xCenter + radius), (yCenter - radius), (zCenter + radius), 1)) >= 0.0f)
        {
            continue;
        }
        if (GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet((xCenter - radius), (yCenter + radius), (zCenter + radius), 1)) >= 0.0f)
        {
            continue;
        }
        if (GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet((xCenter + radius), (yCenter + radius), (zCenter + radius), 1)) >= 0.0f)
        {
            continue;
        }

        return false;
    }

    return true;
}

bool Frustum::CheckSphere(float xCenter, float yCenter, float zCenter, float radius)
{
    // Check if the radius of the sphere is inside the view frustum.
    // 구체의 중심으로 부터 구체의 반경이 절두체 안에 들어오는지 체크함.
    // 만약 절두체의 6개의 모든 평면 안에 들어온다면, 해당 구체를 통과시켜 준다.
    for (int i = 0; i < 6; i++)
    {
        if (GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet(xCenter, yCenter, zCenter, 1)) < -radius)
        {
            return false;
        }
    }

    return true;
}

bool Frustum::CheckRectangle(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize)
{
    // 직육면체의 경우에는 정육면체 체크와 비슷하게 이루어진다.
    // 정육면체는 중심과 반경만으로 체크했다면, 정육면체는 x, y, z 방향의 반경의 크기를 가지고 체크해준다.
    for (int i = 0; i < 6; i++)
    {
        if (GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet((xCenter - xSize), (yCenter - ySize), (zCenter - zSize), 1)) >= 0.0f)
        {
            continue;
        }
        if (GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet((xCenter + xSize), (yCenter - ySize), (zCenter - zSize), 1)) >= 0.0f)
        {
            continue;
        }
        if (GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet((xCenter - xSize), (yCenter + ySize), (zCenter - zSize), 1)) >= 0.0f)
        {
            continue;
        }
        if (GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet((xCenter - xSize), (yCenter - ySize), (zCenter + zSize), 1)) >= 0.0f)
        {
            continue;
        }
        if (GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet((xCenter + xSize), (yCenter + ySize), (zCenter - zSize), 1)) >= 0.0f)
        {
            continue;
        }
        if (GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet((xCenter + xSize), (yCenter - ySize), (zCenter + zSize), 1)) >= 0.0f)
        {
            continue;
        }
        if (GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet((xCenter - xSize), (yCenter + ySize), (zCenter + zSize), 1)) >= 0.0f)
        {
            continue;
        }
        if (GetDotResult(m_frustumPlanes[i], DirectX::XMVectorSet((xCenter + xSize), (yCenter + ySize), (zCenter + zSize), 1)) >= 0.0f)
        {
            continue;
        }

        return false;
    }

    return true;
}
