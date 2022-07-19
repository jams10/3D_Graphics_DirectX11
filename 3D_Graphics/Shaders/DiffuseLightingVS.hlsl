// ���� ����
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
    float3 normal : NORMAL; // ������ ����� ���� �븻 ���� ���� �߰�����.
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
    
	// ���Ϳ� ���� ��ȯ�� �����ϱ� ���� ������ ������ ���� 1�� �������.
    input.position.w = 1.0f;

	// �Է����� ���� ������ ��ġ�� ���� ����, ��, ���� ����� ���� ��ũ�� ��ǥ�� ��ȯ����.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // �ȼ� ���̴����� ����ϱ� ���� �ؽ��� ��ǥ�� ��������.
    output.tex = input.tex;
    
	// �븻 ���Ϳ� ���� ����� ������. (��ü�� Ʈ�������� ���� ��� �̸� �븻 ���� ������ �ݿ��� �־�� �ϱ� ����.) 
    output.normal = mul(input.normal, (float3x3) worldMatrix);
	
    // ������ ����� ���� �븻 ���͸� ����ȭ ����.
    output.normal = normalize(output.normal);

    return output;
}