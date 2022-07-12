#pragma once
#include <Windows/WindowsHeaders.h>
#include <ErrorHandle/CustomException.h>
#include <string>
#include <optional>
#include <dxtex/DirectXTex.h>

// Surface : 이미지 파일을 코드에서 다루기 위해 형상화한 클래스.
class Surface
{
public:
	// 실제 픽셀 값을 의미하는 클래스. dword 타입으로 픽셀 값을 담아줌.
	class Color
	{
	public:
		unsigned int dword; // unsigned int(4바이트) 값으로, 8bit * 4(ARGB) = 32bit. 한 픽셀에 해당하는 색상을 들고 있기에 충분한 크기.
		/*
			<dword 32비트 값 색상>
				  Alpha                Red                Green                Blue
			[][][][] [][][][] | [][][][] [][][][] | [][][][] [][][][] | [][][][] [][][][]
		*/
	public:
		constexpr Color() noexcept : dword()
		{}
		// Color 객체를 받아 dword 값을 초기화 해주는 복사 생성자.
		constexpr Color(const Color& col) noexcept
			:
			dword(col.dword)
		{}
		// dword 값을 직접 받아 초기화 해주는 생성자.
		constexpr Color(unsigned int dw) noexcept
			:
			dword(dw)
		{}
		// 각각의 a,r,g,b 값을 따로 받아 dword 값을 초기화 해주는 생성자.
		constexpr Color(unsigned char x, unsigned char r, unsigned char g, unsigned char b) noexcept
			:
			dword((x << 24u) | (r << 16u) | (g << 8u) | b) // dword에 한번에 넣어주기 위해 각 색상의 비트 위치를 이동시켜 넣어줌.
		{}
		// 각각의 r,g,b 값을 따로 받아 dword 값을 초기화 해주는 생성자.
		constexpr Color(unsigned char r, unsigned char g, unsigned char b) noexcept
			:
			dword((255u << 24u) | (r << 16u) | (g << 8u) | b) // 기본 알파값은 255
		{}
		// r,g,b 값은 Color 객체를, a값은 따로 받아 dword 값을 초기화 해주는 생성자.
		constexpr Color(Color col, unsigned char x) noexcept
			:
			Color((x << 24u) | col.dword)
		{}
		// 복사 대입 연산자
		Color& operator =(Color color) noexcept
		{
			dword = color.dword;
			return *this;
		}
		// 픽셀 색상의 Alpha 값 리턴하는 함수.
		constexpr unsigned char GetX() const noexcept
		{
			return dword >> 24u;
		}
		// 픽셀 색상의 Alpha 값 리턴하는 함수.
		constexpr unsigned char GetA() const noexcept
		{
			return GetX();
		}
		// 픽셀 색상의 Red 값 리턴하는 함수.
		constexpr unsigned char GetR() const noexcept
		{
			return (dword >> 16u) & 0xFFu; // 0xFFu : 1111 1111. 8비트 값을 넘겨주기 위해 맨 뒤로 땡겨와서 &연산 후 넘겨줌.
		}
		// 픽셀 색상의 Green 값 리턴하는 함수.
		constexpr unsigned char GetG() const noexcept
		{
			return (dword >> 8u) & 0xFFu;
		}
		// 픽셀 색상의 Blue 값 리턴하는 함수.
		constexpr unsigned char GetB() const noexcept
		{
			return dword & 0xFFu;         // 애초에 Blue 위치가 8비트 위치이기 때문에 시프트 연산이 필요 없음.
		}
		// 픽셀 색상의 Alpha 값 설정하는 함수.
		void SetX(unsigned char x) noexcept
		{
			dword = (dword & 0xFFFFFFu) | (x << 24u); // 특정 위치의 비트 값을 바꿔주기 위해 설정해줄 비트의 위치만 빼고 | 연산을 해줌.
		}
		// 픽셀 색상의 Alpha 값 설정하는 함수.
		void SetA(unsigned char a) noexcept
		{
			SetX(a);
		}
		// 픽셀 색상의 Red 값 설정하는 함수.
		void SetR(unsigned char r) noexcept
		{
			dword = (dword & 0xFF00FFFFu) | (r << 16u);
		}
		// 픽셀 색상의 Green 값 설정하는 함수.
		void SetG(unsigned char g) noexcept
		{
			dword = (dword & 0xFFFF00FFu) | (g << 8u);
		}
		// 픽셀 색상의 Blue 값 설정하는 함수.
		void SetB(unsigned char b) noexcept
		{
			dword = (dword & 0xFFFFFF00u) | b;
		}
	};
public:
	// 에러 처리용 커스텀 예외 클래스.
	class Exception : public CustomException
	{
	public:
		Exception(int line, const char* file, std::string note, std::optional<HRESULT> hr = {}) noexcept;
		Exception(int line, const char* file, std::string filename, std::string note, std::optional<HRESULT> hr = {}) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		const std::string& GetNote() const noexcept;
	private:
		std::optional<HRESULT> hr;
		std::string note;
	};
public:
	//
	// Surface 클래스관련
	//
	Surface(unsigned int width, unsigned int height);
	Surface(Surface&& source) noexcept = default;
	Surface(Surface&) = delete;
	Surface& operator=(Surface&& donor) noexcept = default;
	Surface& operator=(const Surface&) = delete;
	~Surface() = default;
	//
	// Color 값 관련
	//
	void Clear(Color fillValue) noexcept;
	void PutPixel(unsigned int x, unsigned int y, Color c) noexcept;
	Color GetPixel(unsigned int x, unsigned int y) const noexcept;
	unsigned int GetWidth() const noexcept;
	unsigned int GetHeight() const noexcept;
	Color* GetBufferPtr() noexcept;
	const Color* GetBufferPtr() const noexcept;
	const Color* GetBufferPtrConst() const noexcept;
	//
	//Surface 파일 관련
	//
	static Surface FromFile(const std::string& name);
	void Save(const std::string& filename) const;
	void Copy(const Surface& src) noexcept;
	bool AlphaLoaded() const noexcept;
private:
	Surface(DirectX::ScratchImage scratch) noexcept;
private:
	static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM; // 불러온 이미지를 이 포맷으로 받아줄것임.
	DirectX::ScratchImage scratch; // Surface 클래스는 ScratchImage의 래퍼 클래스가 됨.
};