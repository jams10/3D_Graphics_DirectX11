Texture2D shaderTexture;
SamplerState SampleType;

cbuffer FadeBuffer
{
    float fadeAmount;
    float3 padding;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 color;

    color = shaderTexture.Sample(SampleType, input.tex);
    
    // ������ ��⸦ ���� fade �ۼ�Ʈ�� ���ҽ�Ŵ.
    color = color * fadeAmount;

    return color;
}