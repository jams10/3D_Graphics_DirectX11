cbuffer PerFrameBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer FogBuffer
{
    float fogStart;
    float fogEnd;
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
    float fogFactor : FOG;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    float4 cameraPosition;
    
    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.tex = input.tex;
   
    cameraPosition = mul(input.position, worldMatrix);
    cameraPosition = mul(cameraPosition, viewMatrix);

    // 선형 공식을 이용, 안개 효과에 사용할 인자를 계산.(뷰 공간에서 계산)  
    output.fogFactor = saturate((fogEnd - cameraPosition.z) / (fogEnd - fogStart));

    return output;
}