#pragma once

#include <Windows/WindowsHeaders.h>
#include <DirectXMath.h>

const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class D3DGraphics;
class Camera;
class Model;
class TextureShader;
class HorizontalBlurShader;
class VerticalBlurShader;
class RenderToTexture;
class OrthoWindow;
class DXSound;

class Graphics
{
public:
	Graphics();
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame(DXSound* pSound, int fps, int cpuUsage, float dt);

	Camera* GetCamera() { return m_pCamera; }

private:
	bool Render(DXSound* pSound, int fps, int cpuUsage, float dt);
	void RenderSceneToTexture(float dt);
	void DownSampleTexture();
	void RenderHorizontalBlurToTexture();
	void RenderVerticalBlurToTexture();
	void UpSampleTexture();
	void Render2DTextureScene();
	
private:
	D3DGraphics* m_pD3D;
	Camera* m_pCamera;
	Camera* m_pFixedCamera;
	Model* m_pModel;
	OrthoWindow* m_pSmallWindow;
	OrthoWindow* m_pFullscreenWindow;
	TextureShader* m_pTextureShader;
	HorizontalBlurShader* m_pHorizontalBlurShader;
	VerticalBlurShader* m_pVerticalBlurShader;
	RenderToTexture* m_pRenderToTexture;
	RenderToTexture* m_pDownSampleTexture;
	RenderToTexture* m_pHorizontalBlurTexture;
	RenderToTexture* m_pVerticalBlurTexture;
	RenderToTexture* m_pUpSampleTexture;
};