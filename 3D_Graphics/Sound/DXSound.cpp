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
	// Directsound와 primary sound buffer를 초기화 함.
	InitializeDirectSound(hwnd);

	// secondary buffer에 wav 오디오 파일을 불러옴.
	LoadWaveFile("Sound/Ding.wav", &m_pSecondaryBuffer);

	// secondary buffer에 불러온 wav 오디오 파일을 재생함.
	// PlayWaveFile();
}


void DXSound::Shutdown()
{
	// 보조 버퍼를 해제
	ShutdownWaveFile(&m_pSecondaryBuffer);

	// Directsound API를 닫기.
	ShutdownDirectSound();

	return;
}


void DXSound::InitializeDirectSound(HWND hwnd)
{
	HRESULT hr;
	DSBUFFERDESC bufferDesc;
	WAVEFORMATEX waveFormat;

	// 기본 사운드 장치에 대한 Directsound 인터페이스 포인터를 초기화.
	DXSOUND_THROW_INFO(DirectSoundCreate8(NULL, &m_pDirectSound, NULL));

	// 협력 수준(cooperative level)을 priority로 설정해 메인 사운드 버퍼의 형식이 수정될 수 있도록 함.
	DXSOUND_THROW_INFO(m_pDirectSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY));

	// 메인 버퍼 서술자를 설정.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = NULL;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// 기본 사운드 장치에 대한 메인 사운드 버퍼의 컨트롤을 얻어옴.
	DXSOUND_THROW_INFO(m_pDirectSound->CreateSoundBuffer(&bufferDesc, &m_pPrimaryBuffer, NULL));

	// 메인 사운드 버퍼의 형식을 설정함.
	// 우리는 형식을 16-bit 스테레오로 초당 44,100개의 샘플로 녹음되는 .WAV 파일 형식을 사용할 것임.(CD 오디오 포맷).
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	// 위에서 지정해준 wav 파일 형식으로 메인 버퍼를 설정.
	DXSOUND_THROW_INFO(m_pPrimaryBuffer->SetFormat(&waveFormat));

}


void DXSound::ShutdownDirectSound()
{
	// 메인 사운드 버퍼 포인터를 해제.
	SAFE_RELEASE(m_pPrimaryBuffer)

	// Directsound 인터페이스 포인터를 해제.
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

	// .wav 오디오 파일을 바이너리 형식으로 열기.
	error = fopen_s(&filePtr, filename, "rb");
	if (error != 0)
	{
		assert(0);
		return;
	}

	// .wav 파일 헤더 읽기.
	count = fread(&waveFileHeader, sizeof(waveFileHeader), 1, filePtr);
	if (count != 1)
	{
		assert(0);
		return;
	}

	// chunk ID가 RIFF 포맷인지 체크.
	if ((waveFileHeader.chunkId[0] != 'R') || (waveFileHeader.chunkId[1] != 'I') ||
		(waveFileHeader.chunkId[2] != 'F') || (waveFileHeader.chunkId[3] != 'F'))
	{
		assert(0);
	}

	// 파일 포맷이 .wav 포맷인지 체크.
	if ((waveFileHeader.format[0] != 'W') || (waveFileHeader.format[1] != 'A') ||
		(waveFileHeader.format[2] != 'V') || (waveFileHeader.format[3] != 'E'))
	{
		assert(0);
	}

	// sub chunk ID가 fmt 포맷인지 체크.
	if ((waveFileHeader.subChunkId[0] != 'f') || (waveFileHeader.subChunkId[1] != 'm') ||
		(waveFileHeader.subChunkId[2] != 't') || (waveFileHeader.subChunkId[3] != ' '))
	{
		assert(0);
	}

	// 오디오 포맷이 WAVE_FORMAT_PCM인지 체크.
	if (waveFileHeader.audioFormat != WAVE_FORMAT_PCM)
	{
		assert(0);
	}

	// .wav 파일이 스테레오 포맷으로 녹음되었는지 체크.
	if (waveFileHeader.numChannels != 2)
	{
		assert(0);
	}

	// .wav 파일이 44.1 KHz 샘플 레이트로 녹음되었는지 체크.
	if (waveFileHeader.sampleRate != 44100)
	{
		assert(0);
	}

	// .wav 파일이 16비트 형식으로 녹음되었는지 체크.
	if (waveFileHeader.bitsPerSample != 16)
	{
		assert(0);
	}

	// data chunk 헤더를 검사.
	if ((waveFileHeader.dataChunkId[0] != 'd') || (waveFileHeader.dataChunkId[1] != 'a') ||
		(waveFileHeader.dataChunkId[2] != 't') || (waveFileHeader.dataChunkId[3] != 'a'))
	{
		assert(0);
	}

	// .wav 오디오 파일이 로드될 보조 버퍼의 형식을 wave 형식으로 설정함.
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	// .wav 오디오 파일이 로드될 보조 버퍼의 버퍼 서술자를 설정함.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = waveFileHeader.dataSize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// 위에서 설정해준 버퍼 세팅에 따라 임시 사운드 버퍼를 생성함.
	DXSOUND_THROW_INFO(m_pDirectSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL));

	// Test the buffer format against the direct sound 8 interface and create the secondary buffer.
	// direct sound 8 인터페이스에 대해 버퍼 형식을 테스트하고, 보조 버퍼를 생성함.
	DXSOUND_THROW_INFO(tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&*secondaryBuffer));

	// 임시 버퍼를 해제해줌.
	tempBuffer->Release();
	tempBuffer = nullptr;

	// 파일 포인터를 data chunk 헤더 끝으로 옮겨서 .wav 파일 데이터의 시작 지점으로 이동함.
	fseek(filePtr, sizeof(WaveHeaderType), SEEK_SET);

	// wave 파일 데이터를 담아줄 임시 버퍼를 생성.
	waveData = new unsigned char[waveFileHeader.dataSize];
	if (!waveData)
	{
		assert(0);
	}

	// 새로 생성된 버퍼에다가 wave 파일 데이터를 읽어줌.
	count = fread(waveData, 1, waveFileHeader.dataSize, filePtr);
	if (count != waveFileHeader.dataSize)
	{
		assert(0);
	}

	// 다 읽었으면 파일을 닫아줌.
	error = fclose(filePtr);
	if (error != 0)
	{
		assert(0);
	}

	// 보조 버퍼에 wave 파일 데이터를 쓰기 위해 잠궈줌.
	DXSOUND_THROW_INFO((*secondaryBuffer)->Lock(0, waveFileHeader.dataSize, (void**)&bufferPtr, (DWORD*)&bufferSize, NULL, 0, 0));

	// wave 데이터를 버퍼에 복사해줌.
	memcpy(bufferPtr, waveData, waveFileHeader.dataSize);

	// 보조 버퍼에 데이터를 다 썼다면 다시 잠금을 풀어줌.
	DXSOUND_THROW_INFO((*secondaryBuffer)->Unlock((void*)bufferPtr, bufferSize, NULL, 0));

	// 보조 버퍼에 데이터를 써주었으므로 필요없는 임시 버퍼를 해제해줌.
	delete[] waveData;
	waveData = nullptr;

}


