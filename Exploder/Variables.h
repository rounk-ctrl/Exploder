#pragma once
#include "framework.h"

#define MAX_LOADSTRING 100

int taskbarHeight = 32;
int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);
LPWSTR times = new WCHAR[MAX_PATH];
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND m_hTaskBar;
HWND start;
HWND timeWnd;
UINT menuItemId;
POINT ok;
HWND hWndListView;
BOOL underExplorer;
HMODULE hShDocVwModule;
std::vector<HWND> hwnd;
HMENU hMenu;
HANDLE hMutex;

typedef void(__stdcall* RUNDLGPROC)(HWND, HICON, LPCSTR, LPCSTR, LPCSTR, int);
typedef void(__stdcall* SHUTDOWNDLGPROC)(HWND, int);
void(__stdcall* ShellDDEInit_shdocvw)(bool bInit) = 0;
RUNDLGPROC RunDlg = NULL;
SHUTDOWNDLGPROC ShutdownDlg = NULL;
