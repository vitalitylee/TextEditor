#include "WinTextEditor.h"

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd
) {
	MSG msg;
	BOOL fGotMessage = FALSE;

	if (!InitEnv(hInstance, nShowCmd)) {
		return 0;
	}

	HACCEL hAcc = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR_MAIN));
	if (NULL == hAcc) {
		return 0;
	}

	while ((fGotMessage = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0
		&& fGotMessage != -1)
	{
		if (fGotMessage == -1) {
			break;
		}
		if (!TranslateAccelerator(hMainWindow, hAcc, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}