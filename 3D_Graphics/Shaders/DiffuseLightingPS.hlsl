// 전역 변수
Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
    float padding;
};

// 사용자 정의 타입
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;

    // 텍스쳐를 샘플링 하여 텍스쳐 좌표에 해당하는 픽셀의 색상 값을 얻어옴.
    textureColor = shaderTexture.Sample(SampleType, input.tex);
    
    // 픽셀 셰이더의 출력 값이 기본적으로 주변광 값을 지니고 있도록 함.
    color = ambientColor;

    // 물체 표면의 노말 벡터와 방향을 맞춰주기 위해 라이팅 벡터의 방향을 뒤집어줌.
    lightDir = -lightDirection;

    // 노말 벡터와 라이팅 벡터 간 내적을 통해 빛의 세기를 결정.
    lightIntensity = saturate(dot(input.normal, lightDir));
    
    // 광원에 의해 빛을 받는 부분에는 광원의 색상에 내적에 따른 빛의 세기를 곱해 주변광에 더해줌.
    if(lightIntensity > 0.0f)
    {
        color += (diffuseColor * lightIntensity);
    }

    // 계산한 결과를 saturate를 통해 0 ~ 1 범위로 clamp.
    color = saturate(color);

    // 라이팅 색상과 텍스쳐 색상을 곱해 최종 픽셀의 색상을 결정함.
    color = color * textureColor;

    return color;
}
