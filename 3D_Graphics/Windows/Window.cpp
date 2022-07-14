#include "Window.h"
#include <ErrorHandle/WindowsExceptionMacros.h>
#include <sstream>
#include <imgui/imgui_impl_win32.h>

#pragma region WindowClass
// 윈도우 클래스
Window::WindowClass Window::WindowClass::wndClass; // static 멤버 변수 정의
// WindowClass 생성자. 윈도우 클래스 생성을 담당.
Window::WindowClass::WindowClass() noexcept
	:
	hInst(GetModuleHandle(nullptr)) // exe 파일에서는 이 함수로 얻어오는 인스턴스 핸들이나 WinMain()의 매개변수인 hInstance의 값이 다르지 않다. 
									// 그러나, DLL 안에 윈도우들을 생성할 경우 WinMain의 hInstance는 DLL의 hInstance를, GetModuleHandle은 여전히 DLL을 로드한 exe 파일의 HINSTANCE를 반환 한다.
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = HandleMsgSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetInstance();
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = GetName();
	wc.hIconSm = nullptr;

	RegisterClassEx(&wc);
}
// WindowClass 소멸자. 등록한 윈도우 해제 담당.
Window::WindowClass::~WindowClass()
{
	UnregisterClass(wndClassName, GetInstance()); // 등록한 윈도우를 해제 해줌.
}
// 윈도우 클래스의 이름을 리턴해주는 함수.
const wchar_t* Window::WindowClass::GetName() noexcept
{
	return wndClassName;
}
// 프로그램 인스턴스에 대한 핸들을 리턴해주는 함수.
HINSTANCE Window::WindowClass::GetInstance() noexcept
{
	return wndClass.hInst;
}
#pragma endregion

#pragma region Window
// Window 클래스 생성자. 윈도우 생성 및 설정
Window::Window(const wchar_t* name, int width, int height)
	:
	screenWidth(width),
	screenHeight(height)
{
	// 원하는 client 영역 크기에 맞춰서 winodw의 크기를 계산해줌.
	RECT wr;
	wr.left = 100;
	wr.right = width + wr.left;
	wr.top = 100;
	wr.bottom = height + wr.top;

	// 윈도우 사이즈 조절.
	if (!AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE))
	{
		throw WND_LAST_EXCEPT();
	}

	// 윈도우를 생성하고 윈도우에 대한 핸들을 얻어옴.
	hWnd = CreateWindow(
		WindowClass::GetName(), name,
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
		nullptr, nullptr, WindowClass::GetInstance(), this
	);

	// 윈도우 생성에 실패한 경우
	if (hWnd == nullptr)
	{
		throw WND_LAST_EXCEPT();
	}

	// 윈도우 화면에 띄우기.
	ShowWindow(hWnd, SW_SHOWDEFAULT);

	// Imgui Win32 구현 초기화.
	ImGui_ImplWin32_Init(hWnd);
}

// Window 클래스 소멸자. 생성한 윈도우 파괴.
Window::~Window()
{
	ImGui_ImplWin32_Shutdown();
	DestroyWindow(hWnd);
}

// 윈도우 타이틀 이름 변경해주는 함수.
void Window::SetTitle(const std::wstring& title)
{
	if (SetWindowText(hWnd, title.c_str()) == 0)
	{
		throw WND_LAST_EXCEPT();
	}
}

