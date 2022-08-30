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

    // ���� �Է����� ���� �ȼ��� ��ġ�� �����ϴ� �ؽ��� �ȼ� ���� ����.
    textureColor = shaderTextures[0].Sample(SampleType, input.tex);
	
    // �븻�� �ؽ��Ŀ� �ִ� �ȼ� ���� ����.
    bumpMap = shaderTextures[1].Sample(SampleType, input.tex);

    // �ؽ��� ���� ���� 0 ~ 1 ������ ������ ������ �븻 ���� ���� -1 ~ 1�� �ٲ���.
    bumpMap = (bumpMap * 2.0f) - 1.0f;
    
    // �븻�ʿ� �ִ� ���� ����� �븻 ���� �����.
    bumpNormal = (bumpMap.x * input.tangent) + (bumpMap.y * input.binormal) + (bumpMap.z * input.normal);
	
    // ����� �븻 ���� ����ȭ.
    bumpNormal = normalize(bumpNormal);

    // �븻 ���� ����� ����(ǥ������ ���� ������ ����Ŵ)�� ����Ű���� ������ ������ ��������.
    lightDir = -lightDirection;

    // ����� �븻 ���� ���� �ش� ��ġ�� �ȼ��� �޴� ������ ���� �����.
    lightIntensity = saturate(dot(bumpNormal, lightDir));

    // ������ ��� ������ ������ ������.
    color = saturate(diffuseColor * lightIntensity);

    // �ؽ��� ����� ���� ������ ���� ���� �ȼ� ������ ������.
    color = color * textureColor;
	
    return color;
}
