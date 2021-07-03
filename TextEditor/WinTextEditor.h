#pragma once

#include <Windows.h>
#include <windowsx.h>
#include <strsafe.h>

#include <stdlib.h>
#include <ShlObj.h>

#include "resource.h"

extern HWND hMainWindow;

BOOL InitEnv(HINSTANCE hInstance, int nShowCmd);