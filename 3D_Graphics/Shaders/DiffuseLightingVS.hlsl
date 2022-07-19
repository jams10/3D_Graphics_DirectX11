// 전역 변수
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// 사용자 정의 타입
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL; // 라이팅 계산을 위해 노말 벡터 값을 추가해줌.
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    
	// 벡터에 아핀 변환을 적용하기 위해 마지막 차원의 값을 1로 만들어줌.
    input.position.w = 1.0f;

	// 입력으로 들어온 정점의 위치에 대해 월드, 뷰, 투영 행렬을 곱해 스크린 좌표로 변환해줌.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // 픽셀 셰이더에서 사용하기 위한 텍스쳐 좌표를 저장해줌.
    output.tex = input.tex;
    
	// 노말 벡터에 월드 행렬을 곱해줌. (물체의 트랜스폼이 변할 경우 이를 노말 벡터 값에도 반영해 주어야 하기 때문.) 
    output.normal = mul(input.normal, (float3x3) worldMatrix);
	
    // 라이팅 계산을 위해 노말 벡터를 정규화 해줌.
    output.normal = normalize(output.normal);

    return output;
}