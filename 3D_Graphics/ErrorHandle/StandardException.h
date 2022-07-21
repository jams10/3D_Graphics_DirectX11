#pragma once
#include <exception>
#include <string>

class StandardException : public std::exception
{
public:
	StandardException(int line, const char* file, std::string comment) noexcept;
	const char* what() const noexcept override;
	virtual const char* GetType() const noexcept;
	int GetLine() const noexcept;
	const std::string& GetFile() const noexcept;
	std::string GetOriginString() const noexcept;
private:
	int line;
	std::string file;
	std::string comment;
protected:
	mutable std::string whatBuffer; // whatBuffer�� ��� what() �Լ����� �� ���� �������ִµ�,
									// what() �Լ��� const �Լ��̹Ƿ�, const �Լ� ������ �� ���� �������ֱ� ���� mutable Ű����� �������־���.
};

#define ALLOCATE_EXCEPT(p, comment) if(p==nullptr) { StandardException e(__LINE__, __FILE__, comment); throw e;}
#define STD_EXCEPT(comment) { StandardException e(__LINE__, __FILE__, comment); throw e; }