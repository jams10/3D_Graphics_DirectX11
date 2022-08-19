#include "ModelList.h"
#include <random>
#include <ErrorHandle/StandardException.h>

#define RANDOM_MAX 32767

ModelList::ModelList()
{
}

void ModelList::Initialize(int nModels)
{
    float red, green, blue;

    // 모델의 개수를 저장.
    m_modelCount = nModels;

    // 모델의 정보를 담는 배열을 할당해줌.
    m_pModelInfoList = new ModelInfoType[m_modelCount];
    ALLOCATE_EXCEPT(m_pModelInfoList, "Can't allocate modelInfoList array")

    // 난수를 생성해줌.
    std::random_device rd;
    std::mt19937_64 rng(rd()); // 의사 난수 생성 엔진 시드로 random_device를 넘겨 초기화.

    std::uniform_int_distribution<__int64> dist(0, 32767);

    for (int i = 0; i < m_modelCount; ++i)
    {
        // 랜덤 색상.
        red = (float)dist(rng) / RANDOM_MAX;
        green = (float)dist(rng) / RANDOM_MAX;
        blue = (float)dist(rng) / RANDOM_MAX;
        m_pModelInfoList[i].color = DirectX::XMFLOAT4(red, green, blue, 1.0f);
        // 랜덤 위치.
        m_pModelInfoList[i].positionX = (((float)dist(rng) - (float)dist(rng)) / RANDOM_MAX) * 10.f;
        m_pModelInfoList[i].positionY = (((float)dist(rng) - (float)dist(rng)) / RANDOM_MAX) * 10.f;
        m_pModelInfoList[i].positionZ = (((float)dist(rng) - (float)dist(rng)) / RANDOM_MAX) * 10.f + 5.0f;
    }
}

void ModelList::ShutDown()
{
    if (m_pModelInfoList)
    {
        delete[]m_pModelInfoList;
        m_pModelInfoList = nullptr;
    }
    return;
}

int ModelList::GetModelCount()
{
    return m_modelCount;
}

void ModelList::GetData(int index, float& positionX, float& positionY, float& positionZ, DirectX::XMFLOAT4& color)
{
    color = m_pModelInfoList[index].color;

    positionX = m_pModelInfoList[index].positionX;
    positionY = m_pModelInfoList[index].positionY;
    positionZ = m_pModelInfoList[index].positionZ;
}
