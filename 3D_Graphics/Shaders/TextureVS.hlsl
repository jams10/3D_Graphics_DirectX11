
// ���̴� ���� ����
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// ����� ���� Ÿ��
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
    
	// ���Ϳ� ���� ��ȯ�� �����ϱ� ���� ������ ������ ���� 1�� �������.
    input.position.w = 1.0f;

	// �Է����� ���� ������ ��ġ�� ���� ����, ��, ���� ����� ���� ��ũ�� ��ǥ�� ��ȯ����.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // �ؽ��� ��ǥ ���� ��� �ȼ� ���̴����� �״�� ����� �� �ֵ��� ��� ���� ����.
    output.tex = input.tex;
    
    return output;
}