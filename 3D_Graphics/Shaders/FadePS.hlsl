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
    
    // 색상의 밝기를 현재 fade 퍼센트로 감소시킴.
    color = color * fadeAmount;

    return color;
}