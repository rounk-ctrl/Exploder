#include "desktray.h"

CDeskTray::CDeskTray()
{
    desktop = NULL;
}
STDMETHODIMP_(HRESULT __stdcall) CDeskTray::QueryInterface(REFIID riid, void** ppvObject)
{
    return this->QueryInterface(riid, ppvObject);
}

STDMETHODIMP_(ULONG __stdcall) CDeskTray::AddRef(void)
{
    return 2;
}

STDMETHODIMP_(ULONG __stdcall) CDeskTray::Release(void)
{
    return 1;
}

STDMETHODIMP_(UINT __stdcall) CDeskTray::AppBarGetState(void)
{
    return ABS_ALWAYSONTOP;
}

STDMETHODIMP_(HRESULT __stdcall) CDeskTray::GetTrayWindow(HWND* wnd)
{
    *wnd = hTray;
    return S_OK;
}

STDMETHODIMP_(HRESULT __stdcall) CDeskTray::SetDesktopWindow(HWND wnd)
{
    desktop = wnd;
    return S_OK;
}

STDMETHODIMP_(HRESULT __stdcall) CDeskTray::SetVar(INT, ULONG)
{
    return S_OK;
}
