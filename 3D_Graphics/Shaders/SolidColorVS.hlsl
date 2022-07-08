
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
	float4 color : COLOR;
};

struct PixelInputType
{
	float4 position : SV_POSITION; // SV : System Value �ø�ƽ.
	float4 color : COLOR;
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
    
    // ���� ���� ��� �ȼ� ���̴����� �״�� ����� �� �ֵ��� ��� ���� ����.
    output.color = input.color;
    
    return output;
}