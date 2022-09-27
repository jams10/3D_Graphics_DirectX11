#include <Graphics/D3DGraphics.h>
#include <dxerr/dxerr.h>
#include <sstream>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>
#include <winuser.h>

#include <d3dcompiler.h>

#pragma comment(lib,"d3d11.lib")        // Direct3D 함수들이 정의된 라이브러리를 링크해줌.
#pragma comment(lib, "D3DCompiler.lib") // 셰이더를 런타임에 컴파일 해줄 때 사용할 수 있지만, 우리는 셰이더를 불러오는 함수를 사용하기 위해 연결해줬음. 

D3DGraphics::D3DGraphics()
{
	imguiEnabled = true;
}

D3DGraphics::~D3DGraphics()
{
	ImGui_ImplDX11_Shutdown();
}

bool D3DGraphics::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hWnd, float screenDepth, float screenNear)
{
	// 수직 동기화 여부
	m_vsync_enabled = vsync;

	// 스왑 체인의 설정 정보를 담은 구조체
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = 0;						   // backbuffer 너비
	sd.BufferDesc.Height = 0;						   // backbuffer 높이
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // 픽셀 형식
	sd.BufferDesc.RefreshRate.Numerator = 0;           // hz의 refresh rate 분자
	sd.BufferDesc.RefreshRate.Denominator = 0;         // hz의 refresh rate 분모
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;                 // 이미지가 주어진 모니터의 해상도에 맞추기 위해 어떻게 늘여지는지에 대한 방법을 가리킴.
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; // surface에 이미지를 생성하기 위해 raster가 사용하는 방법을 가리킴.
	sd.SampleDesc.Count = 1;                           // 다중 표본화 개수
	sd.SampleDesc.Quality = 0;                         // 다중 표본화 품질
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // 버퍼를 렌더타겟 용도로 사용.
	sd.BufferCount = 1;                                // 1개의 back buffer를 사용. 그러면 front/back 으로 총 2개의 버퍼를 사용.
	sd.OutputWindow = hWnd;                            // 출력 윈도우를 지정.
	sd.Windowed = TRUE;                                // 창모드를 적용.
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;          // IDXGISwapChain1::Present1 호출 후에 디스플레이 화면에 있는 픽셀들을 어떻게 처리할 것인가? 여기서는 그냥 버림.
	sd.Flags = 0;                                      // 추가적인 플래그들.

	UINT swapCreateFlags = 0u;
#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;        // 디버그 모드인 경우에 디바이스 생성 플래그를 D3D11_CREATE_DEVICE_DEBUG로 설정해줌.
#endif
	swapCreateFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT; // Direct2D와의 호환성을 위해 D3D11_CREATE_DEVICE_BGRA_SUPPORT 플래그 추가. 

	HRESULT hr; // API 함수가 리턴하는 HRESULT를 받아와 저장해 두기 위함.

	// device와 swap chain, device context를 생성함.
	GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
		nullptr,                    // IDXGIAdapter를 nullptr로 설정해, 기본 어댑터를 선택해줌.
		D3D_DRIVER_TYPE_HARDWARE, // 생성할 드라이버 타입.
		nullptr,                    // 소프트웨어 래스터라이저를 구현하는 DLL에 대한 핸들.
		swapCreateFlags,                   // 디버그 모드의 경우 D3D11_CREATE_DEVICE_DEBUG를 설정해, 디버깅 분석 정보를 얻을 수 있음.
		nullptr,               // D3D_FEATURE_LEVEL. D3D 장치 피쳐 레벨. nullptr로 주면, 기본 6가지 피쳐 레벨을 사용.
		0,                      // pFeatureLevels에 있는 원소들의 개수.
		D3D11_SDK_VERSION,        // SDK_VERSION. 피쳐 레벨과는 다름.
		&sd,                   // 스왑 체인 서술자
		&m_pSwap,                   // 생성된 스왑 체인
		&m_pDevice,                   // 생성된 디바이스
		nullptr,                // *pFeatureLevel. 가능한 피쳐레벨을 넣어줌. 어떤 피쳐 레벨이 지원되는지 알 필요 없으면 그냥 nullptr.
		&m_pContext            // 생성된 디바이스 컨텍스트
	));

	// 스왑 체인 내부에 들고 있는 텍스쳐 자원(back buffer)에 대한 접근을 얻어옴.
	Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer;
	GFX_THROW_INFO(m_pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
	GFX_THROW_INFO(m_pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &m_pTarget));

	// depth stencil state 생성.
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	GFX_THROW_INFO(m_pDevice->CreateDepthStencilState(&dsDesc, &m_pDepthStencilState));

	// 깊이 값을 쓰지 않는 depth stencil state 생성.
	D3D11_DEPTH_STENCIL_DESC disableDesc = {};
	disableDesc.DepthEnable = FALSE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	GFX_THROW_INFO(m_pDevice->CreateDepthStencilState(&disableDesc, &m_pDepthDisabledStencilState));

	// 출력 병합기에 depth state 묶기.
	m_pContext->OMSetDepthStencilState(m_pDepthStencilState.Get(), 1u);

	// 알파 블렌딩을 위한 blend state 생성.
	D3D11_BLEND_DESC blendStateDesc = {};
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	m_pDevice->CreateBlendState(&blendStateDesc, &m_pAlphaEnabledBlendingState);

	// BlendEnable를 False로 설정해 알파 블렌딩을 끈 상태의 blend state를 생성함.
	m_pDevice->CreateBlendState(&blendStateDesc, &m_pAlphaDisabledBlendingState);

	// 깊이 스텐실용 텍스쳐 생성.
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = screenWidth;                // 텍스쳐 크기는 스왑 체인의 프레임 버퍼와 맞춰줌.
	descDepth.Height = screenHeight;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1u;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	GFX_THROW_INFO(m_pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil));

	// 깊이 스텐실 텍스쳐에 대한 뷰 생성.
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0u;
	GFX_THROW_INFO(m_pDevice->CreateDepthStencilView(
		pDepthStencil.Get(), &descDSV, &m_pDSV
	));

	// 출력 병합기에 렌더 타겟과 깊이 스텐실 뷰 묶기.
	m_pContext->OMSetRenderTargets(1u, m_pTarget.GetAddressOf(), m_pDSV.Get());

	// 뷰포트 설정
	D3D11_VIEWPORT vp;
	vp.Width = screenWidth;
	vp.Height = screenHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	// 뷰포트들을 rasterizer stage에 묶는다.
	m_pContext->RSSetViewports(1u, &vp); // 1개의 뷰포트를 세팅

	// 투영 행렬 계산을 위한 값들을 세팅.
	float fieldOfView, screenAspect;
	fieldOfView = 3.141592654f / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;

	// DPI 얻어오기.
	wndDpi = GetDpiForWindow(hWnd);

	// 투영 행렬을 생성.
	m_projectionMatrix = dx::XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	// 월드 변환 행렬은 단위 행렬로 만들어줌.
	m_worldMatrix = dx::XMMatrixIdentity();

	// 직교 투영 행렬.
	m_orthoMatrix = dx::XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);
	//m_orthoMatrix = dx::XMMatrixOrthographicOffCenterLH(0.f, (float)screenWidth, (float)screenHeight, 0.f, screenNear, screenDepth);

	// imgui dx11 구현 초기화.
	ImGui_ImplDX11_Init(m_pDevice.Get(), m_pContext.Get());

	return true;
}

