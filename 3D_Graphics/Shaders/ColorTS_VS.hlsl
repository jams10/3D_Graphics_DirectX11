struct VertexInputType
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct HullInputType
{
    float3 position : POSITION;
    float4 color : COLOR;
};

HullInputType main(VertexInputType input)
{
    HullInputType output;
    
    // 정점 위치를 hull 셰이더로 넘겨줌.
    output.position = input.position;
    
    // 정점의 색상 값 또한 hull 셰이더로 넘겨줌.
    output.color = input.color;
    
    return output;
}