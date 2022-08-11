#pragma once

// 매크로가 동작하기 위해서는 지역 스코프 범위에 HRESULT hr 변수가 있어야 함.

#define GFX_EXCEPT_NOINFO(hr) DXSound::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw DXSound::HrException( __LINE__,__FILE__,hr )

#ifndef NDEBUG // 디버그 모드인 경우 추가적인 정보(디버그 출력창의 정보)도 함께 넘겨줌.
#define GFX_EXCEPT(hr) DXSound::HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw GFX_EXCEPT(hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) DXSound::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO_ONLY(call) infoManager.Set(); (call); {auto v = infoManager.GetMessages(); if(!v.empty()) {throw DXSound::InfoException( __LINE__,__FILE__,v);}}
#else
#define GFX_EXCEPT(hr) DXSound::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) DXSound::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO_ONLY(call) (call)
#endif

// HRESULT hr을 지역 스코프에 선언해 주는 매크로.
// 디버그 모드의 경우 DxgiInfoManager 클래스 객체도 지역 스코프로 가져옴.
#ifdef NDEBUG
#define INFOMAN(gfx) HRESULT hr
#else
#define INFOMAN(gfx) HRESULT hr; DxgiInfoManager& infoManager = gfx.GetInfoManager();
#endif