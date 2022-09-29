cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// 반사 뷰 행렬.
cbuffer ReflectionBuffer
{
    matrix reflectionMatrix;
};

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

// 픽셀 셰이더로 투영된 반사 텍스쳐의 위치를 넘겨줌.
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 reflectionPosition : TEXCOORD1;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    matrix reflectProjectWorld;
	
    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.tex = input.tex;

    // 반사 뷰 행렬을 통해 반사 뷰 기준의 월드-뷰-투영 행렬을 만들어줌.
    reflectProjectWorld = mul(reflectionMatrix, projectionMatrix);
    reflectProjectWorld = mul(worldMatrix, reflectProjectWorld);

    // 정점 입력 위치를 반사 투영 행렬을 통해 반사 위치로 변환해줌.
    output.reflectionPosition = mul(input.position, reflectProjectWorld);

    return output;
}