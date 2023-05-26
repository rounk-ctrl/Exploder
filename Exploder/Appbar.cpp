#include "Appbar.h"
#define APPBAR_CALLBACK     (WM_USER + 1010)
BOOL fOnTop = TRUE;
BOOL uSide = ABE_BOTTOM;
int cxWidth, cyHeight = 32;

BOOL AppBar_Register(HWND hwnd)
{
    APPBARDATA abd;
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hwnd;
    abd.uCallbackMessage = APPBAR_CALLBACK;

    BOOL g_fAppRegistered = (BOOL)SHAppBarMessage(ABM_NEW, &abd);
    return g_fAppRegistered;
}

void AppBar_QueryPos(HWND hwnd, LPRECT lprc)
{
    // Fill out the APPBARDATA struct and save the edge we're moving to
    // in the appbar OPTIONS struct.
    APPBARDATA abd;
    abd.hWnd = hwnd;
    abd.cbSize = sizeof(APPBARDATA);
    abd.rc = *lprc;
    abd.uEdge = uSide;

    int iWidth = 0;
    int iHeight = 0;
    if ((ABE_LEFT == abd.uEdge) || (ABE_RIGHT == abd.uEdge))
    {
        iWidth = abd.rc.right - abd.rc.left;
        abd.rc.top = 0;
        abd.rc.bottom = GetSystemMetrics(SM_CYSCREEN);
    }
    else
    {
        iHeight = abd.rc.bottom - abd.rc.top;
        abd.rc.left = 0;
        abd.rc.right = GetSystemMetrics(SM_CXSCREEN);
    }

    // Ask the system for the screen space
    SHAppBarMessage(ABM_QUERYPOS, &abd);

    switch (abd.uEdge)
    {
    case ABE_LEFT:
        abd.rc.right = abd.rc.left + iWidth;
        break;

    case ABE_RIGHT:
        abd.rc.left = abd.rc.right - iWidth;
        break;

    case ABE_TOP:
        abd.rc.bottom = abd.rc.top + iHeight;
        break;

    case ABE_BOTTOM:
        abd.rc.bottom = GetSystemMetrics(SM_CYSCREEN);
        abd.rc.top = abd.rc.bottom - iHeight;
        break;
    }
    *lprc = abd.rc;
}

void AppBar_QuerySetPos(UINT uEdge, LPRECT lprc, PAPPBARDATA pabd, BOOL fMove)
{
    // Fill out the APPBARDATA struct and save the edge we're moving to
    // in the appbar OPTIONS struct.
    pabd->rc = *lprc;
    pabd->uEdge = uEdge;
    uSide = uEdge;

    AppBar_QueryPos(pabd->hWnd, &(pabd->rc));

    // Tell the system we're moving to this new approved position.
    SHAppBarMessage(ABM_SETPOS, pabd);

    if (fMove)
    {
        // Move the appbar window to the new position
        MoveWindow(pabd->hWnd, pabd->rc.left, pabd->rc.top,
            pabd->rc.right - pabd->rc.left,
            pabd->rc.bottom - pabd->rc.top, TRUE);
    }
}

BOOL AppBar_SetSide(HWND hwnd, UINT uSide)
{
    // Calculate the size of the screen so we can occupy the full width or
    // height of the screen on the edge we request.
    RECT rc;
    rc.top = 0;
    rc.left = 0;
    rc.right = GetSystemMetrics(SM_CXSCREEN);
    rc.bottom = GetSystemMetrics(SM_CYSCREEN);

    // Fill out the APPBARDATA struct with the basic information
    APPBARDATA abd;
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hwnd;

    // If the appbar is autohidden, turn that off so we can move the bar
    BOOL fAutoHide = FALSE;
    // Adjust the rectangle to set our height or width depending on the
    // side we want.
    switch (uSide)
    {
    case ABE_TOP:
        rc.bottom = rc.top + cyHeight;
        break;
    case ABE_BOTTOM:
        rc.top = rc.bottom - cyHeight;
        break;
    case ABE_LEFT:
        rc.right = rc.left + cxWidth;
        break;
    case ABE_RIGHT:
        rc.left = rc.right - cxWidth;
        break;
    }

    // Move the appbar to the new screen space.
    AppBar_QuerySetPos(uSide, &rc, &abd, TRUE);

    return TRUE;
}
