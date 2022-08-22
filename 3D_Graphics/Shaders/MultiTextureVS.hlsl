cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    
	// 벡터에 아핀 변환을 적용하기 위해 마지막 차원의 값을 1로 만들어줌.
    input.position.w = 1.0f;

    // 정점에 월드, 뷰, 투영 행렬을 적용.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // 픽셀 셰이더에서 사용하기 위해 텍스쳐 좌표를 저장.
    output.tex = input.tex;
    
    return output;
}