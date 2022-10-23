#include "DemoApp.h"
#include "Animation.h"
#include "SoundManager.h"
#include <time.h>

D2D1_POINT_2U pos = D2D1::Point2U(0, 9);
D2D1_POINT_2U apos = D2D1::Point2U(9, 0);
CSoundManager* soundManager = NULL;
bool soundTog = FALSE;

short int gamestate = 0;

int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	if (SUCCEEDED(CoInitialize(NULL)))
	{
		{
			DemoApp app;

			if (SUCCEEDED(app.Initialize()))
			{
				app.RunMessageLoop();
			}
		}
		CoUninitialize();
		SAFE_DELETE(soundManager);
	}

	return 0;
}

DemoApp::DemoApp() :
	m_hwnd(NULL),
	m_pD2DFactory(NULL),
	m_pWICFactory(NULL),
	m_pDWriteFactory(NULL),
	m_pRenderTarget(NULL),

	m_pBoldBrush(NULL),
	m_pWhiteBrush(NULL),
	m_pSilverBrush(NULL),
	m_pMoonBitmapBrush(NULL),
	m_pPlanetBitmapBrush(NULL),
	m_pSunBitmapBrush(NULL),
	m_pMeteorBitmapMaskBrush(NULL),

	m_pTextFormat(NULL),

	m_pDogBitmap(NULL),
	m_pMeteorBitmapMask(NULL),
	m_pMoonBitmap(NULL),
	m_pPlanetBitmap(NULL),
	m_pSunBitmap(NULL),
	m_pSpaceBitmap(NULL),
	m_pAlienBitmap(NULL),

	m_pSpaceshipDrawGeometry(NULL),
	m_pMissileAnimationGeometry(NULL)
{ }

DemoApp::~DemoApp()
{
	SAFE_RELEASE(m_pD2DFactory);
	SAFE_RELEASE(m_pWICFactory);
	SAFE_RELEASE(m_pDWriteFactory);
	SAFE_RELEASE(m_pRenderTarget);

	SAFE_RELEASE(m_pBoldBrush);
	SAFE_RELEASE(m_pWhiteBrush);
	SAFE_RELEASE(m_pSilverBrush);
	SAFE_RELEASE(m_pMoonBitmapBrush);
	SAFE_RELEASE(m_pPlanetBitmapBrush);
	SAFE_RELEASE(m_pSunBitmapBrush);

	SAFE_RELEASE(m_pMeteorBitmapMaskBrush);

	SAFE_RELEASE(m_pTextFormat);

	SAFE_RELEASE(m_pDogBitmap);
	SAFE_RELEASE(m_pMeteorBitmapMask);
	SAFE_RELEASE(m_pMoonBitmap);
	SAFE_RELEASE(m_pPlanetBitmap);
	SAFE_RELEASE(m_pSunBitmap);
	SAFE_RELEASE(m_pSpaceBitmap);
	SAFE_RELEASE(m_pAlienBitmap);

	SAFE_RELEASE(m_pSpaceshipDrawGeometry);
	SAFE_RELEASE(m_pMissileAnimationGeometry);

	SAFE_DELETE(soundManager);
}

HRESULT DemoApp::Initialize()
{
	HRESULT hr;

	hr = CreateDeviceIndependentResources();

	if (SUCCEEDED(hr))
	{
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = DemoApp::WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = HINST_THISCOMPONENT;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = NULL;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.lpszClassName = L"D2DDemoApp";
		RegisterClassEx(&wcex);

		m_hwnd = CreateWindow(
			L"D2DDemoApp", L"Direct2D Demo Application", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			700, 700, NULL, NULL, HINST_THISCOMPONENT, this);

		hr = m_hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			float length = 0;
			hr = m_pMissileAnimationGeometry->ComputeLength(NULL, &length);

			if (SUCCEEDED(hr))
			{
				m_MissileAnimation.SetStart(0); //start at beginning of path
				m_MissileAnimation.SetEnd(length); //length at end of path
				m_MissileAnimation.SetDuration(3.0f); //seconds

				ShowWindow(m_hwnd, SW_SHOWNORMAL);
				UpdateWindow(m_hwnd);
			}
		}
		if (SUCCEEDED(hr))
		{
			soundManager = new CSoundManager;
			if (soundManager->init(m_hwnd))
			{
				int id;
				soundManager->add(L"bgm.wav", &id);
				soundManager->add(L"Dog Bark.wav", &id);
				soundManager->add(L"Sad Dog.wav", &id);
				soundManager->add(L"engine.wav", &id);

				soundManager->play(0, TRUE);
			}
		}
	}
	QueryPerformanceFrequency(&m_nFrequency);
	QueryPerformanceCounter(&m_nPrevTime);

	return hr;
}

