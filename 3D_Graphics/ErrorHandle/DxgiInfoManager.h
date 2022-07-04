#pragma once
#include <Windows/WindowsHeaders.h>
#include <vector>
#include <string>
#include <wrl.h>
#include <dxgidebug.h>

class DxgiInfoManager
{
public:
	DxgiInfoManager();
	~DxgiInfoManager() = default;
	DxgiInfoManager(const DxgiInfoManager&) = delete;
	DxgiInfoManager& operator=(const DxgiInfoManager&) = delete;
	void Set() noexcept;
	std::vector<std::string> GetMessages() const;
private:
	unsigned long long next = 0u;
	Microsoft::WRL::ComPtr<IDXGIInfoQueue> pDxgiInfoQueue; 	// Debug Information Queue를 제어하는 인터페이스. Debug Layer가 켜졌을 때만 사용할 수 있음.
															// 이 인터페이스를 통해 Debug Output의 결과를 MessageBox에 뿌려줄 수 있도록 할 것임.
};