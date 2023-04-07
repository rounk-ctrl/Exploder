#include "Win32Clock.h"

BOOL ShowLegacyClockExperience(HWND hWnd)
{
    if (!hWnd) return FALSE;
    HRESULT hr = S_OK;
    Win32Clock* pWin32Clock = NULL;
    hr = CoCreateInstance(
        (REFCLSID)GUID_Win32Clock,
        NULL,
        CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
        (REFIID)IID_Win32Clock,
        (void**)&pWin32Clock
    );
    if (SUCCEEDED(hr))
    {
        RECT rc;
        GetWindowRect(hWnd, &rc);
        pWin32Clock->lpVtbl->ShowWin32Clock(pWin32Clock, hWnd, &rc);
        pWin32Clock->lpVtbl->Release(pWin32Clock);
    }
    return TRUE;
}