#pragma once

#include <queue>
#include <bitset>
#include <optional>

// Keyboard : 키보드 입력을 처리해주는 클래스.
class Keyboard
{
	friend class Window;		// Window 클래스가 private 멤버에 접근할 수 있도록 해줌.
public:
#pragma region Event
	class Event
	{
	public:
		enum class Type			// 키 이벤트의 타입.
		{
			Press,				// 키 누름.
			Release,			// 키 뗌.
			Invalid				// 초기화 값. 아무 것도 아닌 상태.
		};
	private:
		Type type;
		unsigned char code;		// 키 Event를 발생시킨 키 코드를 들고 있도록 함.
	public:
		Event()
			:
			type(Type::Invalid),
			code(0u)
		{}
		Event(Type type, unsigned char code) noexcept
			:
			type(type),
			code(code)
		{}
		bool IsPress() const noexcept
		{
			return type == Type::Press;
		}
		bool IsRelease() const noexcept
		{
			return type == Type::Release;
		}
		bool IsValid() const noexcept
		{
			return type != Type::Invalid;
		}
		unsigned char GetCode() const noexcept
		{
			return code;
		}
	};
#pragma endregion
public:
	Keyboard() = default;
	Keyboard(const Keyboard&) = delete;
	Keyboard& operator=(const Keyboard&) = delete;
#pragma region KeyEvent
	bool KeyIsPressed(unsigned char keycode) const noexcept;
	std::optional<Event> ReadKey() noexcept;
	bool KeyIsEmpty() const noexcept;
	void FlushKey() noexcept;
#pragma endregion
#pragma region CharEvent
	std::optional<char> ReadChar() noexcept;
	bool CharIsEmpty() const noexcept;
	void FlushChar() noexcept;
	void Flush() noexcept; // key, char를 둘다 flush
#pragma endregion
#pragma region AutoRepeat
	void EnableAutorepeat() noexcept;
	void DisableAutorepeat() noexcept;
	bool AutorepeatIsEnabled() const noexcept;
#pragma endregion
private: // Window 클래스가 사용할 함수들.
	void OnKeyPressed(unsigned char keycode) noexcept;
	void OnKeyReleased(unsigned char keycode) noexcept;
	void OnChar(char character) noexcept;
	void ClearState() noexcept;
	template<typename T>
	static void TrimBuffer(std::queue<T>& buffer) noexcept;
private:
	static constexpr unsigned int nKeys = 256u;       // 가상 키 코드의 개수. 1바이트 256개.
	static constexpr unsigned int bufferSize = 16u;   // 키 이벤트를 저장해줄 버퍼의 최대 크기.
	bool autorepeatEnabled = false;                   // autorepeatEnabled 체크 변수.
	std::bitset<nKeys> keystates;                     // 해당 가상 키가 눌렸는지 여부를 저장할 bitset.
	std::queue<Event> keybuffer;                      // 키 Event를 저장해 줄 버퍼.
	std::queue<char> charbuffer;                      // 키 Event에 대한 char를 저장해줄 버퍼.
};