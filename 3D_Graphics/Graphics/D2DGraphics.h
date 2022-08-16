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
		using CustomException::CustomException; // CustomException ������ ���.
	};
	// HrException : HRESULT�� �Ѿ���� ������ ���� ���� ó�� Ŭ����.
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
	// InfoException : HRESULT�� �������� �ʴ� �Լ����� ���� ó���� ���� ����� ���â�� �ִ� ������ �����ٰ� �Ѱ��ִ� Ŭ����.
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
	// DeviceRemovedException : HRESULT�� DXGI_ERROR_DEVICE_REMOVED�� ��� ���� ó������ ���� Ŭ����.
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
	DxgiInfoManager infoManager; // ����� ����� ���� DxgiInfoManager ��ü�� ��� �ְ� ��.
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