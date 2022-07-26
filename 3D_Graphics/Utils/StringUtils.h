#pragma once
#include <vector>
#include <string>

std::vector<std::string> TokenizeQuoted(const std::string& input);

std::vector<std::string> SplitString(std::string stringToSplit, char delim);

std::wstring ToWide(const std::string& narrow);

std::string ToNarrow(const std::wstring& wide);
