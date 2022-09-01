Texture2D shaderTextures[3];
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 diffuseColor;
    float4 ambientColor;
    float4 specularColor;
    float specularPower;
    float3 lightDirection;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 viewDirection : TEXCOORD1;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float4 bumpMap;
    float3 bumpNormal;
    float3 lightDir;
    float lightIntensity;
    float4 color;
    float4 specularIntensity;
    float3 reflection;
    float4 specular;
	
    // 디퓨즈맵 픽셀 값 추출.
    textureColor = shaderTextures[0].Sample(SampleType, input.tex);
	
    // 노말맵 픽셀 값 추출.
    bumpMap = shaderTextures[1].Sample(SampleType, input.tex);

    // 노말맵 픽셀의 범위를 노말 벡터의 범위 -1 ~ 1로 만들어줌.
    bumpMap = (bumpMap * 2.0f) - 1.0f;

    // 앞서 계산해준 normal, tangent, binormal 값과 노말맵의 값을 사용해 최종 normal 벡터를 계산.
    bumpNormal = input.normal + bumpMap.x * input.tangent + bumpMap.y * input.binormal;
    bumpNormal = normalize(bumpNormal);

    // 계산을 위해 광원 방향을 뒤집어줌. (표면에서 광원을 향하도록)
    lightDir = -lightDirection;

    // 노말맵 텍스쳐를 통해 계산한 노말 벡터와 광원 방향 벡터로 해당 표면이 받을 빛의 양을 계산.
    lightIntensity = saturate(dot(bumpNormal, lightDir));

    // 빛의 색상과 빛의 양을 곱해 최종 빛 색상을 결정.
    color = saturate(diffuseColor * lightIntensity);
    
    // 빛 색상과 디퓨즈맵 텍스쳐 픽셀 값을 곱해 표면의 색상 값을 결정함.
    color = saturate(color * textureColor);
    
    if (lightIntensity > 0.0f)
    {
        // 스페큘러 맵에서 픽셀 값을 추출.
        specularIntensity = shaderTextures[2].Sample(SampleType, input.tex);

        // 표면의 반사 벡터를 계산함.
        reflection = normalize(2 * lightIntensity * bumpNormal - lightDir);

        // 반사벡터, 카메라 방향 벡터, 정반사광 지수 값을 통해 정반사광의 양을 결정함.
        specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower);

        // 스페큘러 맵의 값을 이용해서 해당 위치 픽셀에 대한 정반사광 값을 계산함.
        specular = specular * specularIntensity;
		
        // 최종 색상 값에 스페큘러 값을 적용해줌.
        color = saturate(color + specular);
    }
	
    return color;
}
