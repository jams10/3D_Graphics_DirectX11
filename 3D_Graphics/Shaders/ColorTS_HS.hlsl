cbuffer TessellationBuffer
{
    float tessellationAmount;
    float3 padding;
};

// 정점 셰이더로부터 들어오는 입력.
struct HullInputType
{
    float3 position : POSITION;
    float4 color : COLOR;
};

// ConstantOutputType 구조체는 패치 상수 함수의 출력 타입임.
struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

// hull 셰이더의 출력.
struct HullOutputType
{
    float3 position : POSITION;
    float4 color : COLOR;
};

ConstantOutputType ColorPatchConstantFunction(InputPatch<HullInputType, 3> inputPatch, uint patchId : SV_PrimitiveID)
{
    ConstantOutputType output;

    // 삼각형의 세 개의 모서리를 위한 테셀레이션 계수 값을 설정.
    output.edges[0] = tessellationAmount;
    output.edges[1] = tessellationAmount;
    output.edges[2] = tessellationAmount;

    // 삼각형 안쪽을 테셀레이션 하기 위한 테셀레이션 계수 값을 설정.
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

    // 출력 위치로 이 제어 지점의 위치를 설정함.
    output.position = patch[pointId].position;

	// Set the input color as the output color.
    // 출력 색상을 입력 색상으로 지정.
    output.color = patch[pointId].color;

    return output;
}
