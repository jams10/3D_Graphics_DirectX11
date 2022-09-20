Texture2D shaderTexture;
SamplerState SampleType;

cbuffer TranslationBuffer
{
    float textureTranslationX;
    float textureTranslationY;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    // 텍스쳐 좌표를 이동시킴.
    input.tex.x += textureTranslationX;
    input.tex.y += textureTranslationY;

    return shaderTexture.Sample(SampleType, input.tex);
}