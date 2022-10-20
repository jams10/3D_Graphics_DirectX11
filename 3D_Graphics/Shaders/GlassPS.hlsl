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

    // ������ ���� �ؽ��� ��ǥ���� �������. (0 ~ 1 ������ �ؽ��� ��ǥ�� ������ ��.)
    refractTexCoord.x = input.refractionPosition.x / input.refractionPosition.w / 2.0f + 0.5f;
    refractTexCoord.y = -input.refractionPosition.y / input.refractionPosition.w / 2.0f + 0.5f;

    // �븻�� �ؽ��Ŀ��� �븻 ���� ���ø�.
    normalMap = normalTexture.Sample(SampleType, input.tex);

    // �븻 �� ������ [0,1]���� [-1,1]�� Ȯ��.
    normal = (normalMap.xyz * 2.0f) - 1.0f;

    // �븻 �� ���� ���� �ؽ��� ���ø� ��ǥ�� ���ġ�Ͽ� ������ �����Ͽ� �����Ǵ� ���� �ùķ��̼� �� �� �ֵ��� ��. (������ ���� ��.)
    refractTexCoord = refractTexCoord + (normal.xy * refractionScale);

    // ������ ���� ����� ���� �ؽ��� ��ǥ�� �̿�, ���� �ؽ��ĸ� ���ø� ��.
    refractionColor = refractionTexture.Sample(SampleType, refractTexCoord);

    // ���� ���� �ؽ��ĸ� ���ø� ��.
    textureColor = colorTexture.Sample(SampleType, input.tex);
	
    // ���� ����� ���� ���� ���� ������ ���� �ȼ� ������ ������.
    color = lerp(refractionColor, textureColor, 0.5f);

    return color;
}
