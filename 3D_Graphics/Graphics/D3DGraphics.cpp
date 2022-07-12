#include <Graphics/D3DGraphics.h>
#include <dxerr/dxerr.h>
#include <sstream>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>

#include <d3dcompiler.h>

#pragma comment(lib,"d3d11.lib")        // Direct3D �Լ����� ���ǵ� ���̺귯���� ��ũ����.
#pragma comment(lib, "D3DCompiler.lib") // ���̴��� ��Ÿ�ӿ� ������ ���� �� ����� �� ������, �츮�� ���̴��� �ҷ����� �Լ��� ����ϱ� ���� ����������. 

D3DGraphics::D3DGraphics()
{
}

bool D3DGraphics::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hWnd, float screenDepth, float screenNear)
{
	// ���� ����ȭ ����
	m_vsync_enabled = vsync;

	// ���� ü���� ���� ������ ���� ����ü
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = 0;						   // backbuffer �ʺ�
	sd.BufferDesc.Height = 0;						   // backbuffer ����
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // �ȼ� ����
	sd.BufferDesc.RefreshRate.Numerator = 0;           // hz�� refresh rate ����
	sd.BufferDesc.RefreshRate.Denominator = 0;         // hz�� refresh rate �и�
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;                 // �̹����� �־��� ������� �ػ󵵿� ���߱� ���� ��� �ÿ��������� ���� ����� ����Ŵ.
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; // surface�� �̹����� �����ϱ� ���� raster�� ����ϴ� ����� ����Ŵ.
	sd.SampleDesc.Count = 1;                           // ���� ǥ��ȭ ����
	sd.SampleDesc.Quality = 0;                         // ���� ǥ��ȭ ǰ��
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // ���۸� ����Ÿ�� �뵵�� ���.
	sd.BufferCount = 1;                                // 1���� back buffer�� ���. �׷��� front/back ���� �� 2���� ���۸� ���.
	sd.OutputWindow = hWnd;                            // ��� �����츦 ����.
	sd.Windowed = TRUE;                                // â��带 ����.
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;          // IDXGISwapChain1::Present1 ȣ�� �Ŀ� ���÷��� ȭ�鿡 �ִ� �ȼ����� ��� ó���� ���ΰ�? ���⼭�� �׳� ����.
	sd.Flags = 0;                                      // �߰����� �÷��׵�.

	UINT swapCreateFlags = 0u;
#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;      // ����� ����� ��쿡 ����̽� ���� �÷��׸� D3D11_CREATE_DEVICE_DEBUG�� ��������.
#endif

	HRESULT hr; // API �Լ��� �����ϴ� HRESULT�� �޾ƿ� ������ �α� ����.

	// device�� swap chain, device context�� ������.
	GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
		nullptr,                    // IDXGIAdapter�� nullptr�� ������, �⺻ ����͸� ��������.
		D3D_DRIVER_TYPE_HARDWARE, // ������ ����̹� Ÿ��.
		nullptr,                    // ����Ʈ���� �����Ͷ������� �����ϴ� DLL�� ���� �ڵ�.
		swapCreateFlags,                   // ����� ����� ��� D3D11_CREATE_DEVICE_DEBUG�� ������, ����� �м� ������ ���� �� ����.
		nullptr,               // D3D_FEATURE_LEVEL. D3D ��ġ ���� ����. nullptr�� �ָ�, �⺻ 6���� ���� ������ ���.
		0,                      // pFeatureLevels�� �ִ� ���ҵ��� ����.
		D3D11_SDK_VERSION,        // SDK_VERSION. ���� �������� �ٸ�.
		&sd,                   // ���� ü�� ������
		&m_pSwap,                   // ������ ���� ü��
		&m_pDevice,                   // ������ ����̽�
		nullptr,                // *pFeatureLevel. ������ ���ķ����� �־���. � ���� ������ �����Ǵ��� �� �ʿ� ������ �׳� nullptr.
		&m_pContext            // ������ ����̽� ���ؽ�Ʈ
	));

	// ���� ü�� ���ο� ��� �ִ� �ؽ��� �ڿ�(back buffer)�� ���� ������ ����.
	Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer;
	GFX_THROW_INFO(m_pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
	GFX_THROW_INFO(m_pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &m_pTarget));

	// depth stencil state ����.
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSState;
	GFX_THROW_INFO(m_pDevice->CreateDepthStencilState(&dsDesc, &pDSState));

	// ��� ���ձ⿡ depth state ����.
	m_pContext->OMSetDepthStencilState(pDSState.Get(), 1u);

	// ���� ���ٽǿ� �ؽ��� ����.
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = screenWidth;                // �ؽ��� ũ��� ���� ü���� ������ ���ۿ� ������.
	descDepth.Height = screenHeight;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1u;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	GFX_THROW_INFO(m_pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil));

	// ���� ���ٽ� �ؽ��Ŀ� ���� �� ����.
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0u;
	GFX_THROW_INFO(m_pDevice->CreateDepthStencilView(
		pDepthStencil.Get(), &descDSV, &m_pDSV
	));

	// ��� ���ձ⿡ ���� Ÿ�ٰ� ���� ���ٽ� �� ����.
	m_pContext->OMSetRenderTargets(1u, m_pTarget.GetAddressOf(), m_pDSV.Get());

	// ����Ʈ ����
	D3D11_VIEWPORT vp;
	vp.Width = screenWidth;
	vp.Height = screenHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	// ����Ʈ���� rasterizer stage�� ���´�.
	m_pContext->RSSetViewports(1u, &vp); // 1���� ����Ʈ�� ����

	// ���� ��� ����� ���� ������ ����.
	float fieldOfView, screenAspect;
	fieldOfView = 3.141592654f / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;

	// ���� ����� ����.
	m_projectionMatrix = dx::XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	// ���� ��ȯ ����� ���� ��ķ� �������.
	m_worldMatrix = dx::XMMatrixIdentity();

	// ���� ���� ���.
	m_orthoMatrix = dx::XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);

	return true;
}

