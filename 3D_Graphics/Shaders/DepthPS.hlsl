struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXTURE0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float depthValue;
    float4 color;
	
    // z값을 동차 좌표의 w로 나누어 픽셀의 깊이 값을 얻어옴.
    depthValue = input.depthPosition.z / input.depthPosition.w;

    //// 깊이 버퍼의 첫 10%는 빨간색으로 칠함.
    //if (depthValue < 0.9f)
    //{
    //    color = float4(1.0, 0.0f, 0.0f, 1.0f);
    //}
	
    //// 다음 0.025%는 초록색으로 칠함.
    //if (depthValue > 0.9f)
    //{
    //    color = float4(0.0, 1.0f, 0.0f, 1.0f);
    //}
    
    //// 나머지 부분들은 파란색으로 칠해줌.
    //if (depthValue > 0.925f)
    //{
    //    color = float4(0.0, 0.0f, 1.0f, 1.0f);
    //}
    
    color = float4(depthValue, depthValue, depthValue, 1.0f);

    return color;
}
