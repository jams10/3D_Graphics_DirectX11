Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float clip : SV_ClipDistance0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;
	
    textureColor = shaderTexture.Sample(SampleType, input.tex);
	
	// 기본 색상은 주변광 색상으로 설정.
    color = ambientColor;
    
    // 라이팅 계산을 위해 광원에서 물체를 향하는 벡터를 뒤집어줌.
    lightDir = -lightDirection;

    // 빛의 양 계산.
    lightIntensity = saturate(dot(input.normal, lightDir));

    if (lightIntensity > 0.0f)
    {
        // 빛이 조금이라도 비치는 경우 난반사광 색상에 곱해줌.
        color += (diffuseColor * lightIntensity);
    }

    color = saturate(color);

    // 라이팅 결과 색상을 텍스쳐 색상에 곱해줌.
    color = color * textureColor;
	
    return color;
}
