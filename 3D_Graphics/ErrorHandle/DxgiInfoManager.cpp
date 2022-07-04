#include "DxgiInfoManager.h"
#include <Windows/Window.h>
#include <Graphics/D3DGraphics.h>
#include <memory>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <ErrorHandle/WindowsExceptionMacros.h>

#pragma comment(lib, "dxguid.lib")

DxgiInfoManager::DxgiInfoManager()
{
	// DXGIGetDebugInterface의 함수 시그니쳐를 정의
	// DXGIGetDebugInterface를 리턴 타입이 HRESULT, 인자가 REFIID, void**, 호출 규약이 WINAPI(__stdcall)인 함수에 대한 함수 포인터로 함.
	typedef HRESULT(WINAPI* DXGIGetDebugInterface)(REFIID, void**);

	// DXGIGetDebugInterface 함수가 있는 dll를 로드함.
	const auto hModDxgiDebug = LoadLibraryExA("dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (hModDxgiDebug == nullptr)
	{
		throw WND_LAST_EXCEPT();
	}

	// dll에 있는 DXGIGetDebugInterface의 주소를 가져옴.
	const auto DxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>(
		reinterpret_cast<void*>(GetProcAddress(hModDxgiDebug, "DXGIGetDebugInterface"))
		);
	if (DxgiGetDebugInterface == nullptr)
	{
		throw WND_LAST_EXCEPT();
	}

	HRESULT hr;
	// 우리가 가져온 dll에 있는 DXGIGetDebugInterface 함수를 통해 IDXGIInfoQueue를 받아와 ComPtr에 넣어줌.
	GFX_THROW_NOINFO(DxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), &pDxgiInfoQueue));
}

// 디버그 메시지 범위 설정 함수.
void DxgiInfoManager::Set() noexcept
{
	/*
	<GetNumStoredMessages의 매개변수인 DXGI_DEBUG_ID>
		디버그 메시지를 생산한 주체를 식별하는 GUID(Globally Unique Identifier)
		DXGI_DEBUG_ALL은 모든 Direct3D, DXGI 객체 및 애플리케이션에 대한 디버그 메시지를 받아줄 것을 의미.
	*/
	/*
		아래 GetMessages를 함수를 호출 할 때 인덱스를 하나씩 돌면서 InfoQueue에 저장된 디버그 메시지들을 하나 씩 가져올 것인데,
		범위를 정해주지 않으면 어느 디버그 메시지가 어느 코드와 연관된 것인지 알기 힘들다.
		그래서 Set 함수에서 next라는 루프의 시작을 담당하는 인덱스의 값을 Set 호출 시점 까지의 디버그 메시지들의 개수로 설정하여,
		가장 최근 함수 호출에 대해서만 에러 관련 디버그 메시지들을 얻어올 수 있도록 했다.
		즉, 마지막 Set() 호출 이후에 일어난 에러에 대해서만 GetMessages를 통해 에러 메시지를 얻어올 수 있는 것이다.
	*/

	next = pDxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
}

// Debug Information Queue의 에러 관련 메시지들을 저장한 문자열 벡터를 리턴하는 함수.
std::vector<std::string> DxgiInfoManager::GetMessages() const
{
	std::vector<std::string> messages;
	const auto end = pDxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
	for (auto i = next; i < end; i++)
	{
		HRESULT hr;
		SIZE_T messageLength;

		// 인덱스 i 메시지의 크기를 byte로 받아옴.
		GFX_THROW_NOINFO(pDxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &messageLength));

		// 메시지를 위한 메모리를 할당함.
		auto bytes = std::make_unique<byte[]>(messageLength);
		auto pMessage = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes.get());

		// 메시지를 받아와서 그 설명을 벡터에 넣음.
		GFX_THROW_NOINFO(pDxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, pMessage, &messageLength));
		messages.emplace_back(pMessage->pDescription);
	}
	return messages;
}