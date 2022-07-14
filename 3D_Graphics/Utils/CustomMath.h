#pragma once
#include <math.h>

// 원주율 정의.
constexpr float PI = 3.14159265f;
constexpr double PI_D = 3.1415926535897932;

// 제곱을 리턴하는 템플릿 함수.
template <typename T>
constexpr auto sq(const T& x)
{
	return x * x;
}

// -π ~ π로 각도를 제한시켜 주는 템플릿 함수.
template<typename T>
T wrap_angle(T theta)
{
	// 회전 각도의 경우 부동 소수점이기 때문에 값이 계속 누적될 경우 값이 커져 정밀도가 떨어짐.
	// 따라서 -pi ~ pi 범위로 각도를 제한시켜 줌.
	// fmod : 나머지 계산 함수.
	const T modded = fmod(theta, (T)2.0 * (T)PI_D); // 분자 : theta / 분모 : 2π
	return (modded > (T)PI_D) ? // 나머지가 π보다 클 경우, 나머지에서 -2π만큼 빼서 -π ~ π 범위로 맞춰줌.(주기 함수이기 때문에 2π 뺀 값과 같음.)
		(modded - (T)2.0 * (T)PI_D) :
		modded;
}

// 선형 보간 템플릿 함수.
template<typename T>
constexpr T interpolate(const T& src, const T& dst, float alpha)
{
	return src + (dst - src) * alpha;
}

// 60분법 각도를 라디안으로 변환해주는 템플릿 함수.
template<typename T>
constexpr T to_rad(T deg)
{
	return deg * PI / (T)180.0;
}