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
    DirectX::XMStoreFloat4x4(&fmatrix, projMatrix); // xmmatrix�� xmfloat4x4�� ����.

    // near ����� �Ÿ��� ���ڷ� ���� far ����� �Ÿ��� ������ ���� ���� ����� ���� ������Ʈ��.
    near = -fmatrix._43 / fmatrix._33;
    ratio = screenDepth / (screenDepth - near);
    fmatrix._33 = ratio;
    fmatrix._43 = -ratio * near;
    projMatrix = DirectX::XMLoadFloat4x4(&fmatrix); // xmfloat4x4�� xmmatrix�� ����.

    matrix = DirectX::XMMatrixMultiply(viewMatrix, projMatrix);
    DirectX::XMStoreFloat4x4(&fmatrix, matrix);

    // ������� ����ü ��� ���͸� ���.
    m_frustumPlanes[0].x = fmatrix._14 + fmatrix._13;
    m_frustumPlanes[0].y = fmatrix._24 + fmatrix._23;
    m_frustumPlanes[0].z = fmatrix._34 + fmatrix._33;
    m_frustumPlanes[0].w = fmatrix._44 + fmatrix._43;
    
    // ������� ����ü ��� ���͸� ���.
    m_frustumPlanes[1].x = fmatrix._14 - fmatrix._13;
    m_frustumPlanes[1].y = fmatrix._24 - fmatrix._23;
    m_frustumPlanes[1].z = fmatrix._34 - fmatrix._33;
    m_frustumPlanes[1].w = fmatrix._44 - fmatrix._43;

    // ���� ����� ����ü ��� ���͸� ���.
    m_frustumPlanes[2].x = fmatrix._14 + fmatrix._11;
    m_frustumPlanes[2].y = fmatrix._24 + fmatrix._21;
    m_frustumPlanes[2].z = fmatrix._34 + fmatrix._31;
    m_frustumPlanes[2].w = fmatrix._44 + fmatrix._41;

    // ���� ����� ����ü ��� ���͸� ���.
    m_frustumPlanes[3].x = fmatrix._14 - fmatrix._11;
    m_frustumPlanes[3].y = fmatrix._24 - fmatrix._21;
    m_frustumPlanes[3].z = fmatrix._34 - fmatrix._31;
    m_frustumPlanes[3].w = fmatrix._44 - fmatrix._41;

    // ��� ����� ����ü ��� ���͸� ���.
    m_frustumPlanes[4].x = fmatrix._14 - fmatrix._12;
    m_frustumPlanes[4].y = fmatrix._24 - fmatrix._22;
    m_frustumPlanes[4].z = fmatrix._34 - fmatrix._32;
    m_frustumPlanes[4].w = fmatrix._44 - fmatrix._42;

    // �ϴ� ����� ����ü ��� ���͸� ���.
    m_frustumPlanes[5].x = fmatrix._14 + fmatrix._12;
    m_frustumPlanes[5].y = fmatrix._24 + fmatrix._22;
    m_frustumPlanes[5].z = fmatrix._34 + fmatrix._32;
    m_frustumPlanes[5].w = fmatrix._44 + fmatrix._42;

    // ����ȭ�� ����� �������� ��� ���� ����ȭ.
    // �� ��, �ش� ����� �������� ���� ���ʹ� ����ü ������ ���ϰ� ������ ����.
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
    // ���ڷ� �־��� ���� ����ü �ȿ� �ִ��� üũ.
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
    // ���ڷ� �־��� ������ü�� �߽� ��ġ�� �߽����κ����� �ݰ����� ����ü �����ȿ� �������� üũ�Ѵ�.
    // �߽� ��ġ�κ��� 8���� �������� üũ�ؼ� �������� �ϳ��� ����ü �ȿ� ������ ��� �׷��� �� �ֵ��� true�� ������.
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
    // ��ü�� �߽����� ���� ��ü�� �ݰ��� ����ü �ȿ� �������� üũ��.
    // ���� ����ü�� 6���� ��� ��� �ȿ� ���´ٸ�, �ش� ��ü�� ������� �ش�.
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
    // ������ü�� ��쿡�� ������ü üũ�� ����ϰ� �̷������.
    // ������ü�� �߽ɰ� �ݰ游���� üũ�ߴٸ�, ������ü�� x, y, z ������ �ݰ��� ũ�⸦ ������ üũ���ش�.
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
