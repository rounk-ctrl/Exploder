#pragma once
#include "framework.h"

// implementation of Win32Clock
DEFINE_GUID(GUID_Win32Clock,
    0x0A323554A,
    0x0FE1, 0x4E49, 0xae, 0xe1,
    0x67, 0x22, 0x46, 0x5d, 0x79, 0x9f
);
DEFINE_GUID(IID_Win32Clock,
    0x7A5FCA8A,
    0x76B1, 0x44C8, 0xa9, 0x7c,
    0xe7, 0x17, 0x3c, 0xca, 0x5f, 0x4f
);
typedef interface Win32Clock Win32Clock;
typedef struct Win32ClockVtbl
{
    BEGIN_INTERFACE

        HRESULT(STDMETHODCALLTYPE* QueryInterface)(
            Win32Clock* This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */
            _COM_Outptr_  void** ppvObject);

    ULONG(STDMETHODCALLTYPE* AddRef)(
        Win32Clock* This);

    ULONG(STDMETHODCALLTYPE* Release)(
        Win32Clock* This);

    HRESULT(STDMETHODCALLTYPE* ShowWin32Clock)(
        Win32Clock* This,
        /* [in] */ HWND hWnd,
        /* [in] */ LPRECT lpRect);

    END_INTERFACE
} Win32ClockVtbl;

interface Win32Clock
{
    CONST_VTBL struct Win32ClockVtbl* lpVtbl;
};
BOOL ShowLegacyClockExperience(HWND hWnd);
