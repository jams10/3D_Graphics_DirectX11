
struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 main(PixelInputType input) : SV_TARGET
{
    // 입력으로 들어온 색상 값을 그대로 뱉어줌.
    return input.color;
}
