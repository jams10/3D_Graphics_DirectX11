#pragma once
#include <exception>
#include <string>

class CustomException : public std::exception
{
public:
	CustomException(int line, const char* file) noexcept;
	const char* what() const noexcept override;
	virtual const char* GetType() const noexcept;
	int GetLine() const noexcept;
	const std::string& GetFile() const noexcept;
	std::string GetOriginString() const noexcept;
private:
	int line;
	std::string file;
protected:
	mutable std::string whatBuffer; // whatBuffer의 경우 what() 함수에서 그 값을 변경해주는데,
									// what() 함수가 const 함수이므로, const 함수 내에서 그 값을 변경해주기 위해 mutable 키워드로 선언해주었다.
};