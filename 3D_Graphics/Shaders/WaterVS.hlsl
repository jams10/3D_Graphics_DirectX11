cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer ReflectionBuffer
{
    matrix reflectionMatrix;
};

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 reflectionPosition : TEXCOORD1;
    float4 refractionPosition : TEXCOORD2;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    matrix reflectProjectWorld;
    matrix viewProjectWorld;
	
    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.tex = input.tex;
    
    // 반사 투영 행렬을 만들어 반사 위치를 변환함.
    reflectProjectWorld = mul(reflectionMatrix, projectionMatrix);
    reflectProjectWorld = mul(worldMatrix, reflectProjectWorld);

    output.reflectionPosition = mul(input.position, reflectProjectWorld);

	// 굴절 투영 행렬을 만들어 굴절 위치를 변환함.
    viewProjectWorld = mul(viewMatrix, projectionMatrix);
    viewProjectWorld = mul(worldMatrix, viewProjectWorld);
   
    output.refractionPosition = mul(input.position, viewProjectWorld);
	
    return output;
}