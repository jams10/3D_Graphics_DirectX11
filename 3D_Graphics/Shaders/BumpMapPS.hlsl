Texture2D shaderTextures[2];
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 diffuseColor;
    float3 lightDirection;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float4 bumpMap;
    float3 bumpNormal;
    float3 lightDir;
    float lightIntensity;
    float4 color;

    // 현재 입력으로 들어온 픽셀의 위치에 대응하는 텍스쳐 픽셀 값을 얻어옴.
    textureColor = shaderTextures[0].Sample(SampleType, input.tex);
	
    // 노말맵 텍스쳐에 있는 픽셀 값을 얻어옴.
    bumpMap = shaderTextures[1].Sample(SampleType, input.tex);

    // 텍스쳐 상의 값은 0 ~ 1 범위를 가지기 때문에 노말 벡터 범위 -1 ~ 1로 바꿔줌.
    bumpMap = (bumpMap * 2.0f) - 1.0f;
    
    // 노말맵에 있는 값을 사용해 노말 값을 계산함.
    bumpNormal = (bumpMap.x * input.tangent) + (bumpMap.y * input.binormal) + (bumpMap.z * input.normal);
	
    // 계산한 노말 값을 정규화.
    bumpNormal = normalize(bumpNormal);

    // 노말 값과 비슷한 방향(표면으로 부터 광원을 가리킴)을 가리키도록 광원의 방향을 뒤집어줌.
    lightDir = -lightDirection;

    // 계산한 노말 값에 따라 해당 위치의 픽셀이 받는 광원의 양을 계산함.
    lightIntensity = saturate(dot(bumpNormal, lightDir));

    // 광원의 양과 광원의 색상을 곱해줌.
    color = saturate(diffuseColor * lightIntensity);

    // 텍스쳐 색상과 광원 색상을 곱해 최종 픽셀 색상을 결정함.
    color = color * textureColor;
	
    return color;
}
