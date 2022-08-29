Texture2D shaderTextures[3];
SamplerState SampleType;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 color1;
    float4 color2;
    float4 alphaValue;
    float4 blendColor;

    // 첫번째 텍스쳐에서 픽셀 색상을 얻어옴.
    color1 = shaderTextures[0].Sample(SampleType, input.tex);

    // 두번째 텍스쳐에서 픽셀 색상을 얻어옴.
    color2 = shaderTextures[1].Sample(SampleType, input.tex);

    // 알파맵 텍스쳐에서 알파 값을 얻어옴.
    alphaValue = shaderTextures[2].Sample(SampleType, input.tex);
	
    // 얻어온 알파 값을 이용해서 두 텍스쳐를 결합해줌.
    blendColor = (alphaValue * color1) + ((1.0 - alphaValue) * color2);
    
    // 최종 색상 값을 0 ~ 1 범위로 만들어줌.
    blendColor = saturate(blendColor);

    return blendColor;
}
