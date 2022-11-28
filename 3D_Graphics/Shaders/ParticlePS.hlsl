Texture2D shaderTexture;
SamplerState SampleType;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 color : COLOR;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float4 finalColor;
    
    textureColor = shaderTexture.Sample(SampleType, input.tex);

	// 정점 색상과 텍스쳐 색상을 결합해 최종 fragment 색상을 결정.
    finalColor = textureColor * input.color;

    return finalColor;
}