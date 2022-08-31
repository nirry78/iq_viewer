#include "IQViewer.h"

IQViewer::IQViewer():
    m_hwnd(NULL),
    m_pD2DFactory(NULL),
    m_pRenderTarget(NULL),
    m_pBlackBrush(NULL),
    m_pGrayBrush(NULL),
    m_pStrokeStyleDotRound(NULL)
{

}

IQViewer::~IQViewer()
{
    SafeRelease(&m_pD2DFactory);
    DiscardDeviceResources();
}

HRESULT IQViewer::CreateDeviceIndependentResources()
{
    HRESULT hr;

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

    return hr;
}

HRESULT IQViewer::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    if (!m_pRenderTarget)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            static_cast<UINT>(rc.right - rc.left),
            static_cast<UINT>(rc.bottom - rc.top)
            );

        // Create a Direct2D render target.
        hr = m_pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
                                                   D2D1::HwndRenderTargetProperties(m_hwnd, size),
                                                   &m_pRenderTarget);

        if (SUCCEEDED(hr))
        {
            // Create a black brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Black),
                &m_pBlackBrush
                );
        }

        if (SUCCEEDED(hr))
        {
            // Create a black brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Gray),
                &m_pGrayBrush
                );
        }


        // Dash array for dashStyle D2D1_DASH_STYLE_CUSTOM
        float dashes[] = {1.0f, 2.0f, 2.0f, 3.0f, 2.0f, 2.0f};

        // Stroke Style with Dash Style -- Custom
        if (SUCCEEDED(hr))
        {
            hr = m_pD2DFactory->CreateStrokeStyle(
                D2D1::StrokeStyleProperties(
                    D2D1_CAP_STYLE_FLAT,
                    D2D1_CAP_STYLE_FLAT,
                    D2D1_CAP_STYLE_ROUND,
                    D2D1_LINE_JOIN_MITER,
                    10.0f,
                    D2D1_DASH_STYLE_DOT,
                    0.0f),
                0,
                0,
                &m_pStrokeStyleDotRound
                );
        }
    }

    return hr;
}

void IQViewer::DiscardDeviceResources()
{
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBlackBrush);
    SafeRelease(&m_pGrayBrush);
    SafeRelease(&m_pStrokeStyleDotRound);
}

HRESULT IQViewer::DrawGraph(D2D1_RECT_F rect)
{
    HRESULT hr = S_OK;
    float left_margin = 30.0f;
    float right_margin = 15.0f;
    float top_margin = 15.0f;
    float bottom_margin = 15.0f;
    float strokeWidth = 1.2f;
    float cell_height = ((rect.bottom - bottom_margin) - (rect.top + top_margin)) / 6;
    float cell_width = ((rect.right - right_margin) - (rect.left + left_margin)) / 10;
    float middle_y = cell_height * 3 + rect.top;

    m_pRenderTarget->DrawLine(D2D1::Point2F(rect.left + left_margin, rect.top + top_margin),
                              D2D1::Point2F(rect.left + left_margin, rect.bottom - bottom_margin),
                              m_pBlackBrush,
                              strokeWidth);


    for (size_t index = 0; index < 7; index++)
    {
        if (index == 3)
        {
            m_pRenderTarget->DrawLine(D2D1::Point2F(rect.left + left_margin, rect.top + top_margin + cell_height * index),
                                      D2D1::Point2F(rect.right - right_margin, rect.top + top_margin + cell_height * index),
                                      m_pBlackBrush,
                                      strokeWidth);
        }
        else
        {
            m_pRenderTarget->DrawLine(D2D1::Point2F(rect.left + left_margin, rect.top + top_margin + cell_height * index),
                                      D2D1::Point2F(rect.right - right_margin, rect.top + top_margin + cell_height * index),
                                      m_pGrayBrush,
                                      strokeWidth,
                                      m_pStrokeStyleDotRound);
        }
    }

    for (size_t index = 1; index < 11; index++)
    {
        m_pRenderTarget->DrawLine(D2D1::Point2F(rect.left + left_margin + index * cell_width, rect.top + top_margin),
                                  D2D1::Point2F(rect.left + left_margin + index * cell_width, rect.bottom - bottom_margin),
                                  m_pGrayBrush,
                                  strokeWidth,
                                  m_pStrokeStyleDotRound);
    }

    return hr;
}

