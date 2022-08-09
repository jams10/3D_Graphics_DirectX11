#include "D2DGraphics.h"
#include "D3DGraphics.h"
#include <dxerr/dxerr.h>
#include <sstream>
#include <ErrorHandle/D2DGraphicsExceptionMacros.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#define SAFE_RELEASE(p) if(p){p->Release(); p=nullptr;}

D2DGraphics::D2DGraphics(const D3DGraphics& D3DGraphics)
{
	HRESULT hr;

	// DWriteFactory ����.
	GFX_THROW_INFO(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&m_pWriteFactory));

	D2D1_FACTORY_OPTIONS option;
	option.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;

	// D2D1Factory ����.
	GFX_THROW_INFO(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &m_pFactory));

	IDXGIDevice* pDxgiDevice;
	GFX_THROW_INFO(D3DGraphics.GetDevice()->QueryInterface(&pDxgiDevice));

	// ID2D1Device ����.
	GFX_THROW_INFO(m_pFactory->CreateDevice(pDxgiDevice, &m_pDevice));
	//m_pFactory->CreateDevice(pDxgiDevice, &m_pDevice);

	// ID2D1DeviceContext ����.
	GFX_THROW_INFO(m_pDevice->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
		&m_pContext
	));

}

D2DGraphics::~D2DGraphics()
{
	SAFE_RELEASE(m_pTextFormat)
	SAFE_RELEASE(m_pBrush)
	SAFE_RELEASE(m_pBitmap)
	SAFE_RELEASE(m_pSurface)
	SAFE_RELEASE(m_pContext)
	SAFE_RELEASE(m_pDevice)
	SAFE_RELEASE(m_pWriteFactory)
}

void D2DGraphics::Initialize(const D3DGraphics& D3DGraphics)
{
	if (D3DGraphics.GetSwapChain() == nullptr) return;

	HRESULT hr;

	// ���� ü�ο��� DXGI Surface�� ����.
	GFX_THROW_INFO(D3DGraphics.GetSwapChain()->GetBuffer(0, __uuidof(IDXGISurface), (void**)&m_pSurface));

	// D2D ��Ʈ�� �Ӽ� ����ü ����.
	D2D1_BITMAP_PROPERTIES1 bp;
	UINT dpi = D3DGraphics.GetDPI();
	bp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
	bp.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	bp.dpiX = dpi;
	bp.dpiY = dpi;
	bp.colorContext = NULL;

	// DXGI Surface�� ���� D2D ��Ʈ�� ����.
	GFX_THROW_INFO(m_pContext->CreateBitmapFromDxgiSurface(m_pSurface, &bp, &m_pBitmap));

	// D2D�� ���� Ÿ���� DXGI Surface�� ���� ������ D2D ��Ʈ������ ����.
	m_pContext->SetTarget(m_pBitmap);

	// �귯�� ����.
	GFX_THROW_INFO(m_pContext->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0, 1), &m_pBrush));

	// �ؽ�Ʈ ���� ����.
	m_pWriteFactory->CreateTextFormat
	(
		L"����ü", NULL,
		DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		15, L"ko", &m_pTextFormat
	);
}

void D2DGraphics::Release()
{
	if (m_pContext) m_pContext->SetTarget(NULL);
	else return;

	SAFE_RELEASE(m_pBrush)
	SAFE_RELEASE(m_pTextFormat)
	SAFE_RELEASE(m_pBitmap)
	SAFE_RELEASE(m_pSurface)
}

void D2DGraphics::BeginFrame()
{
	m_pContext->BeginDraw();
}

void D2DGraphics::EndFrame()
{
	m_pContext->EndDraw();
}

void D2DGraphics::DrawBox(const int& left, const int& top, const int& right, const int& bottom)
{
	D2D1_RECT_F r = { left, top, right, bottom };
	D2D1_ROUNDED_RECT rect = { r, 10.f, 10.f };
	m_pBrush->SetColor(D2D1_COLOR_F{ 0.0f, 255.0f,236.0f,206.0f });
	m_pBrush->SetOpacity(0.3f);
	m_pContext->FillRoundedRectangle(rect, m_pBrush);
}

void D2DGraphics::WriteText(std::wstring s)
{
	m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	m_pBrush->SetColor(D2D1_COLOR_F{ 0.0f, 255.0f,236.0f,206.0f });
	m_pBrush->SetOpacity(1.0f);	
	D2D1_RECT_F r = { 0, 720 - 256, 256, 720 };
	m_pContext->DrawTextW(s.c_str(), s.size(), m_pTextFormat, r, m_pBrush);
}

#pragma region Exception
D2DGraphics::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs) noexcept
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
const char* D2DGraphics::HrException::what() const noexcept
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
const char* D2DGraphics::HrException::GetType() const noexcept
{
	return "Custom Graphics Exception";
}

// ���� �ڵ�(HRESULT)�� �������ִ� �Լ�.
HRESULT D2DGraphics::HrException::GetErrorCode() const noexcept
{
	return hr;
}

// dxerr ���̺귯���� ���� ���� ���ڿ��� �������ִ� �Լ�.
std::string D2DGraphics::HrException::GetErrorString() const noexcept
{
	return DXGetErrorStringA(hr);
}

// dxerr ���̺귯���� ���� ������ ���� ���� ���ڿ��� �������ִ� �Լ�.
std::string D2DGraphics::HrException::GetErrorDescription() const noexcept
{
	char buf[512];
	DXGetErrorDescriptionA(hr, buf, sizeof(buf));
	return buf;
}

// ����� ��� â ���� ���� ���ڿ��� �����ϴ� �Լ�.
std::string D2DGraphics::HrException::GetErrorInfo() const noexcept
{
	return info;
}

// ���� Ÿ�� ���ڿ��� �������ִ� �Լ�.
const char* D2DGraphics::DeviceRemovedException::GetType() const noexcept
{
	return "Custom Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}

D2DGraphics::InfoException::InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept
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
const char* D2DGraphics::InfoException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

// ���� Ÿ�� ���ڿ��� �������ִ� �Լ�.
const char* D2DGraphics::InfoException::GetType() const noexcept
{
	return "Custom Graphics Info Exception";
}

// ����� ��� â ���� ���� ���ڿ��� �����ϴ� �Լ�.
std::string D2DGraphics::InfoException::GetErrorInfo() const noexcept
{
	return info;
}
#pragma endregion