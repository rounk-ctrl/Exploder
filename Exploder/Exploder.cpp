// Exploder.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Exploder.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK       EnumWindowsCallback(HWND, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    hMutex = CreateMutex(NULL, false, L"Exploder");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(hMutex);
        return 0;
    }

    EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(&titles));

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_EXPLODER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!underExplorer && FindWindow(L"Progman", NULL) == NULL)
    {
        hShDocVwModule = LoadLibrary(L"SHDOCVW.DLL");
        if (hShDocVwModule) ShellDDEInit_shdocvw = (void(__stdcall*)(bool)) GetProcAddress(hShDocVwModule, (LPSTR)118);
        if (ShellDDEInit_shdocvw) ShellDDEInit_shdocvw(true);
    }

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

   // HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EXPLODER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        //if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        //{
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        //}
    }

    if (ShellDDEInit_shdocvw) ShellDDEInit_shdocvw(false);
    if (hShDocVwModule) FreeLibrary(hShDocVwModule);

    if (underExplorer) ShowWindow(m_hTaskBar, SW_SHOW);
    CloseHandle(hMutex);
    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EXPLODER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
    wcex.lpszMenuName   = 0;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowExW(
       WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
       szWindowClass,
       NULL,
       WS_POPUP,
       0, screenHeight - taskbarHeight, screenWidth, taskbarHeight,
       NULL,
       NULL,
       hInstance,
       NULL);
   if (!hWnd)
   {
      return FALSE;
   }
   m_hTaskBar = FindWindow(L"Shell_TrayWnd", NULL);
   if (FindWindow(L"Shell_TrayWnd", NULL)) underExplorer = true;
   else underExplorer = false;
   
   if (underExplorer)
   {
       /*
       APPBARDATA msgData;
       msgData.cbSize = sizeof(msgData);
       msgData.hWnd = m_hTaskBar;
       msgData.lParam = ABS_AUTOHIDE;
       SHAppBarMessage(ABM_SETSTATE, &msgData);
       */
       ShowWindow(m_hTaskBar, SW_HIDE);
   }

   ShowWindow(hWnd, nCmdShow);
   HMODULE hShell32Module = GetModuleHandle(L"SHELL32.DLL");
   if (!hShell32Module) hShell32Module = LoadLibrary(L"SHELL32.DLL");
   if (hShell32Module)
   {
       RunDlg = (RUNDLGPROC)GetProcAddress(hShell32Module, (LPCSTR)MAKELPARAM(61, 0));
       ShutdownDlg = (SHUTDOWNDLGPROC)GetProcAddress(hShell32Module, (LPCSTR)MAKELPARAM(60, 0));
   }

   int exStyle = (int)GetWindowLongPtr(hWnd, -20);
   exStyle |= 128;
   SetWindowLongPtr(hWnd, -20, exStyle);
   RECT rekt;
   rekt.top = 0;
   rekt.left = 0;
   rekt.right = screenWidth;
   rekt.bottom = screenHeight - taskbarHeight;
   SystemParametersInfo(SPI_SETWORKAREA, 0, &rekt, SPIF_UPDATEINIFILE);

   // Hide minimized windows...
   MINIMIZEDMETRICS mm;
   ZeroMemory(&mm, sizeof(MINIMIZEDMETRICS));
   mm.cbSize = sizeof(MINIMIZEDMETRICS);
   SystemParametersInfo(SPI_GETMINIMIZEDMETRICS, sizeof(MINIMIZEDMETRICS), &mm, false);
   mm.iArrange |= ARW_HIDE; // ARW_HIDE == 8
   SystemParametersInfo(SPI_SETMINIMIZEDMETRICS, sizeof(MINIMIZEDMETRICS), &mm, false);
   UpdateWindow(hWnd);
   return TRUE;
}

void ShowContextMenu(HWND hwnd, POINT pt, HINSTANCE hInst, WORD i)
{
	hMenu = LoadMenu(hInst, MAKEINTRESOURCE(i));
	if (hMenu)
	{
		HMENU hSubMenu = GetSubMenu(hMenu, 0);
		// respect menu drop alignment
		UINT uFlags = TPM_RIGHTBUTTON;
		if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0)
			uFlags |= TPM_RIGHTALIGN;
		else
			uFlags |= TPM_LEFTALIGN;

		// Use TPM_RETURNCMD flag let TrackPopupMenuEx function return the menu item identifier of the user's selection in the return value.
		uFlags |= TPM_RETURNCMD;

        if (i == IDR_START)
        {
            Button_SetState(start, TRUE);
        }

		if (hSubMenu != 0)
			menuItemId = TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hwnd, NULL);

        if (i == IDR_START)
        {
            Button_SetState(start, FALSE);
        }
        
        SendMessage(hwnd, menuItemId, NULL, NULL);
		//DestroyMenu(hMenu);
	}
}

