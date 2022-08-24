Texture2D shaderTextures[2];
SamplerState SampleType;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 color;
    float4 lightmapColor;
    float4 finalColor;

    // ù��° �ؽ��ķκ��� �ȼ� ������ ����.
    color = shaderTextures[0].Sample(SampleType, input.tex);

    // �ι�° �ؽ��ķκ��� �ȼ� ������ ����.
    lightmapColor = shaderTextures[1].Sample(SampleType, input.tex);

    // �� �ؽ��� ������ ȥ������.
    finalColor = color * lightmapColor;
    
    // ���� ���� ���� 0 ~ 1 ������ clamp.
    finalColor = saturate(finalColor);

    return finalColor;
}