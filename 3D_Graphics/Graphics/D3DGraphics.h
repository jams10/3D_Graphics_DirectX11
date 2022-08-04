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
	DxgiInfoManager infoManager; // ����� ����� ���� DxgiInfoManager ��ü�� ��� �ְ� ��.
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