HRESULT DemoApp::CreateDeviceIndependentResources()
{
	static const WCHAR fontName[] = L"Verdana";
	static const FLOAT fontSize = 50;
	HRESULT hr;
	ID2D1GeometrySink* pSink = NULL;

	// WIC 팩토리를 생성함.
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWICFactory));

	// D2D 팩토리를 생성함.
	if (SUCCEEDED(hr))
	{
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
	}
	if (SUCCEEDED(hr))
	{
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(m_pDWriteFactory),
			reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
		);
	}
	if (SUCCEEDED(hr))
	{
		hr = m_pDWriteFactory->CreateTextFormat(
			fontName, NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL, fontSize, L"", &m_pTextFormat);
	}
	if (SUCCEEDED(hr))
	{
		m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}
	// 우주선 그리기 경로기하
	if (SUCCEEDED(hr)) {
		hr = m_pD2DFactory->CreatePathGeometry(&m_pSpaceshipDrawGeometry);
	}
	if (SUCCEEDED(hr)) {
		hr = m_pSpaceshipDrawGeometry->Open(&pSink);
	}
	if (SUCCEEDED(hr)) {
		pSink->BeginFigure(D2D1::Point2F(0.0F, 0.0F), D2D1_FIGURE_BEGIN_FILLED);
		D2D1_POINT_2F points[] = { D2D1::Point2F(0.f, -10.f),
			D2D1::Point2F(20.f, -20.f), D2D1::Point2F(20.f, -50.f),
			D2D1::Point2F(35.f, -70.f), D2D1::Point2F(50.f, -50.f),
			D2D1::Point2F(50.f, -20.f), D2D1::Point2F(70.f, -10.f),
			D2D1::Point2F(70.f, 0.f), D2D1::Point2F(0.f, 0.f),
		};
		pSink->AddLines(points, ARRAYSIZE(points));
		pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

		//pSink->BeginFigure(D2D1::Point2F(), D2D1_FIGURE_BEGIN_FILLED);


		hr = pSink->Close();
	}
	SAFE_RELEASE(pSink);
	// 운석 애니메이션 경로
	if (SUCCEEDED(hr)) {
		hr = m_pD2DFactory->CreatePathGeometry(&m_pMissileAnimationGeometry);
	}
	if (SUCCEEDED(hr)) {
		hr = m_pMissileAnimationGeometry->Open(&pSink);
	}
	if (SUCCEEDED(hr)) {
		pSink->BeginFigure(D2D1::Point2F(0.0F, 0.0F), D2D1_FIGURE_BEGIN_HOLLOW);

		pSink->AddQuadraticBezier(
			D2D1::QuadraticBezierSegment(
				D2D1::Point2F(-400.f, 200.f),
				D2D1::Point2F(-700.f, 700.f)
			)
		);

		pSink->EndFigure(D2D1_FIGURE_END_OPEN);

		hr = pSink->Close();
	}
	SAFE_RELEASE(pSink);

	return hr;
}

