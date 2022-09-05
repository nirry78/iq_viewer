#include "IQViewer.h"

IQViewer::IQViewer():
    m_hwnd(NULL),
    m_pD2DFactory(NULL),
    m_pRenderTarget(NULL),
    m_pBlackBrush(NULL),
    m_pGrayBrush(NULL),
    m_pYellowBrush(NULL),
    m_pRedBrush(NULL),
    m_pStrokeStyleDotRound(NULL),
    m_IQData(NULL),
    m_ServerSocket(INVALID_SOCKET),
    m_ServerHandle(NULL)
{

}

IQViewer::~IQViewer()
{
    SafeRelease(&m_pD2DFactory);
    DiscardDeviceResources();

    if (m_IQData)
    {
        delete m_IQData;
    }

    if (m_ServerSocket != INVALID_SOCKET)
    {
        shutdown(m_ServerSocket, SD_BOTH);
        closesocket(m_ServerSocket);
    }

    if (m_ServerHandle != NULL)
    {
        CloseHandle(m_ServerHandle);
    }
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

        if (SUCCEEDED(hr))
        {
            // Create a yellow brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Yellow),
                &m_pYellowBrush
                );
        }

        if (SUCCEEDED(hr))
        {
            // Create a yellow brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Red),
                &m_pRedBrush
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
    SafeRelease(&m_pYellowBrush);
    SafeRelease(&m_pRedBrush);
    SafeRelease(&m_pStrokeStyleDotRound);
}

HRESULT IQViewer::DrawGraph(D2D1_RECT_F rect, IQData *iqData, const ValueType valueTypes[])
{
    HRESULT hr = S_OK;
    float left_margin = 30.0f;
    float right_margin = 15.0f;
    float top_margin = 15.0f;
    float bottom_margin = 15.0f;
    float strokeWidth = 1.2f;
    float view_height = ((rect.bottom - bottom_margin) - (rect.top + top_margin));
    float view_width = ((rect.right - right_margin) - (rect.left + left_margin));
    float cell_height = view_height / 6;
    float cell_width = view_width / 10;
    float middle_y = cell_height * 3 + rect.top + top_margin;

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

    double i, i_prev, q, q_prev, power, power_prev;
    float scale_x, scale_y;

    if (iqData)
    {
        bool valueTypeI = false, valueTypeQ = false, valueTypePower = false;
        scale_y = view_height / 500;
        scale_x = view_width / iqData->GetCount();

        for (size_t index = 0; valueTypes[index] != ValueTypeNone ; index++)
        {
            switch (valueTypes[index])
            {
                case ValueTypeI:
                {
                    valueTypeI = true;
                    break;
                }
                case ValueTypeQ:
                {
                    valueTypeQ = true;
                    break;
                }
                case ValueTypePower:
                {
                    valueTypePower = true;
                    break;
                }
                case ValueTypeDemod:
                {
                    break;
                }
                case ValueTypePhase:
                {
                    break;
                }
                case ValueTypeUnwrappedPhase:
                {
                    break;
                }
            }
        }

        for (size_t index = 0; index < iqData->GetCount(); index++)
        {
            if (valueTypeI && iqData->GetValue(index, ValueTypeI, &i))
            {
                if (index > 0)
                {
                    m_pRenderTarget->DrawLine(D2D1::Point2F(rect.left + left_margin + (float)index * scale_x, middle_y - (float)(i_prev * scale_y)),
                                              D2D1::Point2F(rect.left + left_margin + (float)(index + 1) * scale_x, middle_y - (float)(i * scale_y)),
                                              m_pYellowBrush,
                                              strokeWidth);
                }

                i_prev = i;
            }

            if (valueTypeQ && iqData->GetValue(index, ValueTypeQ, &q))
            {
                if (index > 0)
                {
                    m_pRenderTarget->DrawLine(D2D1::Point2F(rect.left + left_margin + (float)index * scale_x, middle_y - (float)(q_prev * scale_y)),
                                              D2D1::Point2F(rect.left + left_margin + (float)(index + 1) * scale_x, middle_y - (float)(q * scale_y)),
                                              m_pRedBrush,
                                              strokeWidth);
                }
                q_prev = q;
            }

            if (valueTypePower && iqData->GetValue(index, ValueTypePower, &power))
            {
                if (index > 0)
                {
                    m_pRenderTarget->DrawLine(D2D1::Point2F(rect.left + left_margin + (float)index * scale_x, middle_y - (float)(power_prev * scale_y)),
                                              D2D1::Point2F(rect.left + left_margin + (float)(index + 1) * scale_x, middle_y - (float)(power * scale_y)),
                                              m_pRedBrush,
                                              strokeWidth);
                }
                power_prev = power;
            }
        }
    }
    return hr;
}

