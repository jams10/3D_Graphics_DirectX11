Texture2D fireTexture : register(t0);
Texture2D noiseTexture : register(t1);
Texture2D alphaTexture : register(t2);
SamplerState SampleType;
SamplerState SampleType2;

cbuffer DistortionBuffer
{
    float2 distortion1;
    float2 distortion2;
    float2 distortion3;
    float distortionScale;
    float distortionBias;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float2 texCoords1 : TEXCOORD1;
    float2 texCoords2 : TEXCOORD2;
    float2 texCoords3 : TEXCOORD3;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 noise1;
    float4 noise2;
    float4 noise3;
    float4 finalNoise;
    float perturb;
    float2 noiseCoords;
    float4 fireColor;
    float4 alphaColor;

    // 세 개의 다른 크기를 갖는 노이즈 텍스쳐를 얻기 위해 하나의 노이즈 텍스쳐를 서로 다른 좌표로 샘플링함.
    noise1 = noiseTexture.Sample(SampleType, input.texCoords1);
    noise2 = noiseTexture.Sample(SampleType, input.texCoords2);
    noise3 = noiseTexture.Sample(SampleType, input.texCoords3);

    // 노이즈 텍스쳐의 범위를 0 ~ 1에서 -1 ~ +1로 변환해줌.
    noise1 = (noise1 - 0.5f) * 2.0f;
    noise2 = (noise2 - 0.5f) * 2.0f;
    noise3 = (noise3 - 0.5f) * 2.0f;

    // 세 개의 x, y 값들을 이용해 세 개의 노이즈 x, y 좌표를 왜곡.
    noise1.xy = noise1.xy * distortion1.xy;
    noise2.xy = noise2.xy * distortion2.xy;
    noise3.xy = noise3.xy * distortion3.xy;

    // 세 개의 노이즈를 하나의 노이즈로 결합.
    finalNoise = noise1 + noise2 + noise3;

    // 왜곡 스케일과 편차 값들을을 통해 입력 텍스쳐 Y 좌표를 교란.
    // 불꽃이 위쪽에서 흔들리는 효과를 만들어내기 위해 위쪽으로 갈수록 교란이 더 많이 일어나도록 함.
    perturb = ((1.0f - input.tex.y) * distortionScale) + distortionBias;

    // 불의 색상 텍스쳐를 샘플링할 때 사용할 교란되고 왜곡된 텍스쳐 샘플링 좌표를 만들어줌.
    noiseCoords.xy = (finalNoise.xy * perturb) + input.tex.xy;

    // 위에서 구한 교란 & 왜곡된 텍스쳐 샘플링 좌표를 사용해 불 색상 텍스쳐를 샘플링 해줌.
    // wrap이 아닌 clamp 모드를 사용해 샘플링. 
    fireColor = fireTexture.Sample(SampleType2, noiseCoords.xy);

    // 위에서 구한 교란 & 왜곡된 텍스쳐 샘플링 좌표를 사용해 알파 텍스쳐를 샘플링 해줌.
    // wrap이 아닌 clamp 모드를 사용해 샘플링.
    alphaColor = alphaTexture.Sample(SampleType2, noiseCoords.xy);

	// 최종 픽셀 색상의 알파 채널을 샘플링한 알파 텍스쳐 값으로 지정해 불꽃 모양을 만들어줌.
    fireColor.a = alphaColor;
	
    return fireColor;
}