HRESULT DemoApp::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	if (!m_pRenderTarget)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);
		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

		// D2D 렌더타겟을 생성함.
		hr = m_pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, size), &m_pRenderTarget);

		// 응용 프로그램 리소스로부터 비트맵 객체 m_pBitmap를 생성함.
		if (SUCCEEDED(hr))
		{
			hr = LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L".\\stay.png", 100, 0, &m_pDogBitmap);
		}
		if (SUCCEEDED(hr))
		{
			hr = LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L".\\meteor.png", 100, 0, &m_pMeteorBitmapMask);
		}
		if (SUCCEEDED(hr))
		{
			hr = LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L".\\moon.jpg", 45, 45, &m_pMoonBitmap);
		}
		if (SUCCEEDED(hr))
		{
			hr = LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L".\\spaceback.png", 0, 0, &m_pSpaceBitmap);
		}
		if (SUCCEEDED(hr))
		{
			hr = LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L".\\sun.png", 150, 0, &m_pSunBitmap);
		}
		if (SUCCEEDED(hr))
		{
			hr = LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L".\\planet.png", 100, 0, &m_pPlanetBitmap);
		}
		if (SUCCEEDED(hr))
		{
			hr = LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L".\\alien.png", 100, 0, &m_pAlienBitmap);
		}
		if (SUCCEEDED(hr))
		{
			D2D1_BITMAP_BRUSH_PROPERTIES propertiesXClampYClamp = D2D1::BitmapBrushProperties(
				D2D1_EXTEND_MODE_CLAMP, D2D1_EXTEND_MODE_CLAMP,
				D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);

			hr = m_pRenderTarget->CreateBitmapBrush(m_pMoonBitmap, propertiesXClampYClamp, &m_pMoonBitmapBrush);

			if (SUCCEEDED(hr))
			{
				hr = m_pRenderTarget->CreateBitmapBrush(m_pSunBitmap, propertiesXClampYClamp, &m_pSunBitmapBrush);
				if (SUCCEEDED(hr))
				{
					hr = m_pRenderTarget->CreateBitmapBrush(m_pPlanetBitmap, propertiesXClampYClamp, &m_pPlanetBitmapBrush);
				}
			}
		}

		if (SUCCEEDED(hr))
		{
			hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightSlateGray), &m_pBoldBrush);
		}
		if (SUCCEEDED(hr))
		{
			hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pWhiteBrush);
		}
		if (SUCCEEDED(hr))
		{
			hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Silver), &m_pSilverBrush);
		}
	}

	return hr;
}

void DemoApp::DiscardDeviceResources()
{
	SAFE_RELEASE(m_pRenderTarget);

	SAFE_RELEASE(m_pBoldBrush);
	SAFE_RELEASE(m_pWhiteBrush);
	SAFE_RELEASE(m_pSilverBrush);
	SAFE_RELEASE(m_pMoonBitmapBrush);
	SAFE_RELEASE(m_pPlanetBitmapBrush);
	SAFE_RELEASE(m_pSunBitmapBrush);
	SAFE_RELEASE(m_pMeteorBitmapMaskBrush);

	SAFE_RELEASE(m_pDogBitmap);
	SAFE_RELEASE(m_pMeteorBitmapMask);
	SAFE_RELEASE(m_pMoonBitmap);
	SAFE_RELEASE(m_pPlanetBitmap);
	SAFE_RELEASE(m_pSunBitmap);
	SAFE_RELEASE(m_pSpaceBitmap);
	SAFE_RELEASE(m_pAlienBitmap);

	SAFE_RELEASE(m_pSpaceshipDrawGeometry);
	SAFE_RELEASE(m_pMissileAnimationGeometry);
}

void DemoApp::RunMessageLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

