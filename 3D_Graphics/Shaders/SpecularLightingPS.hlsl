// 전역 변수
Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
    float  specularPower; // 정반사광 계산에 사용할 거듭제곱 지수 값.
    float4 specularColor; // 정반사광 색상.
};

// 사용자 정의 타입
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 viewDirection : TEXCOORD1; // 정점 셰이더에서 넘겨주는 물체의 정점에서 카메라를 향하는 벡터.
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;
    float3 reflection; // 반사광 벡터
    float4 specular;   // 정반사광 값

    // 텍스쳐를 샘플링 하여 텍스쳐 좌표에 해당하는 픽셀의 색상 값을 얻어옴.
    textureColor = shaderTexture.Sample(SampleType, input.tex);
    
    // 픽셀 셰이더의 출력 값이 기본적으로 주변광 값을 지니고 있도록 함.
    color = ambientColor;

    // 물체 표면의 노말 벡터와 방향을 맞춰주기 위해 라이팅 벡터의 방향을 뒤집어줌.
    lightDir = -lightDirection;
    
    // 정반사광 값을 초기화
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // 노말 벡터와 라이팅 벡터 간 내적을 통해 빛의 세기를 결정.
    lightIntensity = saturate(dot(input.normal, lightDir));
    
    // 광원에 의해 빛을 받는 부분에는 광원의 색상에 내적에 따른 빛의 세기를 곱해 주변광에 더해줌. 여기에 추가적으로 난반사광 값도 계산함.
    if (lightIntensity > 0.0f)
    {
        // 난반사광 값을 계산.
        color += (diffuseColor * lightIntensity);
        
        // 계산한 난반사광 값을 saturate를 통해 0 ~ 1 범위로 clamp.
        color = saturate(color);
        
        // 2(LㆍN)N-L 공식에 따라 반사광 값을 계산 후 정규화 해줌. N : 물체 노말 벡터 / L : 광원 벡터
        reflection = normalize(2 * lightIntensity * input.normal - lightDir);

        // 최종 정반사광 값을 계산해줌. (RㆍV)^(specular power)
        specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower);
    }

    // 난반사광 값과 텍스쳐 색상 값을 곱해 난반사광에 의해 표현되는 도형의 기본 색상을 결정.
    color = color * textureColor;
    
    // 난반사광에 정반사광 값을 더해 최종 픽셀 색상을 결정.
    color = saturate(color + specular);

    return color;
}
