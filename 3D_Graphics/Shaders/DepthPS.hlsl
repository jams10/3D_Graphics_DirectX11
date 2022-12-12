struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXTURE0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float depthValue;
    float4 color;
	
    // z���� ���� ��ǥ�� w�� ������ �ȼ��� ���� ���� ����.
    depthValue = input.depthPosition.z / input.depthPosition.w;

    //// ���� ������ ù 10%�� ���������� ĥ��.
    //if (depthValue < 0.9f)
    //{
    //    color = float4(1.0, 0.0f, 0.0f, 1.0f);
    //}
	
    //// ���� 0.025%�� �ʷϻ����� ĥ��.
    //if (depthValue > 0.9f)
    //{
    //    color = float4(0.0, 1.0f, 0.0f, 1.0f);
    //}
    
    //// ������ �κе��� �Ķ������� ĥ����.
    //if (depthValue > 0.925f)
    //{
    //    color = float4(0.0, 0.0f, 1.0f, 1.0f);
    //}
    
    color = float4(depthValue, depthValue, depthValue, 1.0f);

    return color;
}
