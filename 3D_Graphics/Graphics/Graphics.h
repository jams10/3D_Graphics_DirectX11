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

class Graphics
{
public:
	Graphics();
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame(class Keyboard* kbd, float dt);

private:
	bool Render();

private:
	D3DGraphics* m_pD3D;
	Camera* m_pCamera;
	Model* m_pModel;
	TextureShader* m_pTextureShader;
};