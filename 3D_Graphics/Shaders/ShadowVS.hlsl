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
    
    // 광원의 관점에서 본 물체의 위치를 계산해줌.
    output.lightViewPosition = mul(input.position, worldMatrix);
    output.lightViewPosition = mul(output.lightViewPosition, lightViewMatrix);
    output.lightViewPosition = mul(output.lightViewPosition, lightProjectionMatrix);

    output.tex = input.tex;
    
    // 월드 행렬만 적용한 노말 벡터 계산.
    output.normal = mul(input.normal, (float3x3) worldMatrix);
	
    output.normal = normalize(output.normal);

    // 정점의 월드 위치 계산.
    worldPosition = mul(input.position, worldMatrix);

    // 정점에서 광원을 향하는 벡터를 계산.
    output.lightPos = lightPosition.xyz - worldPosition.xyz;

    output.lightPos = normalize(output.lightPos);

    return output;
}