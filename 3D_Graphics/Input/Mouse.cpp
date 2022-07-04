#include "Mouse.h"
#include <Windows.h>

// 마우스 좌표를 pair 타입으로 리턴하는 함수.
std::pair<int, int> Mouse::GetPos() const noexcept
{
	return { x,y };
}

// 마우스 X 좌표를 리턴하는 함수.
int Mouse::GetPosX() const noexcept
{
	return x;
}

// 마우스 Y 좌표를 리턴하는 함수.
int Mouse::GetPosY() const noexcept
{
	return y;
}

// 마우스가 윈도우 영역 안에 있는지 여부를 리턴하는 함수.
bool Mouse::IsInWindow() const noexcept
{
	return isInWindow;
}

// 마우스 왼쪽 버튼이 눌렸는지 여부를 리턴하는 함수.
bool Mouse::LeftIsPressed() const noexcept
{
	return leftIsPressed;
}

// 마우스 오른쪽 버튼이 눌렸는지 여부를 리턴하는 함수.
bool Mouse::RightIsPressed() const noexcept
{
	return rightIsPressed;
}

// 마우스 Event 버퍼에서 제일 오래된 Event 하나를 꺼내오는 함수.
Mouse::Event Mouse::Read() noexcept
{
	if (buffer.size() > 0u)
	{
		Mouse::Event e = buffer.front();
		buffer.pop();
		return e;
	}
	else
	{
		return Mouse::Event();
	}
}

// 마우스 Event 버퍼를 비워주는 함수.
void Mouse::Flush() noexcept
{
	buffer = std::queue<Event>();
}

// 마우스가 움직인 경우, 마우스 좌표를 갱신하고 마우스 Event 버퍼에 Move Event를 넣어주는 함수.
void Mouse::OnMouseMove(int newx, int newy) noexcept
{
	x = newx;
	y = newy;

	buffer.push(Mouse::Event(Mouse::Event::Type::Move, *this));
	TrimBuffer();
}

// 마우스가 윈도우 영역 밖으로 나간 경우를 처리해주는 함수.
void Mouse::OnMouseLeave() noexcept
{
	isInWindow = false;
	buffer.push(Mouse::Event(Mouse::Event::Type::Leave, *this));
	TrimBuffer();
}

// 마우스가 윈도우 영역 안으로 들어온 경우를 처리해주는 함수.
void Mouse::OnMouseEnter() noexcept
{
	isInWindow = true;
	buffer.push(Mouse::Event(Mouse::Event::Type::Enter, *this));
	TrimBuffer();
}

// 마우스 왼쪽 버튼이 눌린 경우를 처리해주는 함수.
void Mouse::OnLeftPressed(int x, int y) noexcept
{
	leftIsPressed = true;

	buffer.push(Mouse::Event(Mouse::Event::Type::LPress, *this));
	TrimBuffer();
}

// 마우스 왼쪽 버튼을 뗀 경우를 처리해주는 함수.
void Mouse::OnLeftReleased(int x, int y) noexcept
{
	leftIsPressed = false;

	buffer.push(Mouse::Event(Mouse::Event::Type::LRelease, *this));
	TrimBuffer();
}

// 마우스 오른쪽 버튼을 누른 경우를 처리해주는 함수.
void Mouse::OnRightPressed(int x, int y) noexcept
{
	rightIsPressed = true;

	buffer.push(Mouse::Event(Mouse::Event::Type::RPress, *this));
	TrimBuffer();
}

// 마우스 오른쪽 버튼을 뗀 경우를 처리해주는 함수.
void Mouse::OnRightReleased(int x, int y) noexcept
{
	rightIsPressed = false;

	buffer.push(Mouse::Event(Mouse::Event::Type::RRelease, *this));
	TrimBuffer();
}

// 마우스 휠을 올린 경우를 처리해주는 함수.
void Mouse::OnWheelUp(int x, int y) noexcept
{
	buffer.push(Mouse::Event(Mouse::Event::Type::WheelUp, *this));
	TrimBuffer();
}

// 마우스 휠을 내린 경우를 처리해주는 함수.
void Mouse::OnWheelDown(int x, int y) noexcept
{
	buffer.push(Mouse::Event(Mouse::Event::Type::WheelDown, *this));
	TrimBuffer();
}

// 원소의 개수가 버퍼 크기를 유지하도록 버퍼를 정리해주는 함수.
void Mouse::TrimBuffer() noexcept
{
	while (buffer.size() > bufferSize)
	{
		buffer.pop();
	}
}

// 마우스 휠 이벤트를 생성해주는 함수.
void Mouse::OnWheelDelta(int x, int y, int delta) noexcept
{
	wheelDeltaCarry += delta;

	// 절대값이 120이 될 때마다 휠 이벤트 생성. 
	while (wheelDeltaCarry >= WHEEL_DELTA)
	{
		wheelDeltaCarry -= WHEEL_DELTA;
		OnWheelUp(x, y);
	}
	while (wheelDeltaCarry <= -WHEEL_DELTA)
	{
		wheelDeltaCarry += WHEEL_DELTA;
		OnWheelDown(x, y);
	}
}