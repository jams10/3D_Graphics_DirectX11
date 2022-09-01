Texture2D shaderTextures[3];
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 diffuseColor;
    float4 ambientColor;
    float4 specularColor;
    float specularPower;
    float3 lightDirection;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 viewDirection : TEXCOORD1;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float4 bumpMap;
    float3 bumpNormal;
    float3 lightDir;
    float lightIntensity;
    float4 color;
    float4 specularIntensity;
    float3 reflection;
    float4 specular;
	
    // ��ǻ��� �ȼ� �� ����.
    textureColor = shaderTextures[0].Sample(SampleType, input.tex);
	
    // �븻�� �ȼ� �� ����.
    bumpMap = shaderTextures[1].Sample(SampleType, input.tex);

    // �븻�� �ȼ��� ������ �븻 ������ ���� -1 ~ 1�� �������.
    bumpMap = (bumpMap * 2.0f) - 1.0f;

    // �ռ� ������� normal, tangent, binormal ���� �븻���� ���� ����� ���� normal ���͸� ���.
    bumpNormal = input.normal + bumpMap.x * input.tangent + bumpMap.y * input.binormal;
    bumpNormal = normalize(bumpNormal);

    // ����� ���� ���� ������ ��������. (ǥ�鿡�� ������ ���ϵ���)
    lightDir = -lightDirection;

    // �븻�� �ؽ��ĸ� ���� ����� �븻 ���Ϳ� ���� ���� ���ͷ� �ش� ǥ���� ���� ���� ���� ���.
    lightIntensity = saturate(dot(bumpNormal, lightDir));

    // ���� ����� ���� ���� ���� ���� �� ������ ����.
    color = saturate(diffuseColor * lightIntensity);
    
    // �� ����� ��ǻ��� �ؽ��� �ȼ� ���� ���� ǥ���� ���� ���� ������.
    color = saturate(color * textureColor);
    
    if (lightIntensity > 0.0f)
    {
        // ����ŧ�� �ʿ��� �ȼ� ���� ����.
        specularIntensity = shaderTextures[2].Sample(SampleType, input.tex);

        // ǥ���� �ݻ� ���͸� �����.
        reflection = normalize(2 * lightIntensity * bumpNormal - lightDir);

        // �ݻ纤��, ī�޶� ���� ����, ���ݻ籤 ���� ���� ���� ���ݻ籤�� ���� ������.
        specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower);

        // ����ŧ�� ���� ���� �̿��ؼ� �ش� ��ġ �ȼ��� ���� ���ݻ籤 ���� �����.
        specular = specular * specularIntensity;
		
        // ���� ���� ���� ����ŧ�� ���� ��������.
        color = saturate(color + specular);
    }
	
    return color;
}
