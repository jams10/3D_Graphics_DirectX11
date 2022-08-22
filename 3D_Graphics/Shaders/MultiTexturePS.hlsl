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

    // 첫번째 텍스쳐로부터 픽셀 색상을 얻어옴.
    color1 = shaderTextures[0].Sample(SampleType, input.tex);

    // 두번째 텍스쳐로부터 픽셀 색상을 얻어옴.
    color2 = shaderTextures[1].Sample(SampleType, input.tex);

    // 두 픽셀들을 감마 값을 곱해 혼합해줌.
    blendColor = color1 * color2 * gamma;
    
    // 최종 색상 값을 0 ~ 1 범위로 clamp.
    blendColor = saturate(blendColor);

    return blendColor;
}