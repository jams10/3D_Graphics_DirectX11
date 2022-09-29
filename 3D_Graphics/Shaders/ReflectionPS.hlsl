Texture2D shaderTexture;
SamplerState SampleType;
Texture2D reflectionTexture;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 reflectionPosition : TEXCOORD1;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float2 reflectTexCoord;
    float4 reflectionColor;
    float4 color;

    textureColor = shaderTexture.Sample(SampleType, input.tex);
    
    // reflectionPosition을 통해 텍스쳐 좌표계로 바꿔줌. (0 ~ 1 범위)
    reflectTexCoord.x = input.reflectionPosition.x / input.reflectionPosition.w / 2.0f + 0.5f;
    reflectTexCoord.y = -input.reflectionPosition.y / input.reflectionPosition.w / 2.0f + 0.5f;

    // 반사 텍스쳐 픽셀 샘플링.
    reflectionColor = reflectionTexture.Sample(SampleType, reflectTexCoord);

    // 두 텍스쳐간 선형 보간을 통해 텍스쳐 혼합.
    color = lerp(textureColor, reflectionColor, 0.15f);

    return color;
}