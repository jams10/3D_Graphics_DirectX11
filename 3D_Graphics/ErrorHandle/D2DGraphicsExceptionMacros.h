#pragma once

// �׷��Ƚ� ���� ���� �˻�/������ ��ũ�ε�.
// ��ũ�ΰ� �����ϱ� ���ؼ��� ���� ������ ������ HRESULT hr ������ �־�� ��.

#define GFX_EXCEPT_NOINFO(hr) D2DGraphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw D2DGraphics::HrException( __LINE__,__FILE__,hr )

#ifndef NDEBUG // ����� ����� ��� �߰����� ����(����� ���â�� ����)�� �Բ� �Ѱ���.
#define GFX_EXCEPT(hr) D2DGraphics::HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw GFX_EXCEPT(hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) D2DGraphics::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO_ONLY(call) infoManager.Set(); (call); {auto v = infoManager.GetMessages(); if(!v.empty()) {throw D2DGraphics::InfoException( __LINE__,__FILE__,v);}}
#else
#define GFX_EXCEPT(hr) D2DGraphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) D2DGraphics::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO_ONLY(call) (call)
#endif

// HRESULT hr�� ���� �������� ������ �ִ� ��ũ��.
// ����� ����� ��� DxgiInfoManager Ŭ���� ��ü�� ���� �������� ������.
#ifdef NDEBUG
#define INFOMAN(gfx) HRESULT hr
#else
#define INFOMAN(gfx) HRESULT hr; DxgiInfoManager& infoManager = gfx.GetInfoManager();
#endif