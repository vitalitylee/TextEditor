#pragma once

#include <Windows.h>
#include <windowsx.h>
#include <strsafe.h>

#include <stdlib.h>
#include <ShlObj.h>

#include "ErrorHandler.h"

// 第一个参数为 PWSTR，第二个参数为 HWND，没有返回值的函数指针
typedef VOID(*Func_PWSTR_HWND)(PWSTR parameter, HWND hwnd, HWND hTextEditor);

extern LPWSTR currentFileName;

/**
* 作用：
*	选择一个文件，选择成功之后，调用传入的回调函数 pfCallback
*
* 参数：
*	pfCallback
*		当用户成功选择一个文件，并获取到文件路径之后，本函数
*		将回调 pfCallback 函数指针指向的函数，并将获取到的文
*		路径作为参数传入。
*
*	hwnd
*		打开文件对话框的父窗体句柄。
*
*	hTextEditor
*		文本编辑框的句柄。
*
* 返回值：
*	无
*/
VOID EditFile(Func_PWSTR_HWND pfCallback, HWND hwnd, HWND hTextEditor);

/**
* 作用：
*	如果当前已经有了打开的文件，并且内容已经被修改，
*	则弹出对话框，让用户确认是否保存以打开文件，并打开
*	新文件。
*	如果当前没有已打开文件或者当前已打开文件未修改，
*	则直接打开传入路径指定文件。
*
* 参数：
*	fileName
*		要新打开的目标文件路径。
*
*	hwnd
*		弹出对话框时，指定的父窗体，对于本应用来说，
*		应该为主窗体的句柄。
*
*	hTextEditor
*		文本编辑器的句柄。
*
* 返回值：
*	无
*/
VOID OpenNewFile(PWSTR fileName, HWND hwnd, HWND hTextEditor);

/**
* 作用：
*	保存当前已经打开的文件，如果当前没有已打开文件，
*	则调用另存为逻辑。
*
* 参数：
*	hwnd
*		出现错误时，本函数会弹出对话框，
*		此参数为对话框的父窗体句柄。
*
*	hTextEditor
*		文本编辑器的句柄。
*
* 返回值：
*	无
*/
VOID SaveFile(HWND hwnd, HWND hTextEditor);

/**
* 作用：
*	将当前内容保存到 fileName，并且设置 currentFileName
*	为 fileName。
*
* 参数：
*	fileName
*		要将当前内容保存到的目标路径
*
*	hwnd
*		出错弹出消息框时，消息框的父窗体句柄。
*
*	hTextEditor
*		文本编辑器的句柄。
* 返回值：
*	无
*/
VOID SaveFileTo(PWSTR fileName, HWND hwnd, HWND hTextEditor);

/**
* 作用：
*	弹出另存为对话框，在用户选择一个文件路径之后，
*	回调 pfCallback 函数指针指向的函数。
*
* 参数：
*	pfCallback
*		一个函数指针，用于执行用户选择一个保存路径
*		之后的操作。
*
*	hwnd
*		出错情况下，弹出错误对话框的父窗体句柄。
*
*	hTextEditor
*		文本编辑框的句柄。
*
* 返回值：
*	无
*/
VOID SaveFileAs(Func_PWSTR_HWND pfCallback, HWND hwnd, HWND hTextEditor);