HRESULT DemoApp::OnRender()
{
	HRESULT hr = CreateDeviceResources();

	if (SUCCEEDED(hr))
	{

		static const WCHAR sc_Clear[] = L"CLEAR!!! :)";
		static const WCHAR sc_Defeat[] = L"FAIL... ;(";
		// 렌더타겟의 크기를 얻음.
		D2D1_SIZE_F renderTargetSize = m_pRenderTarget->GetSize();

		float width = static_cast<int>(renderTargetSize.width);
		float height = static_cast<int>(renderTargetSize.height);

		float width_gap = width / 10;
		float height_gap = height / 10;

		D2D1::Matrix3x2F scale = D2D1::Matrix3x2F::Scale(width / 700, height / 700);

		D2D1::Matrix3x2F translation = D2D1::Matrix3x2F::Translation(width, 0);
		D2D1::Matrix3x2F meteorTranslation = D2D1::Matrix3x2F::Translation(width + 10, 10);

		D2D1_POINT_2F point;
		D2D1_POINT_2F tangent;
		
		D2D1_MATRIX_3X2_F triangleMatrix;

		// 애니메이션을 가지는 운석이 그려지는 RECT
		D2D1_RECT_F rcMeteor = D2D1::RectF(0, 0, 30, 30);
		// 가만히 있는 배경들이 그려지는 RECT
		D2D1_RECT_F rcSun = D2D1::RectF(0, 0, 150, 150);
		D2D1_RECT_F rcPlanet = D2D1::RectF(0, 0, 100, 100);

		m_pRenderTarget->BeginDraw();
		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

		m_pRenderTarget->SetTransform(scale);
		m_pRenderTarget->DrawBitmap(
			m_pSpaceBitmap,
			D2D1::RectF(0.f, 0.f, 700.f, 700.f)
		);

		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		// 10x10 좌표를 가시화해서 그림.

		for (float x = 0; x < width; x += width_gap)
		{
			m_pRenderTarget->DrawLine(
				D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
				D2D1::Point2F(static_cast<FLOAT>(x), renderTargetSize.height),
				m_pBoldBrush, 5.f
			);
		}

		for (float y = 0; y < height; y += height_gap)
		{
			m_pRenderTarget->DrawLine(
				D2D1::Point2F(0.0f, static_cast<FLOAT>(y)),
				D2D1::Point2F(renderTargetSize.width, static_cast<FLOAT>(y)),
				m_pBoldBrush, 5.f
			);
		}

		// 비트맵 m_pDogBitmap를 그림.
		m_pRenderTarget->DrawBitmap(
			m_pDogBitmap,
			D2D1::RectF(pos.x * width_gap, ((pos.y) * height_gap), 
				(pos.x + 1) * width_gap , ((pos.y + 1) * height_gap))
		);

		// 기하경로를 사용한 우주선 모형을 랜덤좌표에 배치
		srand(static_cast<unsigned int>(time(NULL)));
		static unsigned int randx = rand() % 10;
		srand(static_cast<unsigned int>(time(NULL)));
		static unsigned int randy = (rand() % 9) + 1;
		
		//게임 승리 조건에 만족시
		if (pos.x == randx && pos.y + 1 == randy)
		{
			gamestate = 1;
			m_pRenderTarget->DrawTextW(
				sc_Clear, ARRAYSIZE(sc_Clear) - 1, m_pTextFormat,
				D2D1::RectF(0, 0, width, height), m_pSilverBrush
			);
			if (!soundTog) soundManager->play(3, TRUE);
		}
		//게임 패배 조건 만족시
		if (pos.x == apos.x && pos.y == apos.y)
		{
			gamestate = 2;
			m_pRenderTarget->DrawTextW(
				sc_Defeat, ARRAYSIZE(sc_Defeat) - 1, m_pTextFormat,
				D2D1::RectF(0, 0, width, height), m_pSilverBrush
			);
			if (!soundTog) soundManager->play(2, TRUE);
		}
		if (gamestate != 0)
			soundManager->stop(0);
		else
		{
			m_pRenderTarget->SetTransform(scale * D2D1::Matrix3x2F::Translation(width_gap * randx, height_gap * randy));
			m_pRenderTarget->FillGeometry(m_pSpaceshipDrawGeometry, m_pSilverBrush);
			m_pRenderTarget->FillEllipse(D2D1::Ellipse(D2D1::Point2F(35, -35), 7.f, 7.f), m_pWhiteBrush);
		}
		// 외계인 Bitmap을 그림
		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		m_pRenderTarget->DrawBitmap(
			m_pAlienBitmap,
			D2D1::RectF(apos.x * width_gap, apos.y * height_gap, (apos.x + 1) * width_gap, (apos.y + 1) * height_gap)
		);

		// 운석 애니메니션을 그림
		m_pRenderTarget->SetTransform(scale * translation);
		static float anitime = 0.0f;
		
		//m_pRenderTarget->DrawGeometry(m_pMissileAnimationGeometry, m_pBoldBrush);

		float length = m_MissileAnimation.GetValue(anitime);

		// 현재 시간에 해당하는 기하 길이에 일치하는 이동 동선 상의 지점을 얻음.
		m_pMissileAnimationGeometry->ComputePointAtLength(length, NULL, &point, &tangent);

		triangleMatrix = D2D1::Matrix3x2F(
			tangent.x, tangent.y,
			-tangent.y, tangent.x,
			point.x, point.y);

		m_pRenderTarget->SetTransform(triangleMatrix * scale * meteorTranslation);

		//m_pRenderTarget->FillGeometry(m_pMissileDrawingGeometry , m_pBoldBrush);
		// 운석 비트맵을 불투명 마스크 사용해서 그림
		m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		m_pRenderTarget->FillOpacityMask(m_pMeteorBitmapMask, m_pMoonBitmapBrush,
			D2D1_OPACITY_MASK_CONTENT_GRAPHICS, &rcMeteor);
		m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

		// 배경 행성들을 그림
		m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		m_pRenderTarget->SetTransform(scale * D2D1::Matrix3x2F::Translation(width / 2, (height / 2) - (4 * height_gap)));
		m_pRenderTarget->FillOpacityMask(m_pMeteorBitmapMask, m_pSunBitmapBrush,
			D2D1_OPACITY_MASK_CONTENT_GRAPHICS, &rcSun);
		m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);


		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		m_pRenderTarget->SetTransform(scale * D2D1::Matrix3x2F::Translation((width / 2) - (4 * width_gap), (height / 2) + (2 * height_gap)));
		m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		m_pRenderTarget->FillOpacityMask(m_pMeteorBitmapMask, m_pPlanetBitmapBrush,
			D2D1_OPACITY_MASK_CONTENT_GRAPHICS, &rcPlanet);
		m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);


		hr = m_pRenderTarget->EndDraw();

		if (hr == D2DERR_RECREATE_TARGET)
		{
			hr = S_OK;
			DiscardDeviceResources();
		}

		if (anitime >= m_MissileAnimation.GetDuration())
		{
			anitime = 0.0f;
		}
		else
		{
			LARGE_INTEGER CurrentTime;
			QueryPerformanceCounter(&CurrentTime);

			float elapsedTime = (float)((double)(CurrentTime.QuadPart - m_nPrevTime.QuadPart) / (double)(m_nFrequency.QuadPart));
			m_nPrevTime = CurrentTime;

			anitime += elapsedTime;
		}
	}

	return hr;
}

