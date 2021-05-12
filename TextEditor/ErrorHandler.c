#include "ErrorHandler.h"

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
VOID DisplayError(LPWSTR lpszFunction, HWND hParent) {
	LPVOID lpMsgBuff = NULL;
	LPVOID lpDisplayBuff = NULL;
	DWORD  errCode = GetLastError();

	if (!FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuff,
		0,
		NULL
	)) {
		return;
	}
	lpDisplayBuff = LocalAlloc(
		LMEM_ZEROINIT,
		(((lstrlenW((LPCTSTR)lpMsgBuff)))
			+ lstrlenW((LPCTSTR)lpszFunction)
			+ 40
			) * sizeof(TCHAR)
	);
	if (NULL == lpDisplayBuff) {
		MessageBox(
			hParent,
			TEXT("LocalAlloc failed."),
			TEXT("ERR"),
			MB_OK
		);
		goto RETURN;
	}

	if (FAILED(
		StringCchPrintf(
			(LPTSTR)lpDisplayBuff,
			LocalSize(lpDisplayBuff) / sizeof(TCHAR),
			TEXT("%s failed with error code %d as follows:\n%s"),
			lpszFunction,
			errCode,
			(LPTSTR)lpMsgBuff
		)
	)) {
		goto EXIT;
	}

	MessageBox(hParent, lpDisplayBuff, TEXT("ERROR"), MB_OK);
EXIT:
	LocalFree(lpDisplayBuff);
RETURN:
	LocalFree(lpMsgBuff);
}