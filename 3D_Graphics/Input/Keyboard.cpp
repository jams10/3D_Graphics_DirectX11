#include "Keyboard.h"

// 가상 키 코드를 입력 받아 해당 키가 눌렸는지 여부를 리턴해주는 함수.
bool Keyboard::KeyIsPressed(unsigned char keycode) const noexcept
{
	return keystates[keycode];
}

// 키 Event 버퍼에 저장된 가장 오래된 Event를 꺼내 리턴해주는 함수.
Keyboard::Event Keyboard::ReadKey() noexcept
{
	if (keybuffer.size() > 0u)
	{
		Keyboard::Event e = keybuffer.front();
		keybuffer.pop();
		return e;
	}
	else
	{
		return Keyboard::Event();
	}
}

// 키 Event 버퍼가 비었는지 여부를 리턴해주는 함수.
bool Keyboard::KeyIsEmpty() const noexcept
{
	return keybuffer.empty();
}

// 키 char 버퍼에 있는 가장 오래된 char 값을 꺼내 리턴해주는 함수.
char Keyboard::ReadChar() noexcept
{
	if (charbuffer.size() > 0u)
	{
		unsigned char charcode = charbuffer.front();
		charbuffer.pop();
		return charcode;
	}
	else
	{
		return 0;
	}
}

// 키 char 버퍼가 비었는지 여부를 리턴해주는 함수.
bool Keyboard::CharIsEmpty() const noexcept
{
	return charbuffer.empty();
}

// 키 Event 버퍼를 비워주는 함수.
void Keyboard::FlushKey() noexcept
{
	keybuffer = std::queue<Event>();
}

// 키 char 버퍼를 비워주는 함수.
void Keyboard::FlushChar() noexcept
{
	charbuffer = std::queue<char>();
}

// 키 Event, char 버퍼를 모두 비워주는 함수.
void Keyboard::Flush() noexcept
{
	FlushKey();
	FlushChar();
}

// autorepeatEnabled를 켜주는 함수.
void Keyboard::EnableAutorepeat() noexcept
{
	autorepeatEnabled = true;
}

// autorepeatEnabled를 꺼주는 함수.
void Keyboard::DisableAutorepeat() noexcept
{
	autorepeatEnabled = false;
}

// autorepeatEnabled 상태를 리턴해주는 함수.
bool Keyboard::AutorepeatIsEnabled() const noexcept
{
	return autorepeatEnabled;
}

// 입력을 누른 키의 가상 키 코드를 하나 받아 키 Event 버퍼에 넣어주는 함수.
void Keyboard::OnKeyPressed(unsigned char keycode) noexcept
{
	keystates[keycode] = true;                                                 // 해당 가상 키 코드를 인덱스로 하는 위치의 값을 true로 바꿔줌.
	keybuffer.push(Keyboard::Event(Keyboard::Event::Type::Press, keycode));    // Event 타입은 Press, 해당 가상 키 코드와 함께 Event 버퍼에 넣어줌.
	TrimBuffer(keybuffer);                                                     // 버퍼 원소의 개수가 버퍼 크기를 벗어나지 않도록 정리.
}

// 입력을 뗀 가상 키 코드를 하나 받아 키 Event 버퍼에 넣어주는 함수.
void Keyboard::OnKeyReleased(unsigned char keycode) noexcept
{
	keystates[keycode] = false;                                                // 해당 가상 키 코드를 인덱스로 하는 위치의 값을 false로 바꿔줌.
	keybuffer.push(Keyboard::Event(Keyboard::Event::Type::Release, keycode));  // Event 타입은 Release, 해당 가상 키 코드와 함께 Event 버퍼에 넣어줌.
	TrimBuffer(keybuffer);                                                     // 버퍼 원소의 개수가 버퍼 크기를 벗어나지 않도록 정리.
}

// 입력한 키의 char 값을 하나 받아 char 버퍼에 넣어주는 함수.
void Keyboard::OnChar(char character) noexcept
{
	charbuffer.push(character);                                                // 키 char 버퍼에 char 값을 넣어줌.
	TrimBuffer(charbuffer);                                                    // 버퍼 원소의 개수가 버퍼 크기를 벗어나지 않도록 정리.
}

// 모든 가상 키 상태를 리셋해주는 함수. 모든 가상 키 상태가 false가 됨.
void Keyboard::ClearState() noexcept
{
	keystates.reset();
}

// 원소의 개수가 버퍼 크기를 유지하도록 버퍼를 정리해주는 함수.
template<typename T>
void Keyboard::TrimBuffer(std::queue<T>& buffer) noexcept
{
	// 우리가 정해준 버퍼 크기가 될 때 까지 버퍼에 들어있는 값을 가장 오래된 순서부터 꺼내줌.
	while (buffer.size() > bufferSize)
	{
		buffer.pop();
	}
}