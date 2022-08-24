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

    // 첫번째 텍스쳐로부터 픽셀 색상을 얻어옴.
    color = shaderTextures[0].Sample(SampleType, input.tex);

    // 두번째 텍스쳐로부터 픽셀 색상을 얻어옴.
    lightmapColor = shaderTextures[1].Sample(SampleType, input.tex);

    // 두 텍스쳐 값들을 혼합해줌.
    finalColor = color * lightmapColor;
    
    // 최종 색상 값을 0 ~ 1 범위로 clamp.
    finalColor = saturate(finalColor);

    return finalColor;
}