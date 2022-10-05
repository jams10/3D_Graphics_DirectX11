cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer ClipPlaneBuffer
{
    float4 clipPlane;
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
    float clip : SV_ClipDistance0;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.tex = input.tex;
    
    // 노말 좌표에는 월드 행렬만 곱해 월드 공간으로 변환.
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);
	
    // 클리핑 평면 설정.
    output.clip = dot(mul(input.position, worldMatrix), clipPlane);

    return output;
}