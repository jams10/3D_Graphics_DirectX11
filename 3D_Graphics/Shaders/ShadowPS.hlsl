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

    // �ε� �Ҽ��� ������ �ذ��ϱ� ���� �ణ�� ���� ���� �����.
    bias = 0.001f;

    // �⺻ ������ �ֺ��� �������� �ʱ�ȭ ����.
    color = ambientColor;

    // ���� ���� ���� ��ǥ�� ���� ���ø��� �ؽ��� ��ǥ�� �����.
    projectTexCoord.x = input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
    projectTexCoord.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0f + 0.5f;

    // ������ ��ǥ�� 0 ~ 1 ���� �ȿ� �ִ��� Ȯ����. 0 ~ 1 ���� �ȿ� �ִٸ� ������ �þ� �ȿ� �ִ� ��.
    if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
    {
        // ���� ���� ���� ��ǥ�� ���� ���� ������ ����� �ؽ���(�׸��� ��)�� ���ø� ��.
        depthValue = depthMapTexture.Sample(SampleTypeClamp, projectTexCoord).r;

        // ������ ���� �� ���.(�������� ������Ʈ ������ �Ÿ�.)
        lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w;

        // ���� ���� ���� ���� ���� ���� ����.
        lightDepthValue = lightDepthValue - bias;

        // �ش� �ȼ��� ��������, �׸��ڸ� �帮���� �����ϱ� ���� �׸��� ���� ���� ���� �������� �ش� ������Ʈ ������ ����(�Ÿ�) ���� ����.
        // ����, �׸��� ���� ���� �� ���� �������� ������Ʈ ������ �Ÿ��� �� ª�ٸ�, ���� �����ְ� �ƴϸ� �׸��ڸ� �帮����.
        if (lightDepthValue < depthValue)
        {
            // �ش� �ȼ��� ���� ���� �����.
            lightIntensity = saturate(dot(input.normal, input.lightPos));

            if (lightIntensity > 0.0f)
            {
                // ���ݻ籤 ����� ���� ���⸦ ���� ���� ���ݻ籤 ������ �����.
                color += (diffuseColor * lightIntensity);

				// ���� ���� 0 ~ 1 ������ �������.
                color = saturate(color);
            }
        }
    }

    // ������Ʈ�� ���� �ؽ��ĸ� ���ø��Ͽ� �ؼ� ���� ����.
    textureColor = shaderTexture.Sample(SampleTypeWrap, input.tex);

	// ������ ������� ������ ���� ����� �ؼ� ���� ���� ���� ���� ���� ���.
    color = color * textureColor;

    return color;
}