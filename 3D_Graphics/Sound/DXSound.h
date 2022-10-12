#pragma once

#include <Windows/WindowsHeaders.h>
#include <ErrorHandle/CustomException.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/DirectSoundExceptionMacros.h>
#include <mmsystem.h>
#include <dsound.h>

class DXSound
{
#pragma region Exception
public:
	class Exception : public CustomException
	{
		using CustomException::CustomException; // CustomException ������ ���.
	};
	// HrException : HRESULT�� �Ѿ���� ������ ���� ���� ó�� Ŭ����.
	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {}) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
		std::string GetErrorDescription() const noexcept;
		std::string GetErrorInfo() const noexcept;
	private:
		HRESULT hr;
		std::string info;
	};
	// InfoException : HRESULT�� �������� �ʴ� �Լ����� ���� ó���� ���� ����� ���â�� �ִ� ������ �����ٰ� �Ѱ��ִ� Ŭ����.
	class InfoException : public Exception
	{
	public:
		InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		std::string GetErrorInfo() const noexcept;
	private:
		std::string info;
	};
	// DeviceRemovedException : HRESULT�� DXGI_ERROR_DEVICE_REMOVED�� ��� ���� ó������ ���� Ŭ����.
	class DeviceRemovedException : public HrException
	{
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override;
	private:
		std::string reason;
	};
#pragma endregion
private:
#ifndef NDEBUG
	DxgiInfoManager infoManager; // ����� ����� ���� DxgiInfoManager ��ü�� ��� �ְ� ��.
#endif
private:
	struct WaveHeaderType // .wav ���� ���� ��� ����
	{
		char chunkId[4];
		unsigned long chunkSize;
		char format[4];
		char subChunkId[4];
		unsigned long subChunkSize;
		unsigned short audioFormat;
		unsigned short numChannels;
		unsigned long sampleRate;
		unsigned long bytesPerSecond;
		unsigned short blockAlign;
		unsigned short bitsPerSample;
		char dataChunkId[4];
		unsigned long dataSize;
	};

public:
	DXSound();

	void Initialize(HWND hwnd);
	void Shutdown();

	void SpawnControlWindow();
private:
	void InitializeDirectSound(HWND hwnd);
	void ShutdownDirectSound();

	void LoadWaveFile(const char* filename, IDirectSoundBuffer8** secondaryBuffer, IDirectSound3DBuffer8** secondary3DBuffer);
	void ShutdownWaveFile(IDirectSoundBuffer8** secondaryBuffer, IDirectSound3DBuffer8** secondary3DBuffer);

	void PlayWaveFile();

private:
	int m_volume;

private:
	float positionX = -2.0f, positionY = 0.0f, positionZ = 0.0f;
	IDirectSound8* m_pDirectSound;
	IDirectSoundBuffer* m_pPrimaryBuffer;
	IDirectSoundBuffer8* m_pSecondaryBuffer;
	IDirectSound3DListener8* m_listener;
	IDirectSound3DBuffer8* m_secondary3DBuffer;
};