Texture2D shaderTextures[3];
SamplerState SampleType;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 color1;
    float4 color2;
    float4 alphaValue;
    float4 blendColor;

    // ù��° �ؽ��Ŀ��� �ȼ� ������ ����.
    color1 = shaderTextures[0].Sample(SampleType, input.tex);

    // �ι�° �ؽ��Ŀ��� �ȼ� ������ ����.
    color2 = shaderTextures[1].Sample(SampleType, input.tex);

    // ���ĸ� �ؽ��Ŀ��� ���� ���� ����.
    alphaValue = shaderTextures[2].Sample(SampleType, input.tex);
	
    // ���� ���� ���� �̿��ؼ� �� �ؽ��ĸ� ��������.
    blendColor = (alphaValue * color1) + ((1.0 - alphaValue) * color2);
    
    // ���� ���� ���� 0 ~ 1 ������ �������.
    blendColor = saturate(blendColor);

    return blendColor;
}
