#pragma once
#include "framework.h"

BOOL AppBar_Register(HWND hwnd);
void AppBar_QueryPos(HWND hwnd, LPRECT lprc);
void AppBar_QuerySetPos(UINT uEdge, LPRECT lprc, PAPPBARDATA pabd, BOOL fMove);
BOOL AppBar_SetSide(HWND hwnd, UINT uSide);