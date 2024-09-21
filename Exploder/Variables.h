#pragma once
#define INITGUID
#include "framework.h"
DEFINE_GUID(CLSID_MergedFolder, 0x26FDC864, 0x0BE88, 0x46E7, 0x92, 0x35, 0x3, 0x2D, 0x8E, 0x0A5, 0x16, 0x2E);
DEFINE_GUID(GUID_2f711b17_773c_41d4_93fa_7f23edcecb66, 0x2f711b17, 0x773c, 0x41d4, 0x93, 0xfa, 0x7f, 0x23, 0xed, 0xce, 0xcb, 0x66);
#define MAX_LOADSTRING 100

int taskbarHeight = 24;
int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);
LPWSTR times = new WCHAR[MAX_PATH];
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND start;
HWND timeWnd;
UINT menuItemId;
POINT ok;
HWND hWndListView;
HIMAGELIST hml;
BOOL underExplorer;
HMODULE hShDocVwModule;
std::vector<HWND> hwnd;
HWND hwnd_desktop;
HANDLE hMutex;
HIMAGELIST hSmall;
HANDLE thingtrturn;
static HWND m_hTaskBar;

static HWND g_prevSelectedWindow = NULL;
unsigned int WM_ShellHook = 0;

typedef void(__stdcall* RUNDLGPROC)(HWND, HICON, LPCSTR, LPCSTR, LPCSTR, int);
typedef void(__stdcall* SHUTDOWNDLGPROC)(HWND, int);
typedef BOOL(WINAPI* IsShellWindow_t)(HWND);
FARPROC(__stdcall* RegisterShellHook) (HWND, DWORD) = NULL;
typedef void(__stdcall* ShellDDEInit_t)(bool bInit);
typedef HWND(WINAPI* SHCreateDesktopAPI)(PVOID);
typedef DWORD(WINAPI* SHPtrParamAPI)(PVOID);

static SHCreateDesktopAPI SHCreateDesktopOrig;
static SHCreateDesktopAPI SHDesktopMessageLoop;
RUNDLGPROC RunDlg = NULL;
SHUTDOWNDLGPROC ShutdownDlg = NULL;
IsShellWindow_t IsShellFrameWindow = NULL;
ShellDDEInit_t ShellDDEInit = NULL;
