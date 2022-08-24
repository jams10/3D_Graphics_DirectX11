#include "DXSound.h"
#include <assert.h>
#include <dxerr/dxerr.h>
#include <sstream>
#include <imgui/imgui.h>
#include <stdio.h>

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

#define SAFE_RELEASE(p) if(p){p->Release(); p=nullptr;}

DXSound::DXSound()
{
	m_pDirectSound = nullptr;
	m_pPrimaryBuffer = nullptr;
	m_pSecondaryBuffer = nullptr;
	m_volume = DSBVOLUME_MAX;
}

void DXSound::Initialize(HWND hwnd)
{
	// Directsound�� primary sound buffer�� �ʱ�ȭ ��.
	InitializeDirectSound(hwnd);

	// secondary buffer�� wav ����� ������ �ҷ���.
	LoadWaveFile("Sound/Ding.wav", &m_pSecondaryBuffer);

	// secondary buffer�� �ҷ��� wav ����� ������ �����.
	// PlayWaveFile();
}


void DXSound::Shutdown()
{
	// ���� ���۸� ����
	ShutdownWaveFile(&m_pSecondaryBuffer);

	// Directsound API�� �ݱ�.
	ShutdownDirectSound();

	return;
}


void DXSound::InitializeDirectSound(HWND hwnd)
{
	HRESULT hr;
	DSBUFFERDESC bufferDesc;
	WAVEFORMATEX waveFormat;

	// �⺻ ���� ��ġ�� ���� Directsound �������̽� �����͸� �ʱ�ȭ.
	DXSOUND_THROW_INFO(DirectSoundCreate8(NULL, &m_pDirectSound, NULL));

	// ���� ����(cooperative level)�� priority�� ������ ���� ���� ������ ������ ������ �� �ֵ��� ��.
	DXSOUND_THROW_INFO(m_pDirectSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY));

	// ���� ���� �����ڸ� ����.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = NULL;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// �⺻ ���� ��ġ�� ���� ���� ���� ������ ��Ʈ���� ����.
	DXSOUND_THROW_INFO(m_pDirectSound->CreateSoundBuffer(&bufferDesc, &m_pPrimaryBuffer, NULL));

	// ���� ���� ������ ������ ������.
	// �츮�� ������ 16-bit ���׷����� �ʴ� 44,100���� ���÷� �����Ǵ� .WAV ���� ������ ����� ����.(CD ����� ����).
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	// ������ �������� wav ���� �������� ���� ���۸� ����.
	DXSOUND_THROW_INFO(m_pPrimaryBuffer->SetFormat(&waveFormat));

}


void DXSound::ShutdownDirectSound()
{
	// ���� ���� ���� �����͸� ����.
	SAFE_RELEASE(m_pPrimaryBuffer)

	// Directsound �������̽� �����͸� ����.
	SAFE_RELEASE(m_pDirectSound)
}


