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
class RefractionShader;
class WaterShader;
class RenderToTexture;
class DebugWindow;
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
	void RenderRefractionTexture();
	void RenderReflectionTexture();
	void RenderScene();
	
private:
	float m_waterTranslation;
	float m_waterTranslationDelta = 0.001f;;
	float m_waterHeight;
	float m_reflectRefractScale = 0.01f;

private:
	D3DGraphics* m_pD3D;
	D2DGraphics* m_pD2D;
	Camera* m_pCamera;
	Camera* m_pFixedCamera;
	Model* m_pModelWall;
	Model* m_pModelGround;
	Model* m_pModelBath;
	Model* m_pModelWater;
	LightShader* m_pLightShader;
	TextureShader* m_pTextureShader;
	RefractionShader* m_pRefractionShader;
	WaterShader* m_pWaterShader;
	DebugWindow* m_pDebugWindow;
	RenderToTexture* m_pRefractionTexture;
	RenderToTexture* m_pReflectionTexture;
	Light* m_pLight;
	Bitmap* m_pBitmap;
	Frustum* m_pFrustum;
	ModelList* m_pModelList;
};