void DXSound::ShutdownWaveFile(IDirectSoundBuffer8** secondaryBuffer)
{
	// 보조 버퍼를 해제해줌.
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

	// 재생 위치를 맨 처음으로 설정.
	DXSOUND_THROW_INFO(m_pSecondaryBuffer->SetCurrentPosition(0));

	// 볼륨을 100%로 설정.
	DXSOUND_THROW_INFO(m_pSecondaryBuffer->SetVolume(m_volume));

	// 보조 버퍼에 있는 음원을 재생 시켜줌.
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
	// 마지막에 개행 문자가 있는 경우 삭제해줌.
	if (!info.empty())
	{
		info.pop_back();
	}
}

// what() : 출력할 에러 문자열을 생성해 리턴하는 함수. std::exception으로 부터 오버라이딩.
const char* DXSound::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;

	if (!info.empty()) // 디버그 출력창의 추가적인 정보 또한 포함시켜서 넘겨주도록 함.
	{
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	}

	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

// 예외 타입 문자열을 리턴해주는 함수.
const char* DXSound::HrException::GetType() const noexcept
{
	return "Custom Graphics Exception";
}

// 에러 코드(HRESULT)를 리턴해주는 함수.
HRESULT DXSound::HrException::GetErrorCode() const noexcept
{
	return hr;
}

// dxerr 라이브러리를 통해 에러 문자열을 리턴해주는 함수.
std::string DXSound::HrException::GetErrorString() const noexcept
{
	return DXGetErrorStringA(hr);
}

// dxerr 라이브러리를 통해 에러에 관한 설명 문자열을 리턴해주는 함수.
std::string DXSound::HrException::GetErrorDescription() const noexcept
{
	char buf[512];
	DXGetErrorDescriptionA(hr, buf, sizeof(buf));
	return buf;
}

// 디버그 출력 창 에러 정보 문자열을 리턴하는 함수.
std::string DXSound::HrException::GetErrorInfo() const noexcept
{
	return info;
}

// 예외 타입 문자열을 리턴해주는 함수.
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

// what() : 출력할 에러 문자열을 생성해 리턴하는 함수. std::exception으로 부터 오버라이딩.
const char* DXSound::InfoException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

// 예외 타입 문자열을 리턴해주는 함수.
const char* DXSound::InfoException::GetType() const noexcept
{
	return "Custom Graphics Info Exception";
}

// 디버그 출력 창 에러 정보 문자열을 리턴하는 함수.
std::string DXSound::InfoException::GetErrorInfo() const noexcept
{
	return info;
}
#pragma endregion