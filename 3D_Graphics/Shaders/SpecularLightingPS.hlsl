// ���� ����
Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
    float  specularPower; // ���ݻ籤 ��꿡 ����� �ŵ����� ���� ��.
    float4 specularColor; // ���ݻ籤 ����.
};

// ����� ���� Ÿ��
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 viewDirection : TEXCOORD1; // ���� ���̴����� �Ѱ��ִ� ��ü�� �������� ī�޶� ���ϴ� ����.
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;
    float3 reflection; // �ݻ籤 ����
    float4 specular;   // ���ݻ籤 ��

    // �ؽ��ĸ� ���ø� �Ͽ� �ؽ��� ��ǥ�� �ش��ϴ� �ȼ��� ���� ���� ����.
    textureColor = shaderTexture.Sample(SampleType, input.tex);
    
    // �ȼ� ���̴��� ��� ���� �⺻������ �ֺ��� ���� ���ϰ� �ֵ��� ��.
    color = ambientColor;

    // ��ü ǥ���� �븻 ���Ϳ� ������ �����ֱ� ���� ������ ������ ������ ��������.
    lightDir = -lightDirection;
    
    // ���ݻ籤 ���� �ʱ�ȭ
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // �븻 ���Ϳ� ������ ���� �� ������ ���� ���� ���⸦ ����.
    lightIntensity = saturate(dot(input.normal, lightDir));
    
    // ������ ���� ���� �޴� �κп��� ������ ���� ������ ���� ���� ���⸦ ���� �ֺ����� ������. ���⿡ �߰������� ���ݻ籤 ���� �����.
    if (lightIntensity > 0.0f)
    {
        // ���ݻ籤 ���� ���.
        color += (diffuseColor * lightIntensity);
        
        // ����� ���ݻ籤 ���� saturate�� ���� 0 ~ 1 ������ clamp.
        color = saturate(color);
        
        // 2(L��N)N-L ���Ŀ� ���� �ݻ籤 ���� ��� �� ����ȭ ����. N : ��ü �븻 ���� / L : ���� ����
        reflection = normalize(2 * lightIntensity * input.normal - lightDir);

        // ���� ���ݻ籤 ���� �������. (R��V)^(specular power)
        specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower);
    }

    // ���ݻ籤 ���� �ؽ��� ���� ���� ���� ���ݻ籤�� ���� ǥ���Ǵ� ������ �⺻ ������ ����.
    color = color * textureColor;
    
    // ���ݻ籤�� ���ݻ籤 ���� ���� ���� �ȼ� ������ ����.
    color = saturate(color + specular);

    return color;
}