// 윈도우 메시지 루프. 윈도우 메시지를 프로시져로 보내주는 함수.
std::optional<int> Window::ProcessMessages() noexcept
{
	MSG msg;
	// 메시지 큐에 메시지가 있으면, 해당 메시지를 제거하고 프로시져로 보내줌.(큐가 비어있어도 블록 상태에 들어가지 않음.)
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		// GetMessage와 다르게 PeekMessage는 직접 WM_QUIT 메시지에 대해 체크해 주어야 함.
		if (msg.message == WM_QUIT)
		{
			// PostQuitMessage 함수에 지정된 종료 코드가 wParam안에 있는데 이 코드를 들고있는 optional 객체를 리턴함.
			return msg.wParam;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 앱을 끝내지 않는 경우 빈 optional을 리턴함. 빈 optional은 nullopt 값. if문으로 체크하면 if(false)와 비슷하게 동작함.
	return {};
}

// 직접 만들어준 멤버 함수를 윈도우 프로시져로 사용하기 위한 기본 설정을 담당하는 함수.
LRESULT CALLBACK Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// CreateWindow()의 마지막 인자로 넘겨준 this 포인터를 이용해 윈도우 API 쪽에있는 윈도우 클래스 포인터를 저장함.
	if (msg == WM_NCCREATE)
	{
		// 생성 데이터로 부터 윈도우 클래스에 대한 포인터를 추출.
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);

		// WinAPI에 의해 관리되는 사용자 데이터를 윈도우 클래스에 대한 포인터를 저장하도록 설정함.
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));

		// 메시지 프로시져를 일반 핸들러로 변경하고 메시지를 받아들일 준비를 함.
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));

		// window class 핸들러에 메시지를 전달함.
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}

	// WN_NCCREATE 메시지 전에 메시지를 받는다면, 일반 핸들러로 메시지를 처리.
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

// 윈도우 메시지를 우리가 작성한 멤버 함수로 전달하기 위한 함수.
LRESULT CALLBACK Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// window class에 대한 포인터를 얻어옴.
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	// window class 핸들러에 메시지를 전달함.
	return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

// 실제 윈도우 메시지를 처리해 줄 함수.
LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// 윈도우 메시지가 들어오면, imgui가 먼저 처리하도록 함. (먼저 맛을 봄.)
	// 그래서, 그래서 해당 메시지를 imgui의 프로시져가 처리하게 되면(true를 리턴) 우리의 HandleMsg에서는 처리할 게 없으므로, 바로 리턴해서 프로시져 빠져나감.
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		return true;
	}
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_KILLFOCUS:    // 윈도우에 대한 포커스를 잃어버리면 키 입력 상태를 모두 초기화 해줌.
		kbd.ClearState();
		break;
#pragma region KeyboardMSG
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (!(lParam & 0x40000000) || kbd.AutorepeatIsEnabled())
		{
			kbd.OnKeyPressed(static_cast<unsigned char>(wParam));
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		kbd.OnKeyReleased(static_cast<unsigned char>(wParam));
		break;
	case WM_CHAR:
		kbd.OnChar(static_cast<unsigned char>(wParam));
		break;
#pragma endregion
#pragma region MouseMSG
	case WM_MOUSEMOVE:
	{
		const POINTS pt = MAKEPOINTS(lParam);

		// 마우스 좌표가 클라이언트 영역 안에 있는 경우
		if (pt.x >= 0 && pt.x < screenWidth && pt.y >= 0 && pt.y < screenHeight)
		{
			mouse.OnMouseMove(pt.x, pt.y);    // 마우스 좌표를 갱신.
			if (!mouse.IsInWindow())          // 마우스가 이전에 영역안에 있는 상태가 아닌 경우였다면,
			{
				SetCapture(hWnd);             // 마우스 캡쳐
				mouse.OnMouseEnter();         // 마우스를 클라이언트 영역 안에 있는 상태로 바꿔줌.
			}
		}

		// 마우스 좌표가 클라이언트 영역 밖에 있는 경우
		else
		{
			if (wParam & (MK_LBUTTON | MK_RBUTTON)) // 만약 마우스가 눌린 상태로 바깥에 나갔다면, 캡쳐를 유지하고, 마우스 좌표 갱신해줌.
			{
				mouse.OnMouseMove(pt.x, pt.y);
			}
			else                                    // 마우스가 눌리지 않았다면 캡쳐를 풀어주고, 영역 밖으로 나간 상태로 바꿔줌.
			{
				ReleaseCapture();
				mouse.OnMouseLeave();
			}
		}
		break;
	}
	case WM_LBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnLeftPressed(pt.x, pt.y);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnRightPressed(pt.x, pt.y);
		break;
	}
	case WM_LBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnLeftReleased(pt.x, pt.y);
		// 영역 밖으로 나갔다면 마우스 캡쳐를 풀어줌.
		if (pt.x < 0 || pt.x >= screenWidth || pt.y < 0 || pt.y >= screenHeight)
		{
			ReleaseCapture();
			mouse.OnMouseLeave();
		}
		break;
	}
	case WM_RBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnRightReleased(pt.x, pt.y);
		// 영역 밖으로 나갔다면 마우스 캡쳐를 풀어줌.
		if (pt.x < 0 || pt.x >= screenWidth || pt.y < 0 || pt.y >= screenHeight)
		{
			ReleaseCapture();
			mouse.OnMouseLeave();
		}
		break;
	}
	case WM_MOUSEWHEEL:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
		mouse.OnWheelDelta(pt.x, pt.y, delta);
		break;
	}
