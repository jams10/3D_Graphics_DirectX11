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
class SpecularMapShader;
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
	bool Frame(DXSound* pSound, int fps, int cpuUsage);

	Camera* GetCamera() { return m_pCamera; }

private:
	bool Render(DXSound* pSound, int fps, int cpuUsage);

private:
	D3DGraphics* m_pD3D;
	D2DGraphics* m_pD2D;
	Camera* m_pCamera;
	Camera* m_pFixedCamera;
	Model* m_pModel;
	LightShader* m_pLightShader;
	SpecularMapShader* m_pSpecularMapShader;
	Light* m_pLight;
	Bitmap* m_pBitmap;
	Frustum* m_pFrustum;
	ModelList* m_pModelList;
};