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
    float3 instancePosition : TEXCOORD1; // 다른 정점 버퍼로 만들어주었지만, 입력 레이아웃에 추가해 주었기 때문에 기본 정점 데이터와 함께 들어올 수 있음.
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    

	// 동차 좌표계를 이용, 벡터, 행렬간의 곱을 통한 이동 변환을 적용하기 위해 마지막 w값을 1로 만들어줌. 
    input.position.w = 1.0f;

    // 특정 인스턴스에 대해 두 번째 버퍼로 넘겨준 위치 값을 이용, 기본 정점 위치에 더해 각 인스턴스들이 특정 위치를 갖도록 함.
    input.position.x += input.instancePosition.x;
    input.position.y += input.instancePosition.y;
    input.position.z += input.instancePosition.z;

    // MVP
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.tex = input.tex;
    
    return output;
}