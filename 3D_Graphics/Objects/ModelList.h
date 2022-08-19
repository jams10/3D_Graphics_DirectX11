#pragma once

#include <DirectXMath.h>

class ModelList
{
public:
	ModelList();

	void Initialize(int nModels);
	void ShutDown();

	int GetModelCount();
	void GetData(int index, float& positionX, float& positionY, float& positionZ, DirectX::XMFLOAT4& color);

private:
	struct ModelInfoType
	{
		DirectX::XMFLOAT4 color;
		float positionX, positionY, positionZ;
	};

	int m_modelCount;
	ModelInfoType* m_pModelInfoList;
};
