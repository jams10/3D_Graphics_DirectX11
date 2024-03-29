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
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float clip : SV_ClipDistance0; // 픽셀 셰이더에 들어갈 입력으로 클리핑 평면 시멘틱 추가.
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.tex = input.tex;
    // 클리핑 평면 설정.
    output.clip = dot(mul(input.position, worldMatrix), clipPlane);

    return output;
}