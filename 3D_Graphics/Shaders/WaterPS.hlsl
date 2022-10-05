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

    // ������ �帣�� ȿ���� �ֱ� ���� �븻 �� �ؽ����� v��ǥ�� �̵���Ŵ.
    input.tex.y += waterTranslation;
	
    // �ݻ� ��ǥ�� -1 ~ 1 ������ �������.
    reflectTexCoord.x = input.reflectionPosition.x / input.reflectionPosition.w / 2.0f + 0.5f;
    reflectTexCoord.y = -input.reflectionPosition.y / input.reflectionPosition.w / 2.0f + 0.5f;
	
    // ���� ��ǥ�� -1 ~ 1 ������ �������.
    refractTexCoord.x = input.refractionPosition.x / input.refractionPosition.w / 2.0f + 0.5f;
    refractTexCoord.y = -input.refractionPosition.y / input.refractionPosition.w / 2.0f + 0.5f;

    // �븻�� �ؽ��� ���ø�.
    normalMap = normalTexture.Sample(SampleType, input.tex);

    // 0 ~ 1 ������ �븻�� �ؽ��� ��ǥ ������ -1 ~ 1 ������ �������.
    normal = (normalMap.xyz * 2.0f) - 1.0f;

    // ���� ��� ȿ���� �����ϱ� ���� �븻 �� ������ �ؽ��� ��ǥ ���ø� ��ġ�� ���ġ��.
    reflectTexCoord = reflectTexCoord + (normal.xy * reflectRefractScale);
    refractTexCoord = refractTexCoord + (normal.xy * reflectRefractScale);

    // ���ŵ� �ؽ��� ��ǥ�� ����� �ؽ��ĸ� ���ø�.
    reflectionColor = reflectionTexture.Sample(SampleType, reflectTexCoord);
    refractionColor = refractionTexture.Sample(SampleType, refractTexCoord);

    // ���� ������ �ݻ�� ���� ���ø� ����� ��������.
    color = lerp(reflectionColor, refractionColor, 0.6f);
	
    return color;
}
