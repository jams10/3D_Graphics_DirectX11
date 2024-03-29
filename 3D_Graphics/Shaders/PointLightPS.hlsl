#define NUM_LIGHTS 4

Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightColorBuffer
{
    float4 diffuseColor[NUM_LIGHTS];
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 lightPos1 : TEXCOORD1;
    float3 lightPos2 : TEXCOORD2;
    float3 lightPos3 : TEXCOORD3;
    float3 lightPos4 : TEXCOORD4;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float lightIntensity1, lightIntensity2, lightIntensity3, lightIntensity4;
    float4 color, color1, color2, color3, color4;
	
    // 정점 셰이더로 부터 입력으로 들어온 광원 방향과 정점의 노말 벡터의 내적을 통해 빛의 세기를 계산.
    lightIntensity1 = saturate(dot(input.normal, input.lightPos1));
    lightIntensity2 = saturate(dot(input.normal, input.lightPos2));
    lightIntensity3 = saturate(dot(input.normal, input.lightPos3));
    lightIntensity4 = saturate(dot(input.normal, input.lightPos4));
	
    // 빛의 세기에 빛의 색상 값을 곱해줌.
    color1 = diffuseColor[0] * lightIntensity1;
    color2 = diffuseColor[1] * lightIntensity2;
    color3 = diffuseColor[2] * lightIntensity3;
    color4 = diffuseColor[3] * lightIntensity4;

    textureColor = shaderTexture.Sample(SampleType, input.tex);

    // 최종 색상은 모든 빛의 색상에 물체의 텍스쳐 샘플에서 샘플링해온 텍스쳐 색상 값을 곱해줌.
    color = (color1 + color2 + color3 + color4) * textureColor;
	
    return color;
}
