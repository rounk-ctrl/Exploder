#pragma once
#include "framework.h"
#include "Appbar.h"

// useless
DECLARE_INTERFACE_(IDeskTray, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // IDeskTray
    STDMETHOD_(UINT, AppBarGetState)(THIS) PURE;
    STDMETHOD(GetTrayWindow)(THIS_ HWND * phwndTray) PURE;
    STDMETHOD(SetDesktopWindow)(THIS_ HWND hwndDesktop) PURE;
    STDMETHOD(SetVar)(THIS_ int var, DWORD value) PURE;
};

class CDeskTray : IDeskTray 
{
public:
    CDeskTray();

    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG) AddRef(THIS);
    STDMETHODIMP_(ULONG) Release(THIS);

    STDMETHODIMP_(UINT) AppBarGetState(THIS);
    STDMETHODIMP GetTrayWindow(HWND*);
    STDMETHODIMP SetDesktopWindow(HWND);
    STDMETHODIMP SetVar(INT, ULONG);

    HWND desktop;
};