HRESULT IQViewer::Initialize()
{
    HRESULT hr;

    m_IQData = new IQData(512);
    if (m_IQData)
    {
        for (size_t index = 0; index < 512; index++)
        {
            double i = 240.0 * sin((double)index / 90.0 * M_PI);
            double q = 240.0 * cos((double)index / 90.0 * M_PI);
            m_IQData->AddValue(i, q);
        }
    }

    hr = StartServer();
    if (SUCCEEDED(hr))
    {
        // Initialize device-indpendent resources, such
        // as the Direct2D factory.
        hr = CreateDeviceIndependentResources();
    }

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
        m_hwnd = CreateWindow(
            L"IQViewer",
            L"IQ Viewer",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            NULL,
            NULL,
            HINST_THISCOMPONENT,
            this
            );
        hr = m_hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            float dpi = (float)GetDpiForWindow(m_hwnd);

            // Because the CreateWindow function takes its size in pixels, we
            // obtain the system DPI and use it to scale the window size.
            SetWindowPos(m_hwnd, NULL, 0, 0,
                         static_cast<INT>(ceil(1024.f * dpi / 96.f)),
                         static_cast<INT>(ceil(768.f * dpi / 96.f)),
                         0);

            ShowWindow(m_hwnd, SW_SHOWNORMAL);
            UpdateWindow(m_hwnd);
        }

        if (SUCCEEDED(hr))
        {
            ACCEL accel[1];

            accel[0].fVirt = FCONTROL | FVIRTKEY;
            accel[0].key = 0x56;
            accel[0].cmd = ID_PASTE;

            m_accel = CreateAcceleratorTable(accel, sizeof(accel) / sizeof(accel[0]));
        }
    }

    return hr;
}

void IQViewer::OnAccelerator(IQViewerCommand command)
{
    switch (command)
    {
        case ID_PASTE:
        {
            if (IsClipboardFormatAvailable(CF_TEXT) && OpenClipboard(m_hwnd))
            {
                HGLOBAL hglb = GetClipboardData(CF_TEXT);
                if (hglb)
                {
                    LPTSTR lptstr = (LPTSTR)GlobalLock(hglb);
                    if (lptstr)
                    {
                        size_t size = wcslen(lptstr);
                        wchar_t buffer[100];
                        swprintf(buffer, L"Paste (len: %zu)\n", size);
                        OutputDebugString(buffer);
                        GlobalUnlock(hglb);
                    }
                }
                CloseClipboard();
            }
            break;
        }
    }
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
            const ValueType valueTypesIQ[3] = {ValueTypeI, ValueTypeQ, ValueTypeNone};
            const ValueType valueTypesPower[2] = {ValueTypePower, ValueTypeNone};
            DrawGraph(D2D1::RectF(0.0f, 0.0f, size.width, size.height / 2.0f), m_IQData, valueTypesIQ);
            DrawGraph(D2D1::RectF(0.0f, size.height / 2.0f, size.width, size.height), m_IQData, valueTypesPower);
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

bool IQViewer::OnServerEvent()
{
    WSANETWORKEVENTS events;
    bool result = true;

    if (SOCKET_ERROR != WSAEnumNetworkEvents(m_ServerSocket, m_ServerHandle, &events))
    {
        if (events.lNetworkEvents & FD_ACCEPT)
        {
            sockaddr_in clientAddress;
            int clientAddressLen = sizeof(clientAddress);
            SOCKET clientSocket;

            clientSocket = accept(m_ServerSocket, (sockaddr*)&clientAddress, &clientAddressLen);
            if (clientSocket != INVALID_SOCKET)
            {
                shutdown(clientSocket, SD_BOTH);
                closesocket(clientSocket);
            }
        }
    }
    else
    {
        result = false;
    }

    return result;
}

void IQViewer::RunMessageLoop()
{
    MSG msg;
    HANDLE handleList[2];
    DWORD result;
    DWORD count = 1;

    handleList[0] = m_ServerHandle;

    for (;;)
    {
        result = MsgWaitForMultipleObjects(1, handleList, false, INFINITE, QS_ALLEVENTS);
        if (result == WAIT_OBJECT_0)
        {
            if (!OnServerEvent())
            {
                break;
            }
        }
        else if (result == WAIT_OBJECT_0 + count)
        {
            if (GetMessage(&msg, NULL, 0, 0) > 0)
            {
                if (!TranslateAccelerator(m_hwnd, m_accel, &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            else
            {
                break;
            }
        }
    }
    OutputDebugString(TEXT("RunMessageLoop - Exit\n"));
}

HRESULT IQViewer::StartServer()
{
    HRESULT hr = S_OK;

    m_ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_ServerSocket == INVALID_SOCKET)
    {
        hr = HRESULT_FROM_WIN32(WSAGetLastError());
    }

    if (SUCCEEDED(hr))
    {
        sockaddr_in sa;
        sa.sin_family = AF_INET;
        sa.sin_port = htons(7612);
        sa.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

        if (bind(m_ServerSocket, (const sockaddr *)&sa, sizeof(sa)) == SOCKET_ERROR)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    if (SUCCEEDED(hr))
    {
        if (listen(m_ServerSocket, 10) == SOCKET_ERROR)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    if (SUCCEEDED(hr))
    {
        m_ServerHandle = WSACreateEvent();
        if (m_ServerHandle == NULL)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    if (SUCCEEDED(hr))
    {
        if (WSAEventSelect(m_ServerSocket, m_ServerHandle, FD_ACCEPT) == SOCKET_ERROR)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    return hr;
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
            case WM_COMMAND:
            {
                if (HIWORD(wParam) == 1)
                {
                    pIQViewer->OnAccelerator((IQViewerCommand)LOWORD(wParam));
                }
                wasHandled = true;
                break;
            }
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
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

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

    WSACleanup();

    return 0;
}
