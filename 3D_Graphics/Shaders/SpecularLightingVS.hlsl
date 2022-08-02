// ���� ����
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer CameraBuffer
{
    float3 cameraPosition;
    float  padding;
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
    float3 viewDirection : TEXCOODR1; // ī�޶󿡼� ������ ���ϴ� ����.
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    float4 worldPosition;
    
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
    
    // ������ ���� ���� ��ǥ�� �������.
    worldPosition = mul(input.position, worldMatrix);
    
    // ���� �������� ��ü�� �������� ī�޶� ���ϴ� ���͸� ������. (������ ī�޶�->���� ����������, �ݻ籤�� ������ �����ֱ� ���� �ݴ�� ������.)
    output.viewDirection = cameraPosition.xyz - worldPosition.xyz;
    
    // ������ ���� ���� ����� ���� ����ȭ�Ͽ� ���� ���ͷ� �ٲ���.
    output.viewDirection = normalize(output.viewDirection);

    return output;
}