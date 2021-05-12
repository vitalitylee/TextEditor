#pragma once

#include <Windows.h>
#include <windowsx.h>
#include <strsafe.h>

/**
* 作用：
*	显示最后一次函数调用产生的错误消息。
*
* 参数：
*	lpszFunction
*		最后一次调用的函数名称。
*
*	hParent
*		弹出消息窗体的父窗体，通常情况下，
*		应该指定为我们应用程序的主窗体，这样
*		当消息弹出时，将禁止用户对主窗体进行
*		操作。
*
* 返回值：
*	无
*/
VOID DisplayError(LPWSTR lpszFunction, HWND hParent);