void DXSound::LoadWaveFile(const char* filename, IDirectSoundBuffer8** secondaryBuffer)
{
	int error;
	FILE* filePtr;
	unsigned int count;
	WaveHeaderType waveFileHeader;
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	HRESULT hr;
	IDirectSoundBuffer* tempBuffer;
	unsigned char* waveData;
	unsigned char* bufferPtr;
	unsigned long bufferSize;

	// .wav ����� ������ ���̳ʸ� �������� ����.
	error = fopen_s(&filePtr, filename, "rb");
	if (error != 0)
	{
		assert(0);
		return;
	}

	// .wav ���� ��� �б�.
	count = fread(&waveFileHeader, sizeof(waveFileHeader), 1, filePtr);
	if (count != 1)
	{
		assert(0);
		return;
	}

	// chunk ID�� RIFF �������� üũ.
	if ((waveFileHeader.chunkId[0] != 'R') || (waveFileHeader.chunkId[1] != 'I') ||
		(waveFileHeader.chunkId[2] != 'F') || (waveFileHeader.chunkId[3] != 'F'))
	{
		assert(0);
	}

	// ���� ������ .wav �������� üũ.
	if ((waveFileHeader.format[0] != 'W') || (waveFileHeader.format[1] != 'A') ||
		(waveFileHeader.format[2] != 'V') || (waveFileHeader.format[3] != 'E'))
	{
		assert(0);
	}

	// sub chunk ID�� fmt �������� üũ.
	if ((waveFileHeader.subChunkId[0] != 'f') || (waveFileHeader.subChunkId[1] != 'm') ||
		(waveFileHeader.subChunkId[2] != 't') || (waveFileHeader.subChunkId[3] != ' '))
	{
		assert(0);
	}

	// ����� ������ WAVE_FORMAT_PCM���� üũ.
	if (waveFileHeader.audioFormat != WAVE_FORMAT_PCM)
	{
		assert(0);
	}

	// .wav ������ ���׷��� �������� �����Ǿ����� üũ.
	if (waveFileHeader.numChannels != 2)
	{
		assert(0);
	}

	// .wav ������ 44.1 KHz ���� ����Ʈ�� �����Ǿ����� üũ.
	if (waveFileHeader.sampleRate != 44100)
	{
		assert(0);
	}

	// .wav ������ 16��Ʈ �������� �����Ǿ����� üũ.
	if (waveFileHeader.bitsPerSample != 16)
	{
		assert(0);
	}

	// data chunk ����� �˻�.
	if ((waveFileHeader.dataChunkId[0] != 'd') || (waveFileHeader.dataChunkId[1] != 'a') ||
		(waveFileHeader.dataChunkId[2] != 't') || (waveFileHeader.dataChunkId[3] != 'a'))
	{
		assert(0);
	}

	// .wav ����� ������ �ε�� ���� ������ ������ wave �������� ������.
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	// .wav ����� ������ �ε�� ���� ������ ���� �����ڸ� ������.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = waveFileHeader.dataSize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// ������ �������� ���� ���ÿ� ���� �ӽ� ���� ���۸� ������.
	DXSOUND_THROW_INFO(m_pDirectSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL));

	// Test the buffer format against the direct sound 8 interface and create the secondary buffer.
	// direct sound 8 �������̽��� ���� ���� ������ �׽�Ʈ�ϰ�, ���� ���۸� ������.
	DXSOUND_THROW_INFO(tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&*secondaryBuffer));

	// �ӽ� ���۸� ��������.
	tempBuffer->Release();
	tempBuffer = nullptr;

	// ���� �����͸� data chunk ��� ������ �Űܼ� .wav ���� �������� ���� �������� �̵���.
	fseek(filePtr, sizeof(WaveHeaderType), SEEK_SET);

	// wave ���� �����͸� ����� �ӽ� ���۸� ����.
	waveData = new unsigned char[waveFileHeader.dataSize];
	if (!waveData)
	{
		assert(0);
	}

	// ���� ������ ���ۿ��ٰ� wave ���� �����͸� �о���.
	count = fread(waveData, 1, waveFileHeader.dataSize, filePtr);
	if (count != waveFileHeader.dataSize)
	{
		assert(0);
	}

	// �� �о����� ������ �ݾ���.
	error = fclose(filePtr);
	if (error != 0)
	{
		assert(0);
	}

	// ���� ���ۿ� wave ���� �����͸� ���� ���� �����.
	DXSOUND_THROW_INFO((*secondaryBuffer)->Lock(0, waveFileHeader.dataSize, (void**)&bufferPtr, (DWORD*)&bufferSize, NULL, 0, 0));

	// wave �����͸� ���ۿ� ��������.
	memcpy(bufferPtr, waveData, waveFileHeader.dataSize);

	// ���� ���ۿ� �����͸� �� ��ٸ� �ٽ� ����� Ǯ����.
	DXSOUND_THROW_INFO((*secondaryBuffer)->Unlock((void*)bufferPtr, bufferSize, NULL, 0));

	// ���� ���ۿ� �����͸� ���־����Ƿ� �ʿ���� �ӽ� ���۸� ��������.
	delete[] waveData;
	waveData = nullptr;

}