COLORREF GetBrushColor(HBRUSH brush)
{
    LOGBRUSH lbr;
    if (GetObject(brush, sizeof(lbr), &lbr) != sizeof(lbr)) {
        // Not even a brush!
        return CLR_NONE;
    }
    if (lbr.lbStyle != BS_SOLID) {
        // Not a solid color brush.
        return CLR_NONE;
    }
    return lbr.lbColor;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        start = CreateWindow(L"button", L"", BS_NOTIFY | BS_TEXT | WS_CHILD |  WS_VISIBLE, 2, 2, 70,
            taskbarHeight - 4, hWnd, (HMENU)199, hInst, NULL);

        HBITMAP hbit = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_START));
        SendMessage(start, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hbit);
        WCHAR szStart[MAX_LOADSTRING];
        LoadString(hInst, IDS_START, szStart, MAX_LOADSTRING);
        SetWindowText(start, szStart);

        timeWnd = CreateWindow(L"static", L"time", 
            SS_NOTIFY | SS_SUNKEN | SS_CENTER | WS_CHILD | SS_CENTERIMAGE | WS_VISIBLE,
            screenWidth - 72, 3, 70, taskbarHeight - 6, hWnd, (HMENU)200, hInst, NULL);

        INITCOMMONCONTROLSEX icex;
        icex.dwICC = ICC_LISTVIEW_CLASSES;
        InitCommonControlsEx(&icex);

        hWndListView = CreateWindow(WC_LISTVIEW, L"",
            WS_CHILD | LVS_LIST | WS_VISIBLE | LVS_NOSCROLL | LVS_NOLABELWRAP | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
            74, 4, screenWidth - (75*2), taskbarHeight - 8,
            hWnd, (HMENU)201, hInst, NULL);
        SetWindowTheme(hWndListView, L"Explorer", NULL);
        ListView_SetColumnWidth(hWndListView, 0, 150);
        ListView_SetView(hWndListView, LV_VIEW_TILE);
        ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_TRANSPARENTBKGND);
        LVTILEVIEWINFO tvi;
        tvi.dwFlags = LVTVIF_FIXEDHEIGHT;
        tvi.cbSize = sizeof(tvi);
        tvi.dwMask = LVTVIM_TILESIZE | LVTVIM_COLUMNS;
        tvi.sizeTile.cx = 150;
        tvi.sizeTile.cy = taskbarHeight - 8;
        ListView_SetExtendedListViewStyleEx(hWndListView, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
        ListView_SetTileViewInfo(hWndListView, &tvi);
       
        unsigned row = ListView_GetItemCount(hWndListView);
        LVITEM item;
        item.mask = LVIF_TEXT;
        item.iSubItem = 0;
        for (const auto& title : titles)
        {
            item.iItem = row;
            item.pszText = (LPWSTR)title.c_str();
            ListView_InsertItem(hWndListView, &item);
        }
        GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, TIME_NOSECONDS, NULL, NULL, times, MAX_PATH);
        SetWindowText(timeWnd, times);
        SetTimer(hWnd, IDT_TIMER1, 1000, (TIMERPROC)NULL);

        // tell shell that it is ready
        HANDLE hShellReadyEvent = OpenEvent(EVENT_MODIFY_STATE, false, L"msgina: ShellReadyEvent");
        if (hShellReadyEvent != NULL)
        {
            SetEvent(hShellReadyEvent);
            CloseHandle(hShellReadyEvent);
        }
        break;
    }
    case WM_COMMAND:
    {
        switch (HIWORD(wParam))
        {
            case STN_CLICKED:
            {
                if (LOWORD(wParam) == 200)
                {
                    ShowLegacyClockExperience(hWnd);
                    return 0;
                }
                else if (LOWORD(wParam) == 199)
                {
                    ok.x = 2;
                    ok.y = screenHeight - ((taskbarHeight) + 2);
                    ShowContextMenu(hWnd, ok, hInst, IDR_START);
                    return 0;
                }
            }
            case STN_DBLCLK:
            {
                /*
                ShellExecuteA(0,"open","rundll32.exe","shell32.dll,Control_RunDLL timedate.cpl",NULL,0);
                */
                return 0;
            }
        }
        case WM_RBUTTONUP:
        {
            if((LOWORD(wParam) != 199) || (LOWORD(wParam) == 201))
            {
                GetCursorPos(&ok);
                ShowContextMenu(hWnd, ok, hInst, IDR_PROP);
                return 0;
            }
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    case WM_NOTIFY:
    {
        switch (((LPNMHDR)lParam)->code)
        {
            case NM_RCLICK:
            {
                GetCursorPos(&ok);
                ShowContextMenu(hWnd, ok, hInst, IDR_PROP);
                return 0;
            }
        }
        break;
    }
    case ID_MENU_EXIT:
    {
        PostQuitMessage(0);
        break;
    }
    case ID_MENU_SHUTDOWN:
    {
        ShutdownDlg(0, 0);
        break;
    }
    case ID_MENU_RUN:
    {
        RunDlg(hWnd, NULL, NULL, NULL, NULL, 0);
        break;
    }
    case WM_TIMER:
    {
        switch (wParam)
        {
            case IDT_TIMER1:
            {
                GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, TIME_NOSECONDS, NULL, NULL, times, MAX_PATH);
                SetWindowText(timeWnd, times);
                return 0;
            }
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam) {
    const DWORD TITLE_SIZE = 1024;
    WCHAR windowTitle[TITLE_SIZE];
    GetWindowText(hwnd, windowTitle, TITLE_SIZE);
    int length = ::GetWindowTextLength(hwnd);
    std::wstring title(&windowTitle[0]);
    if (!IsWindowVisible(hwnd) || length == 0 || title == L"Program Manager") {
        return TRUE;
    }
    std::vector<std::wstring>& titles = *reinterpret_cast<std::vector<std::wstring>*>(lParam);
    titles.push_back(title);
    return TRUE;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
