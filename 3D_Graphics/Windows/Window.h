#pragma once

#include <Windows/WindowsHeaders.h>
#include <Input/Keyboard.h>
#include <Input/Mouse.h>
#include <ErrorHandle/CustomException.h>
#include <optional>
#include <string>

class Window
{
#pragma region Exception
public:
	class Exception : public CustomException
	{
		using CustomException::CustomException;
	public:
		static std::string TranslateErrorCode(HRESULT hr) noexcept;
	};
	// HrException : HRESULT�� �Ѿ���� ������ ���� ���� ó�� Ŭ����.
	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorDescription() const noexcept;
	private:
		HRESULT hr;
	};
	// NoGfxException : Graphics ��ü�� nullptr�� ��츦 ���� ó������ ���� Ŭ����.
	class NoGfxException : public Exception
	{
	public:
		using Exception::Exception;
		const char* GetType() const noexcept override;
	};
#pragma endregion
#pragma region WindowClass
private:
	// ������ Ŭ������ ���/������ �����ϴ� �̱���.
	class WindowClass
	{
	public:
		static const wchar_t* GetName() noexcept;
		static HINSTANCE GetInstance() noexcept;
	private:
		WindowClass() noexcept;
		~WindowClass();
		WindowClass(const WindowClass&) = delete;
		WindowClass& operator=(const WindowClass&) = delete;

		static constexpr const wchar_t* wndClassName = L"WindowClass";
		static WindowClass wndClass; // SINGLETON
		HINSTANCE hInst;
	};
#pragma endregion
public:
	Window(const wchar_t* name, int width, int height);
	~Window();
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	void SetTitle(const std::wstring& title);
	static std::optional<int> ProcessMessages() noexcept; // ��� �����쿡 ���� �޽����� ó���ؾ� �ϹǷ� static���� ������.
	HWND GetWindowHandle() { return hWnd; }

	void EnableCursor();
	void DisableCursor();
private:
	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	void ConfineCursor() noexcept;
	void FreeCursor() noexcept;
	void HideCursor();
	void ShowCursor();
	void EnableImGuiMouse();
	void DisableImGuiMouse();
public:
	Keyboard kbd;
	Mouse mouse;
private:
	int screenWidth;
	int screenHeight;
	HWND hWnd;
	bool cursorEnabled = false;
};