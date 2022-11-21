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
    
    // ���� ��ġ�� hull ���̴��� �Ѱ���.
    output.position = input.position;
    
    // ������ ���� �� ���� hull ���̴��� �Ѱ���.
    output.color = input.color;
    
    return output;
}