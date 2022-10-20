SamplerState SampleType;
Texture2D colorTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D refractionTexture : register(t2);

cbuffer GlassBuffer
{
    float refractionScale;
    float3 padding;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 refractionPosition : TEXCOORD1;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float2 refractTexCoord;
    float4 normalMap;
    float3 normal;
    float4 refractionColor;
    float4 textureColor;
    float4 color;

    // 투영된 굴절 텍스쳐 좌표들을 계산해줌. (0 ~ 1 범위의 텍스쳐 좌표를 갖도록 함.)
    refractTexCoord.x = input.refractionPosition.x / input.refractionPosition.w / 2.0f + 0.5f;
    refractTexCoord.y = -input.refractionPosition.y / input.refractionPosition.w / 2.0f + 0.5f;

    // 노말맵 텍스쳐에서 노말 값을 샘플링.
    normalMap = normalTexture.Sample(SampleType, input.tex);

    // 노말 값 범위를 [0,1]에서 [-1,1]로 확장.
    normal = (normalMap.xyz * 2.0f) - 1.0f;

    // 노말 맵 값에 따라 텍스쳐 샘플링 좌표를 재배치하여 유리를 투과하여 굴절되는 빛을 시뮬레이션 할 수 있도록 함. (오프셋 값을 줌.)
    refractTexCoord = refractTexCoord + (normal.xy * refractionScale);

    // 오프셋 값이 적용된 굴절 텍스쳐 좌표를 이용, 굴절 텍스쳐를 샘플링 함.
    refractionColor = refractionTexture.Sample(SampleType, refractTexCoord);

    // 유리 색상 텍스쳐를 샘플링 함.
    textureColor = colorTexture.Sample(SampleType, input.tex);
	
    // 유리 색상과 굴절 값을 고르게 결합해 최종 픽셀 색상을 결정함.
    color = lerp(refractionColor, textureColor, 0.5f);

    return color;
}
