#include "CustomException.h"
#include <sstream>

CustomException::CustomException(int line, const char* file) noexcept
	:
	line(line),
	file(file)
{}

// what() : 출력할 에러 문자열을 생성해 리턴하는 함수.
const char* CustomException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}
// 예외 타입을 문자열로 리턴하는 함수.
const char* CustomException::GetType() const noexcept
{
	return "Custom Exception";
}
// 에러가 발생한 코드 줄 번호를 리턴하는 함수.
int CustomException::GetLine() const noexcept
{
	return line;
}
// 에러가 발생한 파일 이름을 리턴하는 함수.
const std::string& CustomException::GetFile() const noexcept
{
	return file;
}
// 에러가 발생한 코드 줄 번호 + 파일 이름을 합쳐 문자열로 리턴하는 함수.
std::string CustomException::GetOriginString() const noexcept
{
	std::ostringstream oss;
	oss << "[File] " << file << std::endl
		<< "[Line] " << line;
	return oss.str();
}