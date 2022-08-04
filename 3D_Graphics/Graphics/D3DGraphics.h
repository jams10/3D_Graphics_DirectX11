#pragma once

#include <Windows/WindowsHeaders.h>
#include <ErrorHandle/CustomException.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl.h>

namespace dx = DirectX;

class D3DGraphics
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
public:
	D3DGraphics();
	D3DGraphics(const D3DGraphics&) = delete;
	D3DGraphics& operator=(const D3DGraphics&) = delete;
	~D3DGraphics();
	
	bool Initialize(int screenWidth, int screenHeight, bool vsync, HWND hWnd, float screenDepth, float screenNear);

	void BeginFrame(float red, float green, float blue, float alpha) noexcept;
	void EndFrame();

public:
	ID3D11Device* GetDevice() { return m_pDevice.Get(); }
	ID3D11DeviceContext* GetContext() { return m_pContext.Get(); }
	DxgiInfoManager& GetInfoManager() { return infoManager; }
	
	dx::XMMATRIX GetProjectionMatrix() { return m_projectionMatrix; }
	dx::XMMATRIX GetWorldMatrix() { return m_worldMatrix; }
	dx::XMMATRIX GetOrthMatrix() { return m_orthoMatrix; }

	void TurnZBufferOn();
	void TurnZBufferOff();

private:
#ifndef NDEBUG
	DxgiInfoManager infoManager; // 디버그 모드일 때만 DxgiInfoManager 객체를 들고 있게 함.
#endif

private:
	bool m_vsync_enabled;
	bool imguiEnabled;

	dx::XMMATRIX m_projectionMatrix;
	dx::XMMATRIX m_worldMatrix;
	dx::XMMATRIX m_orthoMatrix;

	Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_pSwap;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pTarget;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_pDSV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_pDepthStencilState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_pDepthDisabledStencilState;
};