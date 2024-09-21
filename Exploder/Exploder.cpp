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

DWORD WINAPI DesktopThreadProc(LPVOID lpParam)
{
	SHDesktopMessageLoop(thingtrturn);
	return S_OK;
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

	if (!underExplorer)
	{
		hShDocVwModule = GetModuleHandle(L"Shdocvw.dll");
		if (!hShDocVwModule) hShDocVwModule = LoadLibrary(L"SHDOCVW.DLL");
		if (hShDocVwModule) ShellDDEInit = (ShellDDEInit_t)GetProcAddress(hShDocVwModule, (LPSTR)118);
		if (ShellDDEInit) ShellDDEInit(TRUE);

		CDeskTray tray = CDeskTray();
		thingtrturn= SHCreateDesktopOrig(&tray);
		hwnd_desktop = FindWindow(L"Progman", L"Program Manager");
		SetProp(hTray, L"TaskbarMonitor", (HANDLE)MonitorFromWindow(hTray, MONITOR_DEFAULTTOPRIMARY));

		//init desktop	
		PostMessage(hwnd_desktop, 0x45C, 1, 1); //wallpaper
		PostMessage(hwnd_desktop, 0x45E, 0, 2); //wallpaper host
		PostMessage(hwnd_desktop, 0x45C, 2, 3); //wallpaper & icons
		PostMessage(hwnd_desktop, 0x45B, 0, 0); //final init
		PostMessage(hwnd_desktop, 0x40B, 0, 0); //pins

		HANDLE hThread = CreateThread(NULL, 0, DesktopThreadProc, NULL, 0, NULL);
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

	if (ShellDDEInit) ShellDDEInit(false);
	if (hShDocVwModule) FreeLibrary(hShDocVwModule);

	if (underExplorer) {
		ShowWindow(m_hTaskBar, SW_SHOW);
		APPBARDATA msgData;
		msgData.cbSize = sizeof(msgData);
		msgData.hWnd = m_hTaskBar;
		msgData.lParam = 0;
		SHAppBarMessage(ABM_SETSTATE, &msgData);
	}
	if (RegisterShellHook) RegisterShellHook(m_hTaskBar, 0);
	CoUninitialize();
	if (hMutex) CloseHandle(hMutex);
	return (int)msg.wParam;
}

HMENU CreateMenuFromShellFolder(IShellFolder* pShellFolder, HWND hwndOwner, HIMAGELIST hImageList)
{
	// Create a new menu
	HMENU hMenu = CreatePopupMenu();
	if (!hMenu)
		return NULL;

	// Get an enumerator for the items in the folder
	IEnumIDList* pEnum = NULL;
	int i = 1000;
	HRESULT hr = pShellFolder->EnumObjects(hwndOwner, SHCONTF_NONFOLDERS | SHCONTF_FOLDERS, &pEnum);
	if (SUCCEEDED(hr))
	{
		LPITEMIDLIST pidl;
		while (pEnum->Next(1, &pidl, NULL) == S_OK)
		{
			STRRET str;
			hr = pShellFolder->GetDisplayNameOf(pidl, SHGDN_NORMAL, &str);
			if (SUCCEEDED(hr))
			{
				// Convert STRRET to a string
				char displayName[MAX_PATH];
				if (str.uType == STRRET_WSTR)
				{
					WideCharToMultiByte(CP_UTF8, 0, str.pOleStr, -1, displayName, sizeof(displayName), NULL, NULL);
				}
				else if (str.uType == STRRET_CSTR)
				{
					strcpy(displayName, str.cStr);
				}

				SHFILEINFOA sfi = { 0 };
				SHGetFileInfoA((LPCSTR)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_SYSICONINDEX);
				int imageIndex = ImageList_AddIcon(hImageList, sfi.hIcon);
				DestroyIcon(sfi.hIcon);


				// Create the menu item
				MENUITEMINFOA mii = { sizeof(MENUITEMINFO) };
				mii.fMask = MIIM_STRING | MIIM_ID | MIIM_SUBMENU | MIIM_BITMAP | MIIM_DATA;
				mii.wID = i; // Use a unique ID for the item
				mii.dwTypeData = displayName;
				mii.hbmpItem = HBMMENU_CALLBACK;
				mii.dwItemData = (ULONG_PTR)imageIndex;

				// Check if it's a folder
				IShellFolder* pSubFolder = NULL;
				hr = pShellFolder->BindToObject(pidl, NULL, IID_IShellFolder, (LPVOID*)&pSubFolder);
				if (SUCCEEDED(hr))
				{
					HMENU hSubMenu = CreateMenuFromShellFolder(pSubFolder, hwndOwner, hImageList);
					if (hSubMenu)
					{
						mii.hSubMenu = hSubMenu;
						mii.fMask |= MIIM_SUBMENU;
					}
					pSubFolder->Release();
				}

				InsertMenuItemA(hMenu, -1, TRUE, &mii);
				i++;
			}
			CoTaskMemFree(pidl);
		}
		pEnum->Release();
	}

	return hMenu;
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
		SHCreateDesktopOrig = (SHCreateDesktopAPI)GetProcAddress(hShell32Module, (LPSTR)200);
		SHDesktopMessageLoop = (SHCreateDesktopAPI)GetProcAddress(hShell32Module, (LPSTR)201);
	}
	HMODULE user32 = LoadLibrary(L"user32.dll");
	if (user32)
	{
		IsShellFrameWindow = (IsShellWindow_t)GetProcAddress(user32, (LPCSTR)2573);
	}

	RegisterShellHook = (FARPROC(__stdcall*) (HWND, DWORD))GetProcAddress(hShell32Module, (LPCSTR)((long)0xb5));
	WM_ShellHook = RegisterWindowMessage(L"SHELLHOOK");
	RegisterShellHook(NULL, true);
	RegisterShellHook(hWnd, 3);

	int exStyle = (int)GetWindowLongPtr(hWnd, -20);
	exStyle |= 128;
	SetWindowLongPtr(hWnd, -20, exStyle);
	UpdateWindow(hWnd);
	hTray = hWnd;
	return TRUE;
}
HRESULT BindToDesktop(LPCITEMIDLIST pidl, IShellFolder** ppsfResult)
{
	HRESULT hr;
	IShellFolder* psfDesktop;
	*ppsfResult = NULL;
	hr = SHGetDesktopFolder(&psfDesktop);
	if (FAILED(hr))
		return hr;
	hr = psfDesktop->BindToObject(pidl, NULL, IID_PPV_ARGS(ppsfResult));
	return hr;
}
static HRESULT GetMergedFolder(int folder1, int folder2, IShellFolder** ppsfStartMenu)
{
	HRESULT hr;
	LPITEMIDLIST pidlUserStartMenu;
	LPITEMIDLIST pidlCommonStartMenu;
	IShellFolder* psfUserStartMenu = nullptr;
	IShellFolder* psfCommonStartMenu = nullptr;
	IAugmentedShellFolder* pasf = nullptr;
	*ppsfStartMenu = NULL;
	hr = CoCreateInstance(CLSID_MergedFolder, 0LL, 1u, GUID_2f711b17_773c_41d4_93fa_7f23edcecb66, (LPVOID*)&pasf);
	hr = SHGetSpecialFolderLocation(NULL, folder1, &pidlUserStartMenu);
	if (FAILED(hr))
	{
		hr = SHGetSpecialFolderLocation(NULL, folder2, &pidlCommonStartMenu);
		if (FAILED(hr))
			return hr;
		hr = BindToDesktop(pidlCommonStartMenu, ppsfStartMenu);
		ILFree(pidlCommonStartMenu);
		return hr;
	}
	hr = SHGetSpecialFolderLocation(NULL, folder2, &pidlCommonStartMenu);
	if (FAILED(hr))
	{
		hr = BindToDesktop(pidlUserStartMenu, ppsfStartMenu);
		ILFree(pidlUserStartMenu);
		return hr;
	}
	hr = BindToDesktop(pidlUserStartMenu, &psfUserStartMenu);
	if (FAILED(hr))
		return hr;
	hr = BindToDesktop(pidlCommonStartMenu, &psfCommonStartMenu);
	if (FAILED(hr))
		return hr;
	GUID GUID_NULL = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
	hr = pasf->AddNameSpace(&GUID_NULL, psfUserStartMenu, pidlUserStartMenu, 0x0FF0A, 2);
	if (FAILED(hr))
		return hr;
	hr = pasf->AddNameSpace(&GUID_NULL, psfCommonStartMenu, pidlCommonStartMenu, 0x0E, 2);
	if (FAILED(hr))
		return hr;
	*ppsfStartMenu = pasf;
	ILFree(pidlCommonStartMenu);
	ILFree(pidlUserStartMenu);
	return hr;
}

