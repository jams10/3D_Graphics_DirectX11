Texture2D shaderTextures[2];
SamplerState SampleType;

cbuffer GammaBuffer
{
    float gamma;
    float3 padding;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 color1;
    float4 color2;
    float4 blendColor;

    // ù��° �ؽ��ķκ��� �ȼ� ������ ����.
    color1 = shaderTextures[0].Sample(SampleType, input.tex);

    // �ι�° �ؽ��ķκ��� �ȼ� ������ ����.
    color2 = shaderTextures[1].Sample(SampleType, input.tex);

    // �� �ȼ����� ���� ���� ���� ȥ������.
    blendColor = color1 * color2 * gamma;
    
    // ���� ���� ���� 0 ~ 1 ������ clamp.
    blendColor = saturate(blendColor);

    return blendColor;
}