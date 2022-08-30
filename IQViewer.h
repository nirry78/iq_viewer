#ifndef _IQ_VIEWER_H
#define _IQ_VIEWER_H

#include "Platform.h"

class IQViewer
{
    private:
        HWND m_hwnd;
        ID2D1Factory *m_pD2DFactory;
        ID2D1HwndRenderTarget *m_pRenderTarget;
        ID2D1SolidColorBrush *m_pBlackBrush;

        HRESULT CreateDeviceIndependentResources();
        HRESULT CreateDeviceResources();
        void DiscardDeviceResources();
        HRESULT OnRender();
        void OnResize(UINT width, UINT height);
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    public:
        IQViewer();
        virtual ~IQViewer();

        HRESULT Initialize();
        void RunMessageLoop();
};

#endif
