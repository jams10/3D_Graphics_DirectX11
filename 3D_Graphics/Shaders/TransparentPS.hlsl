Texture2D shaderTexture;
SamplerState SampleType;

cbuffer TransparentBuffer
{
    float blendAmount;
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

    // 픽셀의 alpha 값을 blending amount 값으로 설정해 알파 블렌딩 효과를 생성함.
    color.a = blendAmount;

    return color;
}
