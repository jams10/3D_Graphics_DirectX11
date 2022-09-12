Texture2D shaderTexture;
SamplerState SampleType;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float fogFactor : FOG;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float4 fogColor;
    float4 finalColor;

    textureColor = shaderTexture.Sample(SampleType, input.tex);
    
    // 안개 색상을 회색으로 설정.
    fogColor = float4(0.5f, 0.5f, 0.5f, 1.0f);

    // 안개 효과 방정식을 이용, 정점 셰이더에서 넘겨준 인자를 통해 최종 픽셀 색상을 결정함.
    finalColor = input.fogFactor * textureColor + (1.0 - input.fogFactor) * fogColor;

    return finalColor;
}