// ������ ���� ��� �ܰ踦 �ǹ��ϴ� �Լ�. ������ ���� ó���� �� �͵��� ��� ����.(���� ü�� Present)
void D3DGraphics::EndFrame()
{
	HRESULT hr;

	if (m_vsync_enabled)
	{
		if (FAILED(hr = m_pSwap->Present(1u, 0u)))
		{
			if (hr == DXGI_ERROR_DEVICE_REMOVED) // DXGI_ERROR_DEVICE_REMOVED ������ ��쿡 ���� ó������.
			{
				throw GFX_DEVICE_REMOVED_EXCEPT(m_pDevice->GetDeviceRemovedReason());
			}
			else
			{
				throw GFX_EXCEPT(hr);
			}
		}
	}
	else
	{
		if (FAILED(hr = m_pSwap->Present(0u, 0u)))
		{
			if (hr == DXGI_ERROR_DEVICE_REMOVED) // DXGI_ERROR_DEVICE_REMOVED ������ ��쿡 ���� ó������.
			{
				throw GFX_DEVICE_REMOVED_EXCEPT(m_pDevice->GetDeviceRemovedReason());
			}
			else
			{
				throw GFX_EXCEPT(hr);
			}
		}
	}
}

// �� �������� �׸��� ���� ȭ���� ����. ���� ���۸� �ʱ�ȭ���ִ� �Լ�.
void D3DGraphics::ClearBuffer(float red, float green, float blue, float alpha) noexcept
{
	const float color[] = { red,green,blue,alpha };
	m_pContext->ClearRenderTargetView(m_pTarget.Get(), color); // ���� Ÿ�� �並 �ʱ�ȭ.
	m_pContext->ClearDepthStencilView(m_pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

#pragma region Exception
D3DGraphics::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs) noexcept
	:
	Exception(line, file),
	hr(hr)
{
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	// �������� ���� ���ڰ� �ִ� ��� ��������.
	if (!info.empty())
	{
		info.pop_back();
	}
}

// what() : ����� ���� ���ڿ��� ������ �����ϴ� �Լ�. std::exception���� ���� �������̵�.
const char* D3DGraphics::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;

	if (!info.empty()) // ����� ���â�� �߰����� ���� ���� ���Խ��Ѽ� �Ѱ��ֵ��� ��.
	{
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	}

	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

// ���� Ÿ�� ���ڿ��� �������ִ� �Լ�.
const char* D3DGraphics::HrException::GetType() const noexcept
{
	return "Custom Graphics Exception";
}

// ���� �ڵ�(HRESULT)�� �������ִ� �Լ�.
HRESULT D3DGraphics::HrException::GetErrorCode() const noexcept
{
	return hr;
}

// dxerr ���̺귯���� ���� ���� ���ڿ��� �������ִ� �Լ�.
std::string D3DGraphics::HrException::GetErrorString() const noexcept
{
	return DXGetErrorStringA(hr);
}

// dxerr ���̺귯���� ���� ������ ���� ���� ���ڿ��� �������ִ� �Լ�.
std::string D3DGraphics::HrException::GetErrorDescription() const noexcept
{
	char buf[512];
	DXGetErrorDescriptionA(hr, buf, sizeof(buf));
	return buf;
}

// ����� ��� â ���� ���� ���ڿ��� �����ϴ� �Լ�.
std::string D3DGraphics::HrException::GetErrorInfo() const noexcept
{
	return info;
}

// ���� Ÿ�� ���ڿ��� �������ִ� �Լ�.
const char* D3DGraphics::DeviceRemovedException::GetType() const noexcept
{
	return "Custom Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}

D3DGraphics::InfoException::InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept
	:
	Exception(line, file)
{
	// join all info messages with newlines into single string
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	// remove final newline if exists
	if (!info.empty())
	{
		info.pop_back();
	}
}

// what() : ����� ���� ���ڿ��� ������ �����ϴ� �Լ�. std::exception���� ���� �������̵�.
const char* D3DGraphics::InfoException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

// ���� Ÿ�� ���ڿ��� �������ִ� �Լ�.
const char* D3DGraphics::InfoException::GetType() const noexcept
{
	return "Custom Graphics Info Exception";
}

// ����� ��� â ���� ���� ���ڿ��� �����ϴ� �Լ�.
std::string D3DGraphics::InfoException::GetErrorInfo() const noexcept
{
	return info;
}
#pragma endregion