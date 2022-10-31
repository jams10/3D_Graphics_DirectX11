#pragma once

#include <Windows/WindowsHeaders.h>
#include <DirectXMath.h>

const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class D3DGraphics;
class Camera;
class Model;
class LightShader;
class DepthShader;
class Light;
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
	void RenderScene();
	
private:
	D3DGraphics* m_pD3D;
	Camera* m_pCamera;
	Model* m_pFloorModel;
	Model* m_pBillboardModel;
	LightShader* m_pLightShader;
	DepthShader* m_pDepthShader;
	Light* m_pLight;
};