void ShowContextMenu(HWND hwnd, POINT pt, HINSTANCE hInst, WORD i, HMENU sec, LPARAM lp)
{
	HMENU hMenu;
	if (sec == NULL) hMenu = LoadMenu(hInst, MAKEINTRESOURCE(i));
	else hMenu = sec;
	if (hMenu)
	{
		HMENU hSubMenu;
		if (sec == NULL) hSubMenu = GetSubMenu(hMenu, 0);
		else hSubMenu = sec;

		UINT uFlags = TPM_RIGHTBUTTON;
		if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0) uFlags |= TPM_RIGHTALIGN;
		else uFlags |= TPM_LEFTALIGN;

		uFlags |= TPM_RETURNCMD;

		if (i == IDR_START && sec == NULL)
			Button_SetState(start, TRUE);

		if (i == IDR_START)
		{
			//new
			IShellFolder* shell;
			hml = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
				GetSystemMetrics(SM_CYSMICON),
				ILC_COLOR32, 1, 1);
			HRESULT result = GetMergedFolder(CSIDL_PROGRAMS, CSIDL_COMMON_PROGRAMS, &shell);
			HMENU hMenutoAdd = CreateMenuFromShellFolder(shell, NULL, hml);

			MENUITEMINFO mii = { 0 };
			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID;
			mii.fType = MFT_STRING;
			mii.dwTypeData = (LPWSTR)L"Programs";
			mii.hSubMenu = hMenutoAdd;
			mii.wID = 50998;

			InsertMenuItem(hSubMenu, 0, TRUE, &mii);
			shell->Release();
		}

		if (hSubMenu != 0)
			menuItemId = TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hwnd, NULL);

		if (i == IDR_START && sec == NULL)
			Button_SetState(start, FALSE);

		if (menuItemId != 0)
		{
			if (sec != NULL)
				SendMessage((HWND)lp, WM_SYSCOMMAND, menuItemId, NULL);
			else
				SendMessage(hwnd, menuItemId, NULL, NULL);
		}
	}
}

