cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
};

cbuffer LightBuffer2
{
    float3 lightPosition;
    float padding;
};

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 lightViewPosition : TEXCOORD1;
    float3 lightPos : TEXCOORD2;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    float4 worldPosition;
    
    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // ������ �������� �� ��ü�� ��ġ�� �������.
    output.lightViewPosition = mul(input.position, worldMatrix);
    output.lightViewPosition = mul(output.lightViewPosition, lightViewMatrix);
    output.lightViewPosition = mul(output.lightViewPosition, lightProjectionMatrix);

    output.tex = input.tex;
    
    // ���� ��ĸ� ������ �븻 ���� ���.
    output.normal = mul(input.normal, (float3x3) worldMatrix);
	
    output.normal = normalize(output.normal);

    // ������ ���� ��ġ ���.
    worldPosition = mul(input.position, worldMatrix);

    // �������� ������ ���ϴ� ���͸� ���.
    output.lightPos = lightPosition.xyz - worldPosition.xyz;

    output.lightPos = normalize(output.lightPos);

    return output;
}