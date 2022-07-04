#pragma once

#include <Windows/WindowsHeaders.h>
#include <ErrorHandle/CustomException.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <d3d11.h>
#include <wrl.h>

class D3DGraphics
{
#pragma region Exception
public:
	class Exception : public CustomException
	{
		using CustomException::CustomException; // CustomException 생성자 상속.
	};
	// HrException : HRESULT로 넘어오는 에러에 대한 예외 처리 클래스.
	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {}) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
		std::string GetErrorDescription() const noexcept;
		std::string GetErrorInfo() const noexcept;
	private:
		HRESULT hr;
		std::string info;
	};
	// InfoException : HRESULT를 리턴하지 않는 함수들의 예외 처리를 위해 디버그 출력창에 있는 정보만 가져다가 넘겨주는 클래스.
	class InfoException : public Exception
	{
	public:
		InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		std::string GetErrorInfo() const noexcept;
	private:
		std::string info;
	};
	// DeviceRemovedException : HRESULT가 DXGI_ERROR_DEVICE_REMOVED인 경우 따로 처리해줄 예외 클래스.
	class DeviceRemovedException : public HrException
	{
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override;
	private:
		std::string reason;
	};
#pragma endregion
public:
	D3DGraphics();
	D3DGraphics(const D3DGraphics&) = delete;
	D3DGraphics& operator=(const D3DGraphics&) = delete;
	~D3DGraphics() = default; // ComPtr을 사용하게 되면서 Com 객체들이 알아서 Release 되므로 기본 소멸자로 바꿔줌.
	
	bool Initialize(int screenWidth, int screenHeight, bool vsync, HWND hWnd, float screenDepth, float screenNear);

	void EndFrame();
	void ClearBuffer(float red, float green, float blue, float alpha) noexcept;

private:
#ifndef NDEBUG
	DxgiInfoManager infoManager; // 디버그 모드일 때만 DxgiInfoManager 객체를 들고 있게 함.
#endif

private:
	bool m_vsync_enabled;

	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;
};