void DemoApp::OnResize(UINT width, UINT height)
{
	if (m_pRenderTarget)
	{
		m_pRenderTarget->Resize(D2D1::SizeU(width, height));
	}
}

D2D1_POINT_2U AlienChase(D2D1_POINT_2U apos, D2D1_POINT_2U pos)
{
	if (0 <= apos.x && apos.x <= 9 && pos.x != apos.x)
	{
		if (apos.x > pos.x)
			apos.x--;
		else if (apos.x < pos.x)
			apos.x++;
	}
	else if (0 <= apos.y && apos.y <= 9 && pos.y != apos.y)
	{
		if (apos.y > pos.y)
			apos.y--;
		else if (apos.y < pos.y)
			apos.y++;
	}
	return (apos);
}

LRESULT CALLBACK DemoApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		DemoApp* pDemoApp = (DemoApp*)pcs->lpCreateParams;

		SetWindowLongPtrW(hwnd, GWLP_USERDATA, PtrToUlong(pDemoApp));
		return 1;
	}

	DemoApp* pDemoApp = reinterpret_cast<DemoApp*>(static_cast<LONG_PTR>(GetWindowLongPtrW(hwnd, GWLP_USERDATA)));

	if (pDemoApp)
	{
		switch (message)
		{
		case WM_SIZE:
			{
				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);
				pDemoApp->OnResize(width, height);
			}
			return 0;
		case WM_KEYDOWN:
			switch (wParam)
			{
			case VK_RIGHT:
				// when they press Right Arrow Key
				if (0 <= pos.x && pos.x < 9 && gamestate == 0)
				{
					pos.x++;
					apos = AlienChase(apos, pos);
					if (soundTog == FALSE)
						soundManager->play(1, FALSE);
				}
				break;
			case VK_LEFT:
				// when they press Left Arrow Key
				if (1 <= pos.x && pos.x <= 9 && gamestate == 0)
				{
					pos.x--;
					apos = AlienChase(apos, pos);
					if (soundTog == FALSE)
						soundManager->play(1, FALSE);
				}
				break;
			case VK_UP:
				if (1 <= pos.y && pos.y <= 9 && gamestate == 0)
				{
					pos.y--;
					apos = AlienChase(apos, pos);
					if (soundTog == FALSE)
						soundManager->play(1, FALSE);
				}
				break;
			case VK_DOWN:
				if (0 <= pos.y && pos.y < 9 && gamestate == 0)
				{
					pos.y++;
					apos = AlienChase(apos, pos);
					if (soundTog == FALSE)
						soundManager->play(1, FALSE);
				}
				break;
			case VK_SPACE:
				soundTog = !soundTog;
				if (soundTog)
					soundManager->stop(0);
				else
					soundManager->play(0, TRUE);
				break;
			}

			return 0;

		case WM_DISPLAYCHANGE:
			{
				InvalidateRect(hwnd, NULL, FALSE);
			}
			return 0;

		case WM_PAINT:
			{
				pDemoApp->OnRender();

				//ValidateRect(hwnd, NULL);
			}
			return 0;

		case WM_DESTROY:
			{
				PostQuitMessage(0);
			}
			return 1;
		}
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

