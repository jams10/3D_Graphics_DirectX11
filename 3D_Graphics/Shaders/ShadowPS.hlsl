Texture2D shaderTexture : register(t0);
Texture2D depthMapTexture : register(t1);

SamplerState SampleTypeClamp : register(s0);
SamplerState SampleTypeWrap : register(s1);

cbuffer LightBuffer
{
    float4 ambientColor;
    float4 diffuseColor;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 lightViewPosition : TEXCOORD1;
    float3 lightPos : TEXCOORD2;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float bias;
    float4 color;
    float2 projectTexCoord;
    float depthValue;
    float lightDepthValue;
    float lightIntensity;
    float4 textureColor;

    // 부동 소수점 오차를 해결하기 위해 약간의 편차 값을 사용함.
    bias = 0.001f;

    // 기본 색상을 주변광 색상으로 초기화 해줌.
    color = ambientColor;

    // 광원 시점 투영 좌표를 통해 샘플링할 텍스쳐 좌표를 계산함.
    projectTexCoord.x = input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
    projectTexCoord.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0f + 0.5f;

    // 투영된 좌표가 0 ~ 1 범위 안에 있는지 확인함. 0 ~ 1 범위 안에 있다면 광원의 시야 안에 있는 것.
    if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
    {
        // 광원 시점 투영 좌표를 통해 깊이 정보가 저장된 텍스쳐(그림자 맵)를 샘플링 함.
        depthValue = depthMapTexture.Sample(SampleTypeClamp, projectTexCoord).r;

        // 광원의 깊이 값 계산.(광원에서 오브젝트 까지의 거리.)
        lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w;

        // 구한 광원 깊이 값에 편차 값을 빼줌.
        lightDepthValue = lightDepthValue - bias;

        // 해당 픽셀을 비춰줄지, 그림자를 드리울지 결정하기 위해 그림자 맵의 깊이 값과 광원에서 해당 오브젝트 까지의 깊이(거리) 값을 비교함.
        // 만약, 그림자 맵의 깊이 값 보다 광원에서 오브젝트 까지의 거리가 더 짧다면, 빛을 비춰주고 아니면 그림자를 드리워줌.
        if (lightDepthValue < depthValue)
        {
            // 해당 픽셀의 빛의 양을 계산함.
            lightIntensity = saturate(dot(input.normal, input.lightPos));

            if (lightIntensity > 0.0f)
            {
                // 난반사광 색상과 빛의 세기를 통해 최종 난반사광 색상을 계산함.
                color += (diffuseColor * lightIntensity);

				// 색상 값을 0 ~ 1 범위로 만들어줌.
                color = saturate(color);
            }
        }
    }

    // 오브젝트에 입힐 텍스쳐를 샘플링하여 텍셀 값을 얻어옴.
    textureColor = shaderTexture.Sample(SampleTypeWrap, input.tex);

	// 위에서 계산해준 광원에 의한 색상과 텍셀 값을 곱해 최종 색상 값을 계산.
    color = color * textureColor;

    return color;
}