#pragma endregion
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
#pragma endregion

#pragma region Exception
// 에러 코드를 문자열로 변환해주는 함수.
std::string Window::Exception::TranslateErrorCode(HRESULT hr) noexcept
{
	char* pMsgBuf = nullptr;
	/*
		 <FormatMessage 함수>
		 HRESULT 값을 받아서 에러 코드를 에러 메시지로 변환해줌.
		 윈도우가 에러 문자열을 위한 메모리를 할당하고, 인자로 넘긴 포인터가 이를 가리키도록 해줌.
		 FORMAT_MESSAGE_ALLOCATE_BUFFER : os가 알아서 메시지 저장 공간을 할당하도록 함.
		 FORMAT_MESSAGE_FROM_SYSTEM     : os로 부터 에러 메시지 가져옴.
		 FORMAT_MESSAGE_IGNORE_INSERTS  : 메시지 정의 삽입 시퀀스 무시.
	 */
	const DWORD nMsgLen = FormatMessageA( // WinMain에서 MessageBoxA로 멀티바이트 버전을 사용하고 있어 FormatMessage도 멀티바이트 버전을 사용함.
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&pMsgBuf), 0, nullptr
	);
	// 문자열 길이가 0이면 실패를 나타냄.
	if (nMsgLen == 0)
	{
		return "Unidentified error code";
	}
	// 에러 문자열을 윈도우가 할당해준 버퍼에서 std::string으로 복사해줌.
	std::string errorString = pMsgBuf;
	// free windows buffer
	/*
		<LocalFree 함수>
		특정 로컬 메모리 오브젝트를 해제하고, 그 핸들을 유효하지 않도록 바꿈.
		에러 메시지를 위해 할당한 공간을 해제.
	*/
	LocalFree(pMsgBuf);
	return errorString;
}

Window::HrException::HrException(int line, const char* file, HRESULT hr) noexcept
	:
	Exception(line, file),
	hr(hr)
{}

// what() : 출력할 에러 문자열을 생성해 리턴하는 함수. std::exception으로 부터 오버라이딩.
const char* Window::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

// 예외 타입 문자열을 리턴해주는 함수.
const char* Window::HrException::GetType() const noexcept
{
	return "Custom Window Exception";
}

// 에러 코드(HRESULT)를 리턴해주는 함수.
HRESULT Window::HrException::GetErrorCode() const noexcept
{
	return hr;
}

// dxerr 라이브러리를 통해 에러에 관한 설명 문자열을 리턴해주는 함수.
std::string Window::HrException::GetErrorDescription() const noexcept
{
	return Exception::TranslateErrorCode(hr);
}

// 예외 타입 문자열을 리턴해주는 함수.
const char* Window::NoGfxException::GetType() const noexcept
{
	return "Custom Window Exception [No Graphics]";
}
#pragma endregion