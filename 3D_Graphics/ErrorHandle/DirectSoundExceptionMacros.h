#pragma once

// ��ũ�ΰ� �����ϱ� ���ؼ��� ���� ������ ������ HRESULT hr ������ �־�� ��.

#define GFX_EXCEPT_NOINFO(hr) DXSound::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw DXSound::HrException( __LINE__,__FILE__,hr )

#ifndef NDEBUG // ����� ����� ��� �߰����� ����(����� ���â�� ����)�� �Բ� �Ѱ���.
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

// HRESULT hr�� ���� �������� ������ �ִ� ��ũ��.
// ����� ����� ��� DxgiInfoManager Ŭ���� ��ü�� ���� �������� ������.
#ifdef NDEBUG
#define INFOMAN(gfx) HRESULT hr
#else
#define INFOMAN(gfx) HRESULT hr; DxgiInfoManager& infoManager = gfx.GetInfoManager();
#endif