// Creates a Direct2D bitmap from a resource in the application resource file.
HRESULT DemoApp::LoadBitmapFromResource(ID2D1RenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, PCWSTR resourceName, PCWSTR resourceType, UINT destinationWidth, UINT destinationHeight, ID2D1Bitmap** ppBitmap)
{
	HRESULT hr = S_OK;
	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pSource = NULL;
	IWICStream* pStream = NULL;
	IWICFormatConverter* pConverter = NULL;
	IWICBitmapScaler* pScaler = NULL;

	HRSRC imageResHandle = NULL;
	HGLOBAL imageResDataHandle = NULL;
	void* pImageFile = NULL;
	DWORD imageFileSize = 0;

	// Locate the resource.
	imageResHandle = FindResourceW(HINST_THISCOMPONENT, resourceName, resourceType);

	hr = imageResHandle ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		// Load the resource.
		imageResDataHandle = LoadResource(HINST_THISCOMPONENT, imageResHandle);

		hr = imageResDataHandle ? S_OK : E_FAIL;
	}

	if (SUCCEEDED(hr))
	{
		// Lock it to get a system memory pointer.
		pImageFile = LockResource(imageResDataHandle);

		hr = pImageFile ? S_OK : E_FAIL;
	}

	if (SUCCEEDED(hr))
	{
		// Calculate the size.
		imageFileSize = SizeofResource(HINST_THISCOMPONENT, imageResHandle);

		hr = imageFileSize ? S_OK : E_FAIL;
	}

	if (SUCCEEDED(hr))
	{
		// Create a WIC stream to map onto the memory.
		hr = pIWICFactory->CreateStream(&pStream);
	}

	if (SUCCEEDED(hr))
	{
		// Initialize the stream with the memory pointer and size.
		hr = pStream->InitializeFromMemory(
			reinterpret_cast<BYTE*>(pImageFile),
			imageFileSize
		);
	}

	if (SUCCEEDED(hr))
	{
		// Create a decoder for the stream.
		hr = pIWICFactory->CreateDecoderFromStream(
			pStream,
			NULL,
			WICDecodeMetadataCacheOnLoad,
			&pDecoder
		);
	}

	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}

	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
	}

	if (SUCCEEDED(hr))
	{
		// If a new width or height was specified, create an
		// IWICBitmapScaler and use it to resize the image.
		if (destinationWidth != 0 || destinationHeight != 0)
		{
			UINT originalWidth, originalHeight;
			hr = pSource->GetSize(&originalWidth, &originalHeight);
			if (SUCCEEDED(hr))
			{
				if (destinationWidth == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
					destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
				}
				else if (destinationHeight == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
					destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
				}

				hr = pIWICFactory->CreateBitmapScaler(&pScaler);
				if (SUCCEEDED(hr))
				{
					hr = pScaler->Initialize(
						pSource,
						destinationWidth,
						destinationHeight,
						WICBitmapInterpolationModeCubic
					);
				}

				if (SUCCEEDED(hr))
				{
					hr = pConverter->Initialize(
						pScaler,
						GUID_WICPixelFormat32bppPBGRA,
						WICBitmapDitherTypeNone,
						NULL,
						0.f,
						WICBitmapPaletteTypeMedianCut
					);
				}
			}
		}
		else
		{
			hr = pConverter->Initialize(
				pSource,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				NULL,
				0.f,
				WICBitmapPaletteTypeMedianCut
			);
		}
	}

	if (SUCCEEDED(hr))
	{
		//create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
		);
	}

	SAFE_RELEASE(pDecoder);
	SAFE_RELEASE(pSource);
	SAFE_RELEASE(pStream);
	SAFE_RELEASE(pConverter);
	SAFE_RELEASE(pScaler);

	return hr;
}

