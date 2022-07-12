#include "Surface.h"
#include <Windows/Window.h>
#include <sstream>
#include <cassert>
#include <filesystem>
#include <Utils/StringUtils.h>

Surface::Surface(unsigned int width, unsigned int height)
{
	HRESULT hr = scratch.Initialize2D( // 2차원 배열 scratch image를 생성함.
		format,
		width, height, 1u, 1u
	);
	if (FAILED(hr))
	{
		throw Surface::Exception(__LINE__, __FILE__, "Failed to initialize ScratchImage", hr);
	}
}

// 인자로 주어진 색상 값으로 Surface 내부에 있는 픽셀 버퍼를 초기화하는 함수.
void Surface::Clear(Color fillValue) noexcept
{
	const auto width = GetWidth();
	const auto height = GetHeight();
	auto& imgData = *scratch.GetImage(0, 0, 0); // scratch image 배열의 제일 첫 번째 image 구조체를 얻어옴.
	for (size_t y = 0u; y < height; y++)
	{
		// width, height 크기의 image 버퍼의 각 row 시작 지점을 얻어와 해당 row를 width 만큼 fillValue로 채워줌.
		auto rowStart = reinterpret_cast<Color*>(imgData.pixels + imgData.rowPitch * y);
		std::fill(rowStart, rowStart + imgData.width, fillValue);
	}
}

// x, y 좌표를 입력 받아 픽셀 데이터 메모리 버퍼의 해당 위치 픽셀 값을 변경함.
void Surface::PutPixel(unsigned int x, unsigned int y, Color c) noexcept
{
	assert(x >= 0);
	assert(y >= 0);
	assert(x < GetWidth());
	assert(y < GetHeight());
	auto& imgData = *scratch.GetImage(0, 0, 0);
	reinterpret_cast<Color*>(&imgData.pixels[y * imgData.rowPitch])[x] = c;
}

// x, y 좌표를 입력 받아 픽셀 데이터 메모리 버퍼의 해당 위치 픽셀 값을 리턴함.
Surface::Color Surface::GetPixel(unsigned int x, unsigned int y) const noexcept
{
	assert(x >= 0);
	assert(y >= 0);
	assert(x < GetWidth());
	assert(y < GetHeight());
	auto& imgData = *scratch.GetImage(0, 0, 0);
	return reinterpret_cast<Color*>(&imgData.pixels[y * imgData.rowPitch])[x];
}

// 픽셀 버퍼 너비 리턴.
unsigned int Surface::GetWidth() const noexcept
{
	return (unsigned int)scratch.GetMetadata().width;
}

// 픽셀 버퍼 높이 리턴.
unsigned int Surface::GetHeight() const noexcept
{
	return (unsigned int)scratch.GetMetadata().height;
}

// 픽셀 버퍼에 대한 포인터 리턴.
Surface::Color* Surface::GetBufferPtr() noexcept
{
	return reinterpret_cast<Color*>(scratch.GetPixels());
}

// 픽셀 버퍼에 대한 포인터 리턴.(const)
const Surface::Color* Surface::GetBufferPtr() const noexcept
{
	return const_cast<Surface*>(this)->GetBufferPtr();
}

// 픽셀 버퍼에 대한 포인터 리턴.(const)
const Surface::Color* Surface::GetBufferPtrConst() const noexcept
{
	return const_cast<Surface*>(this)->GetBufferPtr();
}

// 이미지 파일로 부터 ScratchImage 객체를 생성해주는 함수.
Surface Surface::FromFile(const std::string& name)
{

	DirectX::ScratchImage scratch;
	// WIC 지원 비트맵 파일을 로드하여 ScratchImage 객체에 저장해줌.
	HRESULT hr = DirectX::LoadFromWICFile(ToWide(name).c_str(), DirectX::WIC_FLAGS_IGNORE_SRGB, nullptr, scratch);

	if (FAILED(hr))
	{
		throw Surface::Exception(__LINE__, __FILE__, name, "Failed to load image", hr);
	}

	// 만약 로드한 이미지 파일의 형식이 우리가 원하는 형식이 아니라면, 우리가 원하는 형식으로 변환해줌.
	if (scratch.GetImage(0, 0, 0)->format != format)
	{
		DirectX::ScratchImage converted;
		hr = DirectX::Convert(
			*scratch.GetImage(0, 0, 0),
			format,
			DirectX::TEX_FILTER_DEFAULT,
			DirectX::TEX_THRESHOLD_DEFAULT,
			converted
		);

		if (FAILED(hr))
		{
			throw Surface::Exception(__LINE__, __FILE__, name, "Failed to convert image", hr);
		}

		return Surface(std::move(converted));
	}

	return Surface(std::move(scratch));
}

// 시스템 메모리에 로드된 이미지 파일을 원하는 이미지 파일 형식으로 저장 해주는 함수.
void Surface::Save(const std::string& filename) const
{
	const auto GetCodecID = [](const std::string& filename) {
		const std::filesystem::path path = filename;
		const auto ext = path.extension().string();
		if (ext == ".png")
		{
			return DirectX::WIC_CODEC_PNG;
		}
		else if (ext == ".jpg")
		{
			return DirectX::WIC_CODEC_JPEG;
		}
		else if (ext == ".bmp")
		{
			return DirectX::WIC_CODEC_BMP;
		}
		throw Exception(__LINE__, __FILE__, filename, "Image format not supported");
	};

	wchar_t wideName[512];
	mbstowcs_s(nullptr, wideName, filename.c_str(), _TRUNCATE);

	HRESULT hr = DirectX::SaveToWICFile(
		*scratch.GetImage(0, 0, 0),
		DirectX::WIC_FLAGS_NONE,
		GetWICCodec(GetCodecID(filename)),
		ToWide(filename).c_str()
	);
	if (FAILED(hr))
	{
		throw Surface::Exception(__LINE__, __FILE__, filename, "Failed to save image", hr);
	}
}

// 불러온 이미지 파일에 Alpha 채널이 있는지 여부를 리턴하는 함수.
bool Surface::AlphaLoaded() const noexcept
{
	return !scratch.IsAlphaAllOpaque();
}

Surface::Surface(DirectX::ScratchImage scratch) noexcept
	:
	scratch(std::move(scratch))
{}

#pragma region Exception
Surface::Exception::Exception(int line, const char* file, std::string note, std::optional<HRESULT> hr) noexcept
	:
	CustomException(line, file),
	note("[Note] " + note)
{
	if (hr)
	{
		note = "[Error String] " + Window::Exception::TranslateErrorCode(*hr) + "\n" + note;
	}
}

Surface::Exception::Exception(int line, const char* file, std::string filename, std::string note_in, std::optional<HRESULT> hr) noexcept
	:
	CustomException(line, file)
{
	using namespace std::string_literals;
	note = "[File] "s + filename + "\n"s + "[Note] "s + note_in;

	if (hr)
	{
		note = "[Error String] " + Window::Exception::TranslateErrorCode(*hr) + note;
	}
}

const char* Surface::Exception::what() const noexcept
{
	std::ostringstream oss;
	oss << CustomException::what() << std::endl << GetNote();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Surface::Exception::GetType() const noexcept
{
	return "Custom Surface Exception";
}

const std::string& Surface::Exception::GetNote() const noexcept
{
	return note;
}
#pragma endregion