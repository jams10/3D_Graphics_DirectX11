cbuffer TessellationBuffer
{
    float tessellationAmount;
    float3 padding;
};

// ���� ���̴��κ��� ������ �Է�.
struct HullInputType
{
    float3 position : POSITION;
    float4 color : COLOR;
};

// ConstantOutputType ����ü�� ��ġ ��� �Լ��� ��� Ÿ����.
struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

// hull ���̴��� ���.
struct HullOutputType
{
    float3 position : POSITION;
    float4 color : COLOR;
};

ConstantOutputType ColorPatchConstantFunction(InputPatch<HullInputType, 3> inputPatch, uint patchId : SV_PrimitiveID)
{
    ConstantOutputType output;

    // �ﰢ���� �� ���� �𼭸��� ���� �׼����̼� ��� ���� ����.
    output.edges[0] = tessellationAmount;
    output.edges[1] = tessellationAmount;
    output.edges[2] = tessellationAmount;

    // �ﰢ�� ������ �׼����̼� �ϱ� ���� �׼����̼� ��� ���� ����.
    output.inside = tessellationAmount;

    return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ColorPatchConstantFunction")]

HullOutputType main(InputPatch<HullInputType, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    HullOutputType output;

    // ��� ��ġ�� �� ���� ������ ��ġ�� ������.
    output.position = patch[pointId].position;

	// Set the input color as the output color.
    // ��� ������ �Է� �������� ����.
    output.color = patch[pointId].color;

    return output;
}
