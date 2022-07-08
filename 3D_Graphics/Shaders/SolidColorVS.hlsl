
// 셰이더 전역 변수
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
	float4 color : COLOR;
};

struct PixelInputType
{
	float4 position : SV_POSITION; // SV : System Value 시멘틱.
	float4 color : COLOR;
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
    
    // 색상 값의 경우 픽셀 셰이더에서 그대로 사용할 수 있도록 출력 값에 저장.
    output.color = input.color;
    
    return output;
}