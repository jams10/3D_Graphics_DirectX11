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
    
    // reflectionPosition�� ���� �ؽ��� ��ǥ��� �ٲ���. (0 ~ 1 ����)
    reflectTexCoord.x = input.reflectionPosition.x / input.reflectionPosition.w / 2.0f + 0.5f;
    reflectTexCoord.y = -input.reflectionPosition.y / input.reflectionPosition.w / 2.0f + 0.5f;

    // �ݻ� �ؽ��� �ȼ� ���ø�.
    reflectionColor = reflectionTexture.Sample(SampleType, reflectTexCoord);

    // �� �ؽ��İ� ���� ������ ���� �ؽ��� ȥ��.
    color = lerp(textureColor, reflectionColor, 0.15f);

    return color;
}