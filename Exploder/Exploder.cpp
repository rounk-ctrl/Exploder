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

BOOL AppBar_UnRegister(HWND hwnd)
{
	APPBARDATA abd;
	abd.cbSize = sizeof(APPBARDATA);
	abd.hWnd = hwnd;

	BOOL g_fAppRegistered = !SHAppBarMessage(ABM_REMOVE, &abd);

	return !g_fAppRegistered;
}
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
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(&hwnd));

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_EXPLODER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	if (!underExplorer && FindWindow(L"Progman", NULL) == NULL)
	{
		hShDocVwModule = LoadLibrary(L"SHDOCVW.DLL");
		if (hShDocVwModule) ShellDDEInit_shdocvw = (void(__stdcall*)(bool)) GetProcAddress(hShDocVwModule, (LPSTR)118);
		if (ShellDDEInit_shdocvw) ShellDDEInit_shdocvw(true);
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

	if (underExplorer) {
		ShowWindow(m_hTaskBar, SW_SHOW);
		APPBARDATA msgData;
		msgData.cbSize = sizeof(msgData);
		msgData.hWnd = m_hTaskBar;
		msgData.lParam = 0;
		SHAppBarMessage(ABM_SETSTATE, &msgData);
	}
	CoUninitialize();
	if (hMutex) CloseHandle(hMutex);
	return (int)msg.wParam;
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

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EXPLODER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
		APPBARDATA msgData;
		msgData.cbSize = sizeof(msgData);
		msgData.hWnd = m_hTaskBar;
		msgData.lParam = ABS_AUTOHIDE;
		SHAppBarMessage(ABM_SETSTATE, &msgData);
		SetWindowPos(m_hTaskBar, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	AppBar_Register(hWnd);
	AppBar_SetSide(hWnd, ABE_BOTTOM);
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
	UpdateWindow(hWnd);
	return TRUE;
}

void ShowContextMenu(HWND hwnd, POINT pt, HINSTANCE hInst, WORD i)
{
	hMenu = LoadMenu(hInst, MAKEINTRESOURCE(i));
	if (hMenu)
	{
		HMENU hSubMenu = GetSubMenu(hMenu, 0);
		UINT uFlags = TPM_RIGHTBUTTON;
		if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0)
			uFlags |= TPM_RIGHTALIGN;
		else
			uFlags |= TPM_LEFTALIGN;

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
	}
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
		start = CreateWindow(L"button", L"", BS_NOTIFY | BS_TEXT | WS_CHILD | WS_VISIBLE, 2, 2, 70,
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
			74, 4, screenWidth - (75 * 2), taskbarHeight - 8,
			hWnd, (HMENU)201, hInst, NULL);
		SetWindowTheme(hWndListView, L"Explorer", NULL);
		ListView_SetColumnWidth(hWndListView, 0, 150);
		ListView_SetView(hWndListView, LV_VIEW_TILE);
		ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_TRANSPARENTBKGND);
		LVTILEVIEWINFO tvi;
		tvi.dwFlags = LVTVIF_FIXEDHEIGHT;
		tvi.cbSize = sizeof(tvi);
		tvi.dwMask = LVTVIM_TILESIZE | LVTVIM_COLUMNS;
		tvi.sizeTile.cx = 100;
		tvi.sizeTile.cy = taskbarHeight - 8;
		ListView_SetExtendedListViewStyleEx(hWndListView, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
		ListView_SetTileViewInfo(hWndListView, &tvi);

		unsigned row = ListView_GetItemCount(hWndListView);
		LVITEM item;
		int i = 0;
		item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		item.iSubItem = 0;
		HIMAGELIST hSmall;
		hSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
			GetSystemMetrics(SM_CYSMICON),
			ILC_COLOR32, 1, 1);
		ListView_SetImageList(hWndListView, hSmall, LVSIL_NORMAL);
		for (const auto& wnd : hwnd)
		{
			WCHAR windowTitle[MAX_PATH];
			if (IsWindowVisible(wnd) && ::GetWindowTextLength(wnd) != 0) {
				GetWindowText(wnd, windowTitle, MAX_PATH);
				if (wcscmp(windowTitle, L"Program Manager") && wcscmp(windowTitle, L"Windows Input Experience")
					&& wcscmp(windowTitle, L"Windows Shell Experience Host"))
				{
					item.iItem = row;
					item.pszText = windowTitle;
					item.iImage = i;
					item.lParam = (LPARAM)wnd;
					ListView_InsertItem(hWndListView, &item);

					// icon
					HICON icon = (HICON)SendMessage(wnd, WM_GETICON, ICON_SMALL2, 0);
					if (icon == 0)
						icon = (HICON)SendMessage(wnd, WM_GETICON, ICON_SMALL, 0);
					if (icon == 0)
						icon = (HICON)SendMessage(wnd, WM_GETICON, ICON_BIG, 0);
					if (icon == 0)
						icon = (HICON)GetClassLongPtr(wnd, -14);
					if (icon == 0)
						icon = (HICON)GetClassLongPtr(wnd, -34);
					ImageList_AddIcon(hSmall, icon);
					i++;
				}
			}
		}
		GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, TIME_NOSECONDS, NULL, NULL, times, MAX_PATH);
		SetWindowText(timeWnd, times);
		SetTimer(hWnd, IDT_TIMER1, 1000, (TIMERPROC)NULL);

		// Hide minimized windows...
		MINIMIZEDMETRICS mm;
		ZeroMemory(&mm, sizeof(MINIMIZEDMETRICS));
		mm.cbSize = sizeof(MINIMIZEDMETRICS);
		SystemParametersInfo(SPI_GETMINIMIZEDMETRICS, sizeof(MINIMIZEDMETRICS), &mm, false);
		mm.iArrange = ARW_BOTTOMLEFT | ARW_HIDE; // ARW_HIDE == 8
		SystemParametersInfo(SPI_SETMINIMIZEDMETRICS, sizeof(MINIMIZEDMETRICS), &mm, SPIF_SENDCHANGE);

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
				ok.y = screenHeight - ((taskbarHeight)+2);
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
		if ((LOWORD(wParam) != 199) || (LOWORD(wParam) == 201))
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
		// what
		case NM_DBLCLK:
			return 1;
		case LVN_ITEMCHANGING:
		{
			NMLISTVIEW* pNMListView = (NMLISTVIEW*)lParam;
			if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED))
			{
				HWND wnd = (HWND)pNMListView->lParam;
				if (pNMListView->uOldState & LVIS_SELECTED)
				{
					// temp
					SwitchToThisWindow(wnd, TRUE);
				}
				else
				{
					if (GetForegroundWindow() != wnd)
					{
						SwitchToThisWindow(wnd, TRUE);
						break;
					}
					else if (GetForegroundWindow() == NULL) break;
					else if (GetForegroundWindow() == wnd)
					{
						ShowWindow(wnd, SW_MINIMIZE);
					}
				}
			}
			break;
		}
		}
		break;
	}
	case ID_MENU_EXIT:
	{
		AppBar_UnRegister(hWnd);
		KillTimer(hWnd, IDT_TIMER1);
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
		AppBar_UnRegister(hWnd);
		KillTimer(hWnd, IDT_TIMER1);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam) {
	std::vector<HWND>& titles = *reinterpret_cast<std::vector<HWND>*>(lParam);
	titles.push_back(hwnd);
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
