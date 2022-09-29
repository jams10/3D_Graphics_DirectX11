cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// �ݻ� �� ���.
cbuffer ReflectionBuffer
{
    matrix reflectionMatrix;
};

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

// �ȼ� ���̴��� ������ �ݻ� �ؽ����� ��ġ�� �Ѱ���.
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 reflectionPosition : TEXCOORD1;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    matrix reflectProjectWorld;
	
    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.tex = input.tex;

    // �ݻ� �� ����� ���� �ݻ� �� ������ ����-��-���� ����� �������.
    reflectProjectWorld = mul(reflectionMatrix, projectionMatrix);
    reflectProjectWorld = mul(worldMatrix, reflectProjectWorld);

    // ���� �Է� ��ġ�� �ݻ� ���� ����� ���� �ݻ� ��ġ�� ��ȯ����.
    output.reflectionPosition = mul(input.position, reflectProjectWorld);

    return output;
}