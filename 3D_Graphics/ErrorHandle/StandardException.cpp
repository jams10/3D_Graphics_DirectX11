#include "StandardException.h"
#include <sstream>

StandardException::StandardException(int line, const char* file, std::string comment) noexcept
    :
    line(line),
    file(file),
    comment(comment)
{
}

const char* StandardException::what() const noexcept
{
    std::ostringstream oss;
    oss << GetType() << std::endl
        << GetOriginString() << std::endl << comment;
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* StandardException::GetType() const noexcept
{
    return "Standard Exception";
}

int StandardException::GetLine() const noexcept
{
    return line;
}

const std::string& StandardException::GetFile() const noexcept
{
    return file;
}

std::string StandardException::GetOriginString() const noexcept
{
    std::ostringstream oss;
    oss << "[File] " << file << std::endl
        << "[Line] " << line;
    return oss.str();
}
