#pragma once

#include <Windows/\WindowsHeaders.h>
#include <ErrorHandle/CustomException.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <d3d11.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <wrl.h>

class D3DGraphics;

class D2DGraphics
{
	friend class Graphics;
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
private:
#ifndef NDEBUG
	DxgiInfoManager infoManager; // 디버그 모드일 때만 DxgiInfoManager 객체를 들고 있게 함.
#endif

public:
	D2DGraphics(const D3DGraphics& D3DGraphics);
	D2DGraphics(const D2DGraphics&) = delete;
	D2DGraphics& operator=(const D2DGraphics&) = delete;
	~D2DGraphics();

	void Initialize(const D3DGraphics& D3DGraphics);
	void Release();

	void BeginFrame();
	void EndFrame();

public:
	void DrawBox(const int& left, const int& top, const int& right, const int& bottom);
	void WriteText(std::wstring s, const float& left, const float& top, const float& right, const float& bottom);

private:
	ID2D1Factory1* m_pFactory;
	IDWriteFactory* m_pWriteFactory;
	ID2D1Device* m_pDevice;
	ID2D1DeviceContext* m_pContext;
	ID2D1Bitmap1* m_pBitmap;
	IDXGISurface* m_pSurface;
	ID2D1SolidColorBrush* m_pBrush;
	IDWriteTextFormat* m_pTextFormat;
};