bool isWindow(HWND wnd)
{
	if (!IsWindow(wnd))
		return FALSE;

	if (IsShellFrameWindow)
	{
		if (IsShellFrameWindow(wnd))
			return TRUE;
	}

	if (GetWindowTextLength(wnd) == 0)
		return FALSE;

	if (!IsWindowVisible(wnd))
		return FALSE;

	BOOL bCloaked;
	DwmGetWindowAttribute(wnd, DWMWA_CLOAKED, &bCloaked, sizeof(BOOL));
	if (bCloaked)
		return FALSE;

	return TRUE;

}

void AddHWND(HWND wnd)
{
	LVITEM item;
	item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	item.iSubItem = 0;
	unsigned row = ListView_GetItemCount(hWndListView);

	WCHAR windowTitle[MAX_PATH];
	if (isWindow(wnd))
	{
		GetWindowText(wnd, windowTitle, MAX_PATH);
		if (wcscmp(windowTitle,L"Program Manager"))
		{

			item.iItem = row;
			item.pszText = windowTitle;
			item.iImage = row;
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

			HICON hIcon = ImageList_GetIcon(hSmall, row, ILD_NORMAL);
			if (hIcon != NULL)
			{
				// hack
				ImageList_ReplaceIcon(hSmall, row, icon);
				DestroyIcon(hIcon);
			}
			else
			{
				ImageList_AddIcon(hSmall, icon);
			}
		}
	}
}