// 새 프레임을 그리기 위해 화면을 정리. 각종 버퍼를 초기화해주는 함수.
void D3DGraphics::BeginFrame(float red, float green, float blue, float alpha) noexcept
{
	// 출력 병합기에 렌더 타겟과 깊이 스텐실 뷰 묶기.
	m_pContext->OMSetRenderTargets(1u, m_pTarget.GetAddressOf(), m_pDSV.Get());

	// Imgui를 사용한 UI들을 그리기 위한 새 프레임을 생성.
	if (imguiEnabled)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	const float color[] = { red,green,blue,alpha };
	m_pContext->ClearRenderTargetView(m_pTarget.Get(), color); // 렌더 타겟 뷰를 초기화.
	m_pContext->ClearDepthStencilView(m_pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

// 프레임 최종 결과 단계를 의미하는 함수. 프레임 끝에 처리해 줄 것들을 담고 있음.(스왑 체인 Present)
void D3DGraphics::EndFrame()
{
	HRESULT hr;

	// Imgui를 사용한 UI들을 화면에 렌더링.
	if (imguiEnabled)
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		ImGui::UpdatePlatformWindows(); // Imgui 창을 업데이트 하고 화면에 보여줌.
		ImGui::RenderPlatformWindowsDefault();
	}
	if (m_vsync_enabled)
	{
		if (FAILED(hr = m_pSwap->Present(1u, 0u)))
		{
			if (hr == DXGI_ERROR_DEVICE_REMOVED) // DXGI_ERROR_DEVICE_REMOVED 에러인 경우에 따로 처리해줌.
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
			if (hr == DXGI_ERROR_DEVICE_REMOVED) // DXGI_ERROR_DEVICE_REMOVED 에러인 경우에 따로 처리해줌.
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

void D3DGraphics::SetBackBufferRenderTarget()
{
	m_pContext->OMSetRenderTargets(1, m_pTarget.GetAddressOf(), m_pDSV.Get());
}

void D3DGraphics::TurnZBufferOn()
{
	m_pContext->OMSetDepthStencilState(m_pDepthStencilState.Get(), 1);
}

void D3DGraphics::TurnZBufferOff()
{
	m_pContext->OMSetDepthStencilState(m_pDepthDisabledStencilState.Get(), 1);
}

void D3DGraphics::TurnOnAlphaBlending()
{
	float blendFactor[4];

	// Setup the blend factor.
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;

	// Turn on the alpha blending.
	m_pContext->OMSetBlendState(m_pAlphaEnabledBlendingState.Get(), blendFactor, 0xffffffff);

	return;
}

void D3DGraphics::TurnOffAlphaBlending()
{
	float blendFactor[4];

	// Setup the blend factor.
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;

	// Turn on the alpha blending.
	m_pContext->OMSetBlendState(m_pAlphaDisabledBlendingState.Get(), blendFactor, 0xffffffff);

	return;
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
	// 마지막에 개행 문자가 있는 경우 삭제해줌.
	if (!info.empty())
	{
		info.pop_back();
	}
}

// what() : 출력할 에러 문자열을 생성해 리턴하는 함수. std::exception으로 부터 오버라이딩.
const char* D3DGraphics::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;

	if (!info.empty()) // 디버그 출력창의 추가적인 정보 또한 포함시켜서 넘겨주도록 함.
	{
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	}

	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

// 예외 타입 문자열을 리턴해주는 함수.
const char* D3DGraphics::HrException::GetType() const noexcept
{
	return "Custom Graphics Exception";
}

// 에러 코드(HRESULT)를 리턴해주는 함수.
HRESULT D3DGraphics::HrException::GetErrorCode() const noexcept
{
	return hr;
}

// dxerr 라이브러리를 통해 에러 문자열을 리턴해주는 함수.
std::string D3DGraphics::HrException::GetErrorString() const noexcept
{
	return DXGetErrorStringA(hr);
}

// dxerr 라이브러리를 통해 에러에 관한 설명 문자열을 리턴해주는 함수.
std::string D3DGraphics::HrException::GetErrorDescription() const noexcept
{
	char buf[512];
	DXGetErrorDescriptionA(hr, buf, sizeof(buf));
	return buf;
}

// 디버그 출력 창 에러 정보 문자열을 리턴하는 함수.
std::string D3DGraphics::HrException::GetErrorInfo() const noexcept
{
	return info;
}

// 예외 타입 문자열을 리턴해주는 함수.
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

// what() : 출력할 에러 문자열을 생성해 리턴하는 함수. std::exception으로 부터 오버라이딩.
const char* D3DGraphics::InfoException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

// 예외 타입 문자열을 리턴해주는 함수.
const char* D3DGraphics::InfoException::GetType() const noexcept
{
	return "Custom Graphics Info Exception";
}

// 디버그 출력 창 에러 정보 문자열을 리턴하는 함수.
std::string D3DGraphics::InfoException::GetErrorInfo() const noexcept
{
	return info;
}
#pragma endregion
