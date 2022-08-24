#pragma once

// ��ũ�ΰ� �����ϱ� ���ؼ��� ���� ������ ������ HRESULT hr ������ �־�� ��.

#define DXSOUND_EXCEPT_NOINFO(hr) DXSound::HrException( __LINE__,__FILE__,(hr) )
#define DXSOUND_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw DXSound::HrException( __LINE__,__FILE__,hr )

#ifndef NDEBUG // ����� ����� ��� �߰����� ����(����� ���â�� ����)�� �Բ� �Ѱ���.
#define DXSOUND_EXCEPT(hr) DXSound::HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define DXSOUND_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw DXSOUND_EXCEPT(hr)
#define DXSOUND_DEVICE_REMOVED_EXCEPT(hr) DXSound::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define DXSOUND_THROW_INFO_ONLY(call) infoManager.Set(); (call); {auto v = infoManager.GetMessages(); if(!v.empty()) {throw DXSound::InfoException( __LINE__,__FILE__,v);}}
#else
#define DXSOUND_EXCEPT(hr) DXSound::HrException( __LINE__,__FILE__,(hr) )
#define DXSOUND_THROW_INFO(hrcall) DXSOUND_THROW_NOINFO(hrcall)
#define DXSOUND_DEVICE_REMOVED_EXCEPT(hr) DXSound::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#define DXSOUND_THROW_INFO_ONLY(call) (call)
#endif

// HRESULT hr�� ���� �������� ������ �ִ� ��ũ��.
// ����� ����� ��� DxgiInfoManager Ŭ���� ��ü�� ���� �������� ������.
#ifdef NDEBUG
#define INFOMAN(gfx) HRESULT hr
#else
#define INFOMAN(gfx) HRESULT hr; DxgiInfoManager& infoManager = gfx.GetInfoManager();
#endif