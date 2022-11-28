#pragma once

#include<Graphics/D3DGraphics.h>
#include<DirectXMath.h>

class Texture;

class ParticleSystem
{
private:
	struct ParticleType
	{
		float positionX, positionY, positionZ;
		float red, green, blue;
		float velocity;
		bool active;
	};

	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
		DirectX::XMFLOAT4 color;
	};

public:
	ParticleSystem();
	~ParticleSystem();

	void Initialize(D3DGraphics& gfx, std::string textureFilePath);
	void Shutdown();
	void Frame(float deltaTime, D3DGraphics& gfx);
	void Bind(D3DGraphics& gfx);

	unsigned int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

private:
	void InitializeParticleSystem();
	void ShutdownParticleSystem();

	void InitializeBuffers(D3DGraphics& gfx);
	void UpdateBuffers(D3DGraphics& gfx);
	void BindBuffers(D3DGraphics& gfx);

	void LoadTexture(D3DGraphics& gfx, std::string filePath);
	void ReleaseTexture();

	void EmitParticles(float deltaTime);
	void UpdateParticles(float deltaTime);
	void KillParticles();

private:
	float m_particleDeviationX;
	float m_particleDeviationY;
	float m_particleDeviationZ;
	float m_particleVelocity;
	float m_particleVelocityVariation;
	float m_particleSize;
	float m_particlesPerSecond;
	int m_maxParticles;

	int m_currentParticleCount;
	float m_accumulatedTime;

	Texture* m_Texture;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;

	unsigned int m_vertexCount;
	unsigned int m_indexCount;
	VertexType* m_pVertices;

	ParticleType* m_particleList;
};