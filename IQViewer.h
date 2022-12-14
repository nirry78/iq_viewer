#ifndef _IQ_VIEWER_H
#define _IQ_VIEWER_H

#include "Platform.h"
#include "IQData.h"
#include "IQDebug.h"

typedef enum {
    ID_PASTE,
} IQViewerCommand;

class IQViewer
{
    private:
        HWND m_hwnd;
        HACCEL m_accel;
        ID2D1Factory *m_pD2DFactory;
        ID2D1HwndRenderTarget *m_pRenderTarget;
        ID2D1SolidColorBrush *m_pGraphColor1Brush;
        ID2D1SolidColorBrush *m_pGraphColor2Brush;
        ID2D1SolidColorBrush *m_pYellowBrush;
        ID2D1SolidColorBrush *m_pRedBrush;
        ID2D1SolidColorBrush *m_pGraphBrush;
        ID2D1StrokeStyle *m_pStrokeStyleDotRound;
        IQData *m_IQData;
        SOCKET m_ServerSocket;
        HANDLE m_ServerHandle;

        HRESULT CreateDeviceIndependentResources();
        HRESULT CreateDeviceResources();
        void DiscardDeviceResources();
        HRESULT DrawGraph(D2D1_RECT_F rect, IQData *iqData, const ValueType valueTypes[]);
        void OnAccelerator(IQViewerCommand command);
        HRESULT OnRender();
        void OnResize(UINT width, UINT height);
        bool OnServerEvent();
        HRESULT StartServer();
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    public:
        IQViewer();
        virtual ~IQViewer();

        HRESULT Initialize();
        void RunMessageLoop();
};

#endif
