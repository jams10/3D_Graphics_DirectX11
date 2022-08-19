#include "Graphics.h"
#include <Graphics/D3DGraphics.h>
#include <Graphics/D2DGraphics.h>
#include <Shaders/TextureShader.h>
#include <Shaders/LightShader.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/CustomException.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <Objects/Model.h>
#include <Objects/Camera.h>
#include <Objects/Light.h>
#include <Objects/Bitmap.h>
#include <Objects/ModelList.h>
#include <Utils/Frustum.h>
#include <Input/Keyboard.h>
#include <Sound/DXSound.h>
#include <imgui/imgui.h>
#include <stdexcept>
#include <sstream>

#define SAFE_RELEASE(p) if(p){delete p; p=nullptr;}

Graphics::Graphics()
{
    m_pD3D = nullptr;
    m_pD2D = nullptr;
    m_pCamera = nullptr;
    m_pModel = nullptr;
    m_pLightShader = nullptr;
    m_pBitmap = nullptr;
    m_pFrustum = nullptr;
    m_pModelList = nullptr;
}

Graphics::~Graphics()
{
}

bool Graphics::Initialize(int screenWidth, int screenHeight, HWND Wnd)
{
    m_pD3D = new D3DGraphics();
    m_pD3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, Wnd, SCREEN_DEPTH, SCREEN_NEAR);

    m_pD2D = new D2DGraphics(*m_pD3D);
    m_pD2D->Initialize(*m_pD3D);

    m_pModel = new Model();
    m_pModel->Initialize(*m_pD3D, "Resources\\Models\\Sphere.model", "Resources\\Images\\seafloor.png");

    m_pCamera = new Camera();
    m_pFixedCamera = new Camera();

    m_pLight = new Light();

    m_pTextureShader = new TextureShader();
    m_pTextureShader->Initialize(*m_pD3D);

    m_pLightShader = new LightShader();
    m_pLightShader->Initialize(*m_pD3D);

    m_pBitmap = new Bitmap();
    m_pBitmap->Initialize(*m_pD3D, screenWidth, screenHeight, "Resources\\Images\\seafloor.png", 256, 256);

    m_pModelList = new ModelList();
    m_pModelList->Initialize(50);

    m_pFrustum = new Frustum();

    return true;
}

void Graphics::Shutdown()
{
    SAFE_RELEASE(m_pFrustum)
    SAFE_RELEASE(m_pModelList)
    SAFE_RELEASE(m_pBitmap)
    SAFE_RELEASE(m_pLightShader)
    SAFE_RELEASE(m_pTextureShader)
    SAFE_RELEASE(m_pLight)
    SAFE_RELEASE(m_pCamera)
    SAFE_RELEASE(m_pModel)
    SAFE_RELEASE(m_pD2D)
    SAFE_RELEASE(m_pD3D)
}

bool Graphics::Frame(DXSound* pSound, int fps, int cpuUsage)
{
    if (!Render(pSound, fps, cpuUsage))
    {
        return false;
    }

    return true;
}

bool Graphics::Render(DXSound* pSound, int fps, int cpuUsage)
{
    m_pD3D->BeginFrame(0.5f, 0.5f, 0.5f, 1.f);
    m_pD2D->BeginFrame();
    
    // FPS, CPU 사용량 표시.
    std::wstringstream wss_perf;
    wss_perf << L"FPS : " << fps << L"\nCPU : " << cpuUsage << L"%";
    m_pD2D->DrawBox(1280 - 100, 0, 1280, 100);
    m_pD2D->WriteText(wss_perf.str(), 1280 - 100, 0, 1280, 100);

    // 월드, 뷰, 원근 투영, 정사영 투영 행렬을 얻어옴.
    dx::XMMATRIX worldFor2D = dx::XMMatrixIdentity();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX viewFor2D = m_pFixedCamera->GetViewMatrix();
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();
    dx::XMMATRIX orth = m_pD3D->GetOrthMatrix();

#pragma region Frustum Culing Test

    float posX = 0, posY = 0, posZ = 0, radius = 0;
    dx::XMFLOAT4 color;
    dx::XMMATRIX world;
    int modelCount = 0, renderCount = 0; // modelCount : 생성하고자 하는 모델의 개수, renderCount : 컬링을 통과해 실제로 그려지는 모델의 개수.
    // 절두체 생성.
    m_pFrustum->ConstructFrustum(SCREEN_DEPTH, projection, view);

    modelCount = m_pModelList->GetModelCount();

    for (int i = 0; i < modelCount; ++i)
    {
        // 랜덤으로 모델의 색상과 위치를 생성해 얻어옴.
        m_pModelList->GetData(i, posX, posY, posZ, color);

        // 구체 모델의 반지름은 1로 설정했음.
        radius = 1.0f;

        // 구체에 대해 절두체 컬링을 수행 했을 때 통과한 경우.
        if (m_pFrustum->CheckSphere(posX, posY, posZ, radius))
        {
            // 물체의 위치를 가지고 월드 변환 행렬을 만들어 물체가 해당 월드 공간 위치에 그려질 수 있도록 함.
            world = DirectX::XMMatrixTranslation(posX, posY, posZ);

            // 모델을 구성하는 정점과 인덱스 버퍼를 파이프라인에 바인딩 함.
            m_pModel->Bind(*m_pD3D);

            // 3D 모델을 light shader로 그려줌.
            m_pLightShader->Bind(*m_pD3D, m_pModel->GetIndexCount(), world, view, projection, m_pCamera->GetPosition(),
                m_pModel->GetTexture(), m_pLight->GetAmbientColor(), color, m_pLight->GetLightDirection(),
                m_pLight->GetSpecularColor(), m_pLight->GetSpecularPower());

            // 월드 행렬을 단위 행렬로 초기화.
            world = DirectX::XMMatrixIdentity();

            // 그려준 물체의 개수를 증가시켜줌.
            renderCount++;
        }
    }

    // 컬링을 통과해 화면에 그려지는 모델의 개수를 표시해줌.
    std::wstringstream wss_culling;
    wss_culling << L"Rendered Models Count : " << renderCount << L" | " << modelCount;
    m_pD2D->DrawBox(1280 - 400, 0, 1280 - 100, 100);
    m_pD2D->WriteText(wss_culling.str(), 1280 - 400, 0, 1280 - 100, 100);

#pragma endregion

#pragma region 2D Rendering
    //// 2D 렌더링을 위해 깊이 버퍼를 꺼줌.
    //m_pD3D->TurnZBufferOff();

    //// 2D 이미지 렌더링을 위한 bitmap 객체가 가진 정점과 인덱스 버퍼를 파이프라인에 바인딩.
    //m_pBitmap->Bind(*m_pD3D, (1280 / 2) * -1 + (256 / 2), 720 / 2 - (256/ 2));

    //// 2D 이미지는 texture shader로 그려줌.
    //m_pTextureShader->Bind(*m_pD3D, m_pBitmap->GetIndexCount(), worldFor2D, viewFor2D, orth, m_pBitmap->GetTexture());

    //m_pD3D->TurnZBufferOn();
#pragma endregion

#pragma region UI
    m_pModel->SpawnControlWindow();
    m_pCamera->SpawnControlWindow();
    m_pLight->SpawnControlWindow();
    pSound->SpawnControlWindow();
#pragma endregion

    m_pD2D->EndFrame();
    // 렌더링된 씬을 화면에 표시.
    m_pD3D->EndFrame();

    return true;
}
