cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer ScreenSizeBuffer
{
    float screenHeight;
    float3 padding;
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
    float2 texCoord1 : TEXCOORD1;
    float2 texCoord2 : TEXCOORD2;
    float2 texCoord3 : TEXCOORD3;
    float2 texCoord4 : TEXCOORD4;
    float2 texCoord5 : TEXCOORD5;
    float2 texCoord6 : TEXCOORD6;
    float2 texCoord7 : TEXCOORD7;
    float2 texCoord8 : TEXCOORD8;
    float2 texCoord9 : TEXCOORD9;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    float texelSize;

    input.position.w = 1.0f;
    
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // 픽셀 셰이더에서 사용할 텍스쳐 좌표.
    output.tex = input.tex;
    
    // 1을 화면 너비(혹은 렌더 투 텍스쳐 너비)로 나누어 텍셀의 크기를 구함.
    texelSize = 1.0f / screenHeight;
    
    // 가운데 텍스쳐 좌표와 상하로 인접한 각 4개의 텍스쳐 좌표를 구해줌.
    output.texCoord1 = input.tex + float2(0.0f, texelSize * -4.0f);
    output.texCoord2 = input.tex + float2(0.0f, texelSize * -3.0f);
    output.texCoord3 = input.tex + float2(0.0f, texelSize * -2.0f);
    output.texCoord4 = input.tex + float2(0.0f, texelSize * -1.0f);
    output.texCoord5 = input.tex + float2(0.0f, texelSize * 0.0f);
    output.texCoord6 = input.tex + float2(0.0f, texelSize * 1.0f);
    output.texCoord7 = input.tex + float2(0.0f, texelSize * 2.0f);
    output.texCoord8 = input.tex + float2(0.0f, texelSize * 3.0f);
    output.texCoord9 = input.tex + float2(0.0f, texelSize * 4.0f);

    return output;
}