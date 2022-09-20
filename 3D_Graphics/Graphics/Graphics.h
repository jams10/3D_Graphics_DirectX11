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
class LightShader;
class TextureShader;
class TranslateShader;
class DebugWindow;
class RenderToTexture;
class Light;
class Bitmap;
class DXSound;
class Frustum;
class ModelList;

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
	void RenderToTextureFunc();
	void RenderScene();
	
private:
	float accumulatedTime;
	float translationX;
	float translationY;
	bool bSineX;
	bool bSineY;

private:
	D3DGraphics* m_pD3D;
	D2DGraphics* m_pD2D;
	Camera* m_pCamera;
	Camera* m_pFixedCamera;
	Model* m_pModel;
	LightShader* m_pLightShader;
	TextureShader* m_pTextureShader;
	TranslateShader* m_pTranslateShader;
	DebugWindow* m_pDebugWindow;
	RenderToTexture* m_pRenderToTexture;
	Light* m_pLight;
	Bitmap* m_pBitmap;
	Frustum* m_pFrustum;
	ModelList* m_pModelList;
};