#pragma once

#include <Windows/WindowsHeaders.h>
#include <DirectXMath.h>

const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class D3DGraphics;
class D2DGraphics;
class Camera;
class Model;
class GlassShader;
class LightShader;
class TextureShader;
class DebugWindow;
class RenderToTexture;
class Light;
class Bitmap;
class DXSound;
class Frustum;

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
	void RenderScene();
	void RenderSceneToTexture();
	float refractionScale = 0.01f;
	
private:
	D3DGraphics* m_pD3D;
	D2DGraphics* m_pD2D;
	Camera* m_pCamera;
	Camera* m_pFixedCamera;
	Model* m_pModel;
	Model* m_pWindowModel;
	GlassShader* m_pGlassShader;
	LightShader* m_pLightShader;
	RenderToTexture* m_pRenderToTexture;
	TextureShader* m_pTextureShader;
	DebugWindow* m_pDebugWindow;
	Light* m_pLight;
	Bitmap* m_pBitmap;
	Frustum* m_pFrustum;
};