void DXSound::ShutdownWaveFile(IDirectSoundBuffer8** secondaryBuffer)
{
	// ���� ���۸� ��������.
	if (*secondaryBuffer)
	{
		(*secondaryBuffer)->Release();
		*secondaryBuffer = 0;
	}

	return;
}


void DXSound::SpawnControlWindow()
{
	if (ImGui::Begin("Sound"))
	{
		ImGui::SliderInt("Volume", &m_volume, DSBVOLUME_MIN, DSBVOLUME_MAX, "%d");
		if (ImGui::Button("Play"))
		{
			PlayWaveFile();
		}
	}
	ImGui::End();
}

void DXSound::PlayWaveFile()
{
	HRESULT hr;

	// ��� ��ġ�� �� ó������ ����.
	DXSOUND_THROW_INFO(m_pSecondaryBuffer->SetCurrentPosition(0));

	// ������ 100%�� ����.
	DXSOUND_THROW_INFO(m_pSecondaryBuffer->SetVolume(m_volume));

	// ���� ���ۿ� �ִ� ������ ��� ������.
	DXSOUND_THROW_INFO(m_pSecondaryBuffer->Play(0, 0, 0));
}

#pragma region Exception
DXSound::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs) noexcept
	:
	Exception(line, file),
	hr(hr)
{
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	// �������� ���� ���ڰ� �ִ� ��� ��������.
	if (!info.empty())
	{
		info.pop_back();
	}
}

// what() : ����� ���� ���ڿ��� ������ �����ϴ� �Լ�. std::exception���� ���� �������̵�.
const char* DXSound::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;

	if (!info.empty()) // ����� ���â�� �߰����� ���� ���� ���Խ��Ѽ� �Ѱ��ֵ��� ��.
	{
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	}

	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

// ���� Ÿ�� ���ڿ��� �������ִ� �Լ�.
const char* DXSound::HrException::GetType() const noexcept
{
	return "Custom Graphics Exception";
}

// ���� �ڵ�(HRESULT)�� �������ִ� �Լ�.
HRESULT DXSound::HrException::GetErrorCode() const noexcept
{
	return hr;
}

// dxerr ���̺귯���� ���� ���� ���ڿ��� �������ִ� �Լ�.
std::string DXSound::HrException::GetErrorString() const noexcept
{
	return DXGetErrorStringA(hr);
}

// dxerr ���̺귯���� ���� ������ ���� ���� ���ڿ��� �������ִ� �Լ�.
std::string DXSound::HrException::GetErrorDescription() const noexcept
{
	char buf[512];
	DXGetErrorDescriptionA(hr, buf, sizeof(buf));
	return buf;
}

// ����� ��� â ���� ���� ���ڿ��� �����ϴ� �Լ�.
std::string DXSound::HrException::GetErrorInfo() const noexcept
{
	return info;
}

// ���� Ÿ�� ���ڿ��� �������ִ� �Լ�.
const char* DXSound::DeviceRemovedException::GetType() const noexcept
{
	return "Custom Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}

DXSound::InfoException::InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept
	:
	Exception(line, file)
{
	// join all info messages with newlines into single string
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	// remove final newline if exists
	if (!info.empty())
	{
		info.pop_back();
	}
}

// what() : ����� ���� ���ڿ��� ������ �����ϴ� �Լ�. std::exception���� ���� �������̵�.
const char* DXSound::InfoException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

// ���� Ÿ�� ���ڿ��� �������ִ� �Լ�.
const char* DXSound::InfoException::GetType() const noexcept
{
	return "Custom Graphics Info Exception";
}

// ����� ��� â ���� ���� ���ڿ��� �����ϴ� �Լ�.
std::string DXSound::InfoException::GetErrorInfo() const noexcept
{
	return info;
}
#pragma endregion