void RemoveHWND(HWND wnd)
{
	int count = ListView_GetItemCount(hWndListView);
	for (int i = 0; i < count; i++)
	{
		LVITEM lv;
		lv.iItem = i;
		lv.mask = LVIF_PARAM | LVIF_IMAGE;
		ListView_GetItem(hWndListView, &lv);
		if ((HWND)lv.lParam == wnd)
		{
			ListView_DeleteItem(hWndListView, i);
			//ImageList_Remove(hSmall, i);
			ListView_Arrange(hWndListView, LVA_ALIGNLEFT);
			count--;
			i--;
		}
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
		start = CreateWindow(L"button", L"", BS_NOTIFY | BS_TEXT | WS_CHILD | WS_VISIBLE, -1,-1, 65,
			taskbarHeight+2, hWnd, (HMENU)199, hInst, NULL);

		HBITMAP hbit = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_START));
		SendMessage(start, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hbit);
		WCHAR szStart[MAX_LOADSTRING];
		LoadString(hInst, IDS_START, szStart, MAX_LOADSTRING);
		SetWindowText(start, szStart);

		timeWnd = CreateWindow(L"static", L"time",
			SS_NOTIFY | SS_SUNKEN | SS_CENTER | WS_CHILD | SS_CENTERIMAGE | WS_VISIBLE,
			screenWidth - 68, 3, 64, taskbarHeight - 6, hWnd, (HMENU)200, hInst, NULL);

		INITCOMMONCONTROLSEX icex;
		icex.dwICC = ICC_LISTVIEW_CLASSES;
		InitCommonControlsEx(&icex);

		hWndListView = CreateWindow(WC_LISTVIEW, L"",
			WS_CHILD | LVS_REPORT | WS_VISIBLE | LVS_NOSCROLL | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
			68, 1, screenWidth - (75 * 2), taskbarHeight - 2,
			hWnd, (HMENU)201, hInst, NULL);
		SetWindowTheme(hWndListView, L"Explorer", NULL);
		ListView_SetView(hWndListView, LV_VIEW_TILE);
		ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_TRANSPARENTBKGND);
		LVTILEVIEWINFO tvi;
		tvi.dwFlags = LVTVIF_FIXEDSIZE;
		tvi.cbSize = sizeof(tvi);
		tvi.dwMask = LVTVIM_TILESIZE;
		tvi.sizeTile.cx = 150;
		tvi.sizeTile.cy = taskbarHeight -2;
		ListView_SetTileViewInfo(hWndListView, &tvi);
		ListView_SetExtendedListViewStyleEx(hWndListView, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

		hSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
			GetSystemMetrics(SM_CYSMICON),
			ILC_COLOR32, 1, 1);
		ListView_SetImageList(hWndListView, hSmall, LVSIL_NORMAL);
		for (const auto& wnd : hwnd)
		{
			AddHWND(wnd);
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
				ShowContextMenu(hWnd, ok, hInst, IDR_START, NULL, NULL);
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
		break;
	}
	case WM_RBUTTONUP:
	{
		if ((LOWORD(wParam) != 199) || (LOWORD(wParam) == 201))
		{
			GetCursorPos(&ok);
			ShowContextMenu(hWnd, ok, hInst, IDR_PROP, NULL, NULL);
			return 0;
		}
	}
	//return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_NOTIFY:
	{
		LPNMHDR lpnmh = (LPNMHDR)lParam;
		switch (lpnmh->code)
		{
		case NM_RCLICK:
		{
			GetCursorPos(&ok);
			ScreenToClient(lpnmh->hwndFrom, &ok);
			LVHITTESTINFO hitTestInfo;
			hitTestInfo.pt = ok;
			hitTestInfo.flags = LVHT_ONITEM;
			int itemIndex = ListView_HitTest(lpnmh->hwndFrom, &hitTestInfo);
			if (itemIndex != -1 && (hitTestInfo.flags & LVHT_ONITEM))
			{
				LVITEM lv;
				lv.iItem = itemIndex;
				lv.mask = LVIF_PARAM;
				ListView_GetItem(hWndListView, &lv);
				HMENU hSysMenu = GetSystemMenu((HWND)lv.lParam, FALSE);
				if (hSysMenu)
				{
					GetCursorPos(&ok);
					ShowContextMenu(hWnd, ok, hInst, NULL, hSysMenu, lv.lParam);

				}
			}
			else
			{
				GetCursorPos(&ok);
				ShowContextMenu(hWnd, ok, hInst, IDR_PROP, NULL, NULL);
			}

			return 0;
		}
		// what
		case NM_DBLCLK:
			return 1;
		case LVN_ITEMCHANGED:
		{
			NMLISTVIEW* pNMListView = (NMLISTVIEW*)lParam;
			if (((pNMListView->uNewState & LVIS_FOCUSED)))
			{
				HWND wnd = (HWND)pNMListView->lParam;

				if (g_prevSelectedWindow == wnd)
				{
					ShowWindow(wnd, SW_MINIMIZE);
					g_prevSelectedWindow = NULL;
				}
				else
				{
					SwitchToThisWindow(wnd, TRUE);
					g_prevSelectedWindow = wnd;
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
	{
		if (message == WM_ShellHook)
		{
			if (wParam == HSHELL_WINDOWCREATED)
				AddHWND((HWND)lParam);
			else if (wParam == HSHELL_WINDOWDESTROYED)
				RemoveHWND((HWND)lParam);
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
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
