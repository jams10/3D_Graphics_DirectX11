#include "GameTimer.h"

GameTimer::GameTimer() noexcept
    :
    hasStopped(false),
    pausedTime(0.0f)
{
    initialTimePoint = std::chrono::steady_clock::now();
    startTimePoint = initialTimePoint;
    stopTimePoint = initialTimePoint;
    prevTimePoint = initialTimePoint;
    currentTimePoint = initialTimePoint;
}

// 게임 전체 시간 계산 함수.
float GameTimer::GetTotalTime() noexcept
{
    // 초기 Time Point에서 현재 Time Point 사이의 duration.
    std::chrono::duration<float> timePerFrameFromStart = currentTimePoint - initialTimePoint;

    // 멈춘 동안의 시간은 전체 게임 시간에 포함을 시키지 않기 위해 빼줌.
    return timePerFrameFromStart.count() - pausedTime;
}

// 프레임 사이 시간 계산 함수.
float GameTimer::GetDeltaTime() noexcept
{
    // duration 클래스를 이용, 경과 time point 계산.
    std::chrono::duration<float> timePerFrame = currentTimePoint - prevTimePoint;
    // duration 클래스의 멤버 함수 count()를 이용, 경과 시간 리턴.
    return timePerFrame.count();
}

// 전체 시간 타이머 재시작.
void GameTimer::ReStartTotalTimer() noexcept
{
    // 타이머 시작 포인트 찍기.
    startTimePoint = std::chrono::steady_clock::now();

    // 멈춘 상태에서 다시 시작하는 경우
    if (hasStopped)
    {
        // 일시 정지된 기간 계산.
        std::chrono::duration<float> timePerFrame = startTimePoint - stopTimePoint;

        // 일시 정지된 시간 누적.
        pausedTime += timePerFrame.count();

        // 일시 정지 시점 포인트도 초기화 해줌.
        stopTimePoint = startTimePoint;

        hasStopped = false;
    }
}

// 전체 시간 타이머 정지.
void GameTimer::PauseTotalTimer() noexcept
{
    // 멈춘 상태가 아닌 경우
    if (!hasStopped)
    {
        // 타이머 정지 포인트 찍기.
        stopTimePoint = std::chrono::steady_clock::now();
        hasStopped = true;
    }
}

// 타이머 틱. 타이머가 계속 돌아가게 함.
void GameTimer::Tick() noexcept
{
    prevTimePoint = currentTimePoint;
    currentTimePoint = std::chrono::steady_clock::now();
}