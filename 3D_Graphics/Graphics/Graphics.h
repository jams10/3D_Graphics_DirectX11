#pragma once

#include <Windows/WindowsHeaders.h>
#include <DirectXMath.h>

const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 100.0f;
const float SCREEN_NEAR = 1.0f;

class D3DGraphics;
class Camera;
class Model;
class Light;
class RenderToTexture;
class DepthShader;
class ShadowShader;
class LightShader;
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
	void RenderSceneToTexture();
	
private:
	D3DGraphics* m_pD3D;
	Camera* m_pCamera;
	Model* m_pCubeModel;
	Model* m_pGroundModel;
	Model* m_pSphereModel;
	Light* m_pLight;
	RenderToTexture* m_pRenderTexture;
	DepthShader* m_pDepthShader;
	ShadowShader* m_pShadowShader;
	LightShader* m_pLightShader;
};