// Creates a Direct2D bitmap from the specified file name.
HRESULT DemoApp::LoadBitmapFromFile(ID2D1RenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, PCWSTR uri, UINT destinationWidth, UINT destinationHeight, ID2D1Bitmap** ppBitmap)
{
	HRESULT hr = S_OK;

	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pSource = NULL;
	IWICStream* pStream = NULL;
	IWICFormatConverter* pConverter = NULL;
	IWICBitmapScaler* pScaler = NULL;

	hr = pIWICFactory->CreateDecoderFromFilename(
		uri,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);

	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}

	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
	}

	if (SUCCEEDED(hr))
	{
		// If a new width or height was specified, create an
		// IWICBitmapScaler and use it to resize the image.
		if (destinationWidth != 0 || destinationHeight != 0)
		{
			UINT originalWidth, originalHeight;
			hr = pSource->GetSize(&originalWidth, &originalHeight);
			if (SUCCEEDED(hr))
			{
				if (destinationWidth == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
					destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
				}
				else if (destinationHeight == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
					destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
				}

				hr = pIWICFactory->CreateBitmapScaler(&pScaler);
				if (SUCCEEDED(hr))
				{
					hr = pScaler->Initialize(
						pSource,
						destinationWidth,
						destinationHeight,
						WICBitmapInterpolationModeCubic
					);
				}

				if (SUCCEEDED(hr))
				{
					hr = pConverter->Initialize(
						pScaler,
						GUID_WICPixelFormat32bppPBGRA,
						WICBitmapDitherTypeNone,
						NULL,
						0.f,
						WICBitmapPaletteTypeMedianCut
					);
				}
			}
		}
		else // Don't scale the image.
		{
			hr = pConverter->Initialize(
				pSource,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				NULL,
				0.f,
				WICBitmapPaletteTypeMedianCut
			);
		}
	}
	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
		);
	}

	SAFE_RELEASE(pDecoder);
	SAFE_RELEASE(pSource);
	SAFE_RELEASE(pStream);
	SAFE_RELEASE(pConverter);
	SAFE_RELEASE(pScaler);

	return hr;
}
