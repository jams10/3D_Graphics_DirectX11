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
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;

	// 정점 위치에 월드, 뷰, 투영 행렬 적용.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // 텍스쳐 좌표는 그대로 저장해 픽셀 셰이더에서 사용함.
    output.tex = input.tex;
    
    // normal 벡터에 월드 변환 행렬을 적용하고 그 결과를 정규화 해줌.
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);

    // tangent 벡터에 월드 변환 행렬을 적용하고 그 결과를 정규화 해줌.
    output.tangent = mul(input.tangent, (float3x3) worldMatrix);
    output.tangent = normalize(output.tangent);

    // binormal 벡터에 월드 변환 행렬을 적용하고 그 결과를 정규화 해줌.
    output.binormal = mul(input.binormal, (float3x3) worldMatrix);
    output.binormal = normalize(output.binormal);

    return output;
}