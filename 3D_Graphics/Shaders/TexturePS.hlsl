
// ���� ����
Texture2D shaderTexture;
SamplerState SampleType;

// ����� ���� Ÿ��
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;


    // �����⸦ ���� ���� ������ UV ��ǥ�� �ش��ϴ� �κ��� �ȼ� ���� ���÷��� ���� �ؽ��� �ڿ����� ���� ����.
    textureColor = shaderTexture.Sample(SampleType, input.tex);

    return textureColor;
}
