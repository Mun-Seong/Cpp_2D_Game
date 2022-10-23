#pragma once

// Windows Header Files:
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>
// DX Header Files
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include "Animation.h"

// 자원 안전 반환 매크로.
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define SAFE_DELETE(p) { if(p) { delete (p); (p)=NULL; } }

// 현재 모듈의 시작주소 얻기.
#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

class DemoApp
{
public:
	DemoApp();
	~DemoApp();
	HRESULT Initialize();
	void RunMessageLoop();

private:
	HRESULT CreateDeviceIndependentResources();
	HRESULT CreateDeviceResources();
	void DiscardDeviceResources();
	HRESULT OnRender();
	void OnResize(UINT width, UINT height);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HRESULT CreateGridPatternBrush(ID2D1RenderTarget* pRenderTarget, __deref_out ID2D1BitmapBrush** ppBitmapBrush);

	HRESULT LoadBitmapFromResource(ID2D1RenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, PCWSTR resourceName, PCWSTR resourceType, UINT destinationWidth, UINT destinationHeight, __deref_out ID2D1Bitmap** ppBitmap);
	HRESULT LoadBitmapFromFile(ID2D1RenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, PCWSTR uri, UINT destinationWidth, UINT destinationHeight, __deref_out ID2D1Bitmap** ppBitmap);

private:
	HWND m_hwnd;
	ID2D1Factory* m_pD2DFactory;
	IWICImagingFactory* m_pWICFactory;
	IDWriteFactory* m_pDWriteFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;

	ID2D1SolidColorBrush* m_pBoldBrush;
	ID2D1SolidColorBrush* m_pWhiteBrush;
	ID2D1SolidColorBrush* m_pSilverBrush;
	ID2D1BitmapBrush* m_pMoonBitmapBrush;
	ID2D1BitmapBrush* m_pMeteorBitmapMaskBrush;
	ID2D1BitmapBrush* m_pPlanetBitmapBrush;
	ID2D1BitmapBrush* m_pSunBitmapBrush;

	IDWriteTextFormat* m_pTextFormat;
	
	ID2D1Bitmap* m_pDogBitmap;
	ID2D1Bitmap* m_pMeteorBitmapMask;
	ID2D1Bitmap* m_pMoonBitmap;
	ID2D1Bitmap* m_pPlanetBitmap;
	ID2D1Bitmap* m_pSunBitmap;
	ID2D1Bitmap* m_pSpaceBitmap;

	ID2D1Bitmap* m_pAlienBitmap;



	AnimationLinear<float> m_MissileAnimation;

	LARGE_INTEGER m_nPrevTime;
	LARGE_INTEGER m_nFrequency;

	ID2D1PathGeometry* m_pSpaceshipDrawGeometry;
	ID2D1PathGeometry* m_pMissileAnimationGeometry;
};
