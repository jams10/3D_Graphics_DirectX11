
// 전역 변수
Texture2D shaderTexture;
SamplerState SampleType;

// 사용자 정의 타입
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;


    // 보간기를 거쳐 들어온 정점의 UV 좌표에 해당하는 부분의 픽셀 값을 샘플러를 통해 텍스쳐 자원으로 부터 얻어옴.
    textureColor = shaderTexture.Sample(SampleType, input.tex);

    return textureColor;
}
