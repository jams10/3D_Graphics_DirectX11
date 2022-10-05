SamplerState SampleType;
Texture2D reflectionTexture;
Texture2D refractionTexture;
Texture2D normalTexture;

cbuffer WaterBuffer
{
    float waterTranslation;
    float reflectRefractScale;
    float2 padding;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 reflectionPosition : TEXCOORD1;
    float4 refractionPosition : TEXCOORD2;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float2 reflectTexCoord;
    float2 refractTexCoord;
    float4 normalMap;
    float3 normal;
    float4 reflectionColor;
    float4 refractionColor;
    float4 color;

    // 물결이 흐르는 효과를 주기 위해 노말 맵 텍스쳐의 v좌표를 이동시킴.
    input.tex.y += waterTranslation;
	
    // 반사 좌표를 -1 ~ 1 범위로 만들어줌.
    reflectTexCoord.x = input.reflectionPosition.x / input.reflectionPosition.w / 2.0f + 0.5f;
    reflectTexCoord.y = -input.reflectionPosition.y / input.reflectionPosition.w / 2.0f + 0.5f;
	
    // 굴절 좌표를 -1 ~ 1 범위로 만들어줌.
    refractTexCoord.x = input.refractionPosition.x / input.refractionPosition.w / 2.0f + 0.5f;
    refractTexCoord.y = -input.refractionPosition.y / input.refractionPosition.w / 2.0f + 0.5f;

    // 노말맵 텍스쳐 샘플링.
    normalMap = normalTexture.Sample(SampleType, input.tex);

    // 0 ~ 1 범위의 노말맵 텍스쳐 좌표 범위를 -1 ~ 1 범위로 만들어줌.
    normal = (normalMap.xyz * 2.0f) - 1.0f;

    // 물결 모양 효과를 구현하기 위해 노말 맵 값으로 텍스쳐 좌표 샘플링 위치를 재배치함.
    reflectTexCoord = reflectTexCoord + (normal.xy * reflectRefractScale);
    refractTexCoord = refractTexCoord + (normal.xy * reflectRefractScale);

    // 갱신된 텍스쳐 좌표를 사용해 텍스쳐를 샘플링.
    reflectionColor = reflectionTexture.Sample(SampleType, reflectTexCoord);
    refractionColor = refractionTexture.Sample(SampleType, refractTexCoord);

    // 최종 색상은 반사와 굴절 샘플링 결과를 결합해줌.
    color = lerp(reflectionColor, refractionColor, 0.6f);
	
    return color;
}
