// ���� ����
Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
    float padding;
};

// ����� ���� Ÿ��
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;

    // �ؽ��ĸ� ���ø� �Ͽ� �ؽ��� ��ǥ�� �ش��ϴ� �ȼ��� ���� ���� ����.
    textureColor = shaderTexture.Sample(SampleType, input.tex);
    
    // �ȼ� ���̴��� ��� ���� �⺻������ �ֺ��� ���� ���ϰ� �ֵ��� ��.
    color = ambientColor;

    // ��ü ǥ���� �븻 ���Ϳ� ������ �����ֱ� ���� ������ ������ ������ ��������.
    lightDir = -lightDirection;

    // �븻 ���Ϳ� ������ ���� �� ������ ���� ���� ���⸦ ����.
    lightIntensity = saturate(dot(input.normal, lightDir));
    
    // ������ ���� ���� �޴� �κп��� ������ ���� ������ ���� ���� ���⸦ ���� �ֺ����� ������.
    if(lightIntensity > 0.0f)
    {
        color += (diffuseColor * lightIntensity);
    }

    // ����� ����� saturate�� ���� 0 ~ 1 ������ clamp.
    color = saturate(color);

    // ������ ����� �ؽ��� ������ ���� ���� �ȼ��� ������ ������.
    color = color * textureColor;

    return color;
}
