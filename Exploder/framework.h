// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#ifndef _WIN32_IE 
#define _WIN32_IE               0x0500  // Enables all the needed define's in ShellAPI.h
#endif 
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <initguid.h>
#include <list>
#include <string>
#include <shellapi.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <CommCtrl.h>
#include <vector>
#include <ShObjIdl.h>
#include <ShlObj.h>
#include <Uxtheme.h>
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "dwmapi.lib")
#include <initguid.h>
#include <objbase.h>