HRESULT IQViewer::Initialize()
{
    HRESULT hr;

    // Initialize device-indpendent resources, such
    // as the Direct2D factory.
    hr = CreateDeviceIndependentResources();
    if (SUCCEEDED(hr))
    {
        // Register the window class.
        WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
        wcex.style         = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc   = IQViewer::WndProc;
        wcex.cbClsExtra    = 0;
        wcex.cbWndExtra    = sizeof(LONG_PTR);
        wcex.hInstance     = HINST_THISCOMPONENT;
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName  = NULL;
        wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wcex.lpszClassName = L"IQViewer";

        RegisterClassEx(&wcex);

        // Create the application window.
        //
        // Because the CreateWindow function takes its size in pixels, we
        // obtain the system DPI and use it to scale the window size.
        FLOAT dpiX, dpiY;
        m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

        // Create the application window.
        m_hwnd = CreateWindow(
            L"IQViewer",
            L"IQ Viewer",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<INT>(ceil(1024.f * dpiX / 96.f)),
            static_cast<INT>(ceil(768.f * dpiY / 96.f)),
            NULL,
            NULL,
            HINST_THISCOMPONENT,
            this
            );
        hr = m_hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            ShowWindow(m_hwnd, SW_SHOWNORMAL);
            UpdateWindow(m_hwnd);
        }
    }

    return hr;
}

HRESULT IQViewer::OnRender()
{
    HRESULT hr;

    hr = CreateDeviceResources();
    if (SUCCEEDED(hr) && !(m_pRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_F size = D2D1::SizeF(
            static_cast<FLOAT>(rc.right - rc.left),
            static_cast<FLOAT>(rc.bottom - rc.top)
            );

        m_pRenderTarget->BeginDraw();

        m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

        if (size.height / 2 > 60)
        {
            DrawGraph(D2D1::RectF(0.0f, 0.0f, size.width, size.height / 2.0f));
            DrawGraph(D2D1::RectF(0.0f, size.height / 2.0f, size.width, size.height));
        }

        hr = m_pRenderTarget->EndDraw();

        if (hr == D2DERR_RECREATE_TARGET)
        {
            hr = S_OK;
            DiscardDeviceResources();
        }
    }

    return hr;
}

void IQViewer::OnResize(UINT width, UINT height)
{
    if (m_pRenderTarget)
    {
        D2D1_SIZE_U size;
        size.width = width;
        size.height = height;

        m_pRenderTarget->Resize(size);
    }
}

void IQViewer::RunMessageLoop()
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT IQViewer::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        IQViewer *pIQViewer = (IQViewer *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hWnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(pIQViewer)
            );

        result = 1;
    }
    else
    {
        IQViewer *pIQViewer = reinterpret_cast<IQViewer *>(
            ::GetWindowLongPtrW(
                hWnd,
                GWLP_USERDATA
                ));

        bool wasHandled = false;

        if (pIQViewer)
        {
            switch (message)
            {
            case WM_SIZE:
                {
                    UINT width = LOWORD(lParam);
                    UINT height = HIWORD(lParam);
                    pIQViewer->OnResize(width, height);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_PAINT:
            case WM_DISPLAYCHANGE:
                {
                    PAINTSTRUCT ps;
                    BeginPaint(hWnd, &ps);
                    pIQViewer->OnRender();
                    EndPaint(hWnd, &ps);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_DESTROY:
                {
                    PostQuitMessage(0);
                }
                result = 1;
                wasHandled = true;
                break;
            }
        }

        if (!wasHandled)
        {
            result = DefWindowProc(hWnd, message, wParam, lParam);
        }
    }

    return result;
}

int WINAPI WinMain(HINSTANCE hInstance ,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if (SUCCEEDED(CoInitialize(NULL)))
    {
        {
            IQViewer iqViewer;

            if (SUCCEEDED(iqViewer.Initialize()))
            {
                iqViewer.RunMessageLoop();
            }
        }
        CoUninitialize();
    }

    return 0;
}