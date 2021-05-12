#include "WinTextEditor.h"
#include "File.h"
#include "ErrorHandler.h"

BOOL CreateMainWindow(HINSTANCE hInstance, int cmdShow);
BOOL InitMainWindowClass(HINSTANCE hInstance);

LPCWSTR mainWIndowClassName = L"TextEditorMainWindow";
HANDLE gHFont = NULL;
HWND hTextEditor = NULL;
BOOL textChanged = FALSE;

BOOL InitEnv(HINSTANCE hInstance, int nShowCmd) {
	if (!InitMainWindowClass(hInstance)
		|| !CreateMainWindow(hInstance, nShowCmd)) {
		return FALSE;
	}
	return TRUE;
}

void MainWindow_Cls_OnDestroy(HWND hwnd) {
	if (gHFont) {
		DeleteObject(gHFont);
		gHFont = NULL;
	}

	PostQuitMessage(0);
}

/**
* 作用：
*	创建编辑器使用的字体，这里默认为 "Courier New"
*
* 参数：
*	无
*
* 返回值：
*	新建字体的句柄。
*/
HANDLE CreateDefaultFont() {
	LOGFONT lf;
	ZeroMemory(&lf, sizeof(lf));

	// 设置字体为Courier New
	lf.lfHeight = 16;
	lf.lfWidth = 8;
	lf.lfWeight = 400;
	lf.lfOutPrecision = 3;
	lf.lfClipPrecision = 2;
	lf.lfQuality = 1;
	lf.lfPitchAndFamily = 1;
	StringCchCopy((STRSAFE_LPWSTR)&lf.lfFaceName, 32, L"Courier New");

	return CreateFontIndirect(&lf);
}

/**
* 作用：
*	创建编辑器窗体
*
* 参数：
*	hInstance
*		当前应用程序实例的句柄
*
*	hParent
*		当前控件的所属父窗体
*
* 返回值：
*	创建成功，返回新建编辑器的句柄，否则返回 NULL。
*/
HWND CreateTextEditor(
	HINSTANCE hInstance, HWND hParnet) {
	RECT rect;
	HWND hEdit;

	// 获取窗体工作区的大小，以备调整编辑控件的大小
	GetClientRect(hParnet, &rect);

	hEdit = CreateWindowEx(
		0,
		TEXT("EDIT"),
		TEXT(""),
		WS_CHILDWINDOW |
		WS_VISIBLE |
		WS_VSCROLL |
		ES_LEFT |
		ES_MULTILINE |
		ES_NOHIDESEL,
		0,
		0,
		rect.right,
		rect.bottom,
		hParnet,
		NULL,
		hInstance,
		NULL
	);

	gHFont = CreateDefaultFont();
	if (NULL != gHFont) {
		// 设置文本编辑器的字体。并且在设置之后立刻重绘。
		SendMessage(hEdit, WM_SETFONT, (WPARAM)gHFont, TRUE);
	}

	return hEdit;
}

BOOL MainWindow_Cls_OnCreate(
	HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
	return NULL != (
		hTextEditor = CreateTextEditor(
			GetWindowInstance(hwnd), hwnd)
		);
}

/**
* 作用：
*	处理主窗体的菜单命令
*
* 参数：
*	hwnd
*		主窗体的句柄
*	id
*		点击菜单的ID
*
*	hwndCtl
*		如果消息来自一个控件，则此值为该控件的句柄，
*		否则这个值为 NULL
*
*	codeNotify
*		如果消息来自一个控件，此值表示通知代码，如果
*		此值来自一个快捷菜单，此值为1，如果消息来自菜单
*		此值为0
*
* 返回值：
*	无
*/
void MainWindow_Cls_OnCommand(
	HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
	switch (id) {
	case ID_OPEN:
		EditFile(OpenNewFile, hwnd, hTextEditor);
		textChanged = FALSE;
		break;
	case ID_SAVE:
		SaveFile(hwnd, hTextEditor);
		textChanged = FALSE;
		break;
	case ID_SAVE_AS:
		SaveFileAs(SaveFileTo, hwnd, hTextEditor);
		textChanged = FALSE;
		break;
	case ID_EXIT:
		MainWindow_Cls_OnDestroy(hwnd);
		break;
	default:
		if (hwndCtl != NULL) {
			switch (codeNotify)
			{
			case EN_CHANGE:
				if (!textChanged && currentFileName != NULL) {
					SendMessage(
						hwnd,
						WM_SETTEXT,
						0,
						(LPARAM)((((PWCHAR)currentFileName)) - 1)
					);
				}
				textChanged = TRUE;
				break;
			default:
				break;
			}
		}
		break;
	}
}

/**
* 作用：
*	处理主窗体的大小变更事件，这里只是调整文本编辑器
*	的大小。
*
* 参数:
*	hwnd
*		主窗体的句柄
*
*	state
*		窗体大小发生变化的类型，如：最大化，最小化等
*
*	cx
*		窗体工作区的新宽度
*
*	cy
*		窗体工作区的新高度
*
* 返回值：
*	无
*/
VOID MainWindow_Cls_OnSize(
	HWND hwnd, UINT state, int cx, int cy) {
	MoveWindow(
		hTextEditor,
		0,
		0,
		cx,
		cy,
		TRUE
	);
}


/**
* 作用：
*	主窗体消息处理函数
*
* 参数：
*	hWnd
*		消息目标窗体的句柄。
*	msg
*		具体的消息的整型值定义，要了解系统
*		支持的消息列表，请参考 WinUser.h 中
*		以 WM_ 开头的宏定义。
*
*	wParam
*		根据不同的消息，此参数的意义不同，
*		主要用于传递消息的附加信息。
*
*	lParam
*		根据不同的消息，此参数的意义不同，
*		主要用于传递消息的附加信息。
*
* 返回值：
*	本函数返回值根据发送消息的不同而不同，
*	具体的返回值意义，请参考 MSDN 对应消息
*	文档。
*/
LRESULT CALLBACK mainWindowProc(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam) {
	switch (msg) {
	case WM_DESTROY:
		return HANDLE_WM_DESTROY(
			hWnd,
			wParam,
			lParam,
			MainWindow_Cls_OnDestroy
		);
	case WM_CREATE:
		return HANDLE_WM_CREATE(
			hWnd, wParam, lParam, MainWindow_Cls_OnCreate
		);
	case WM_COMMAND:
		return HANDLE_WM_COMMAND(
			hWnd, wParam, lParam, MainWindow_Cls_OnCommand
		);
	case WM_SIZE:
		// 主窗体大小发生变化，我们要调整编辑控件大小。
		return HANDLE_WM_SIZE(
			hWnd, wParam, lParam, MainWindow_Cls_OnSize);
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

/**
* 作用：
*   注册主窗体类型。
*
* 参数：
*   hInstance
*       当前应用程序的实例句柄，通常情况下在
*       进入主函数时，由操作系统传入。
*
* 返回值：
*   类型注册成功，返回 TRUE，否则返回 FALSE。
*/
BOOL InitMainWindowClass(HINSTANCE hInstance) {
	WNDCLASSEX wcx;
	// 在初始化之前，我们先将结构体的所有字段
	// 均设置为 0.
	ZeroMemory(&wcx, sizeof(wcx));

	// 标识此结构体的大小，用于属性扩展。
	wcx.cbSize = sizeof(wcx);
	// 当窗体的大小发生改变时，重绘窗体。
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	// 在注册窗体类型时，要设置一个窗体消息
	// 处理函数，以处理窗体消息。
	// 如果此字段为 NULL，则程序运行时会抛出
	// 空指针异常。
	wcx.lpfnWndProc = mainWindowProc;
	// 设置窗体背景色为白色。
	wcx.hbrBackground = GetStockObject(WHITE_BRUSH);
	// 指定主窗体类型的名称，之后创建窗体实例时
	// 也需要传入此名称。
	wcx.lpszClassName = mainWIndowClassName;
	// 将主窗体的菜单设置为主菜单
	wcx.lpszMenuName = MAKEINTRESOURCE(IDR_MENU_MAIN);

	return RegisterClassEx(&wcx) != 0;
}

/**
* 作用：
*	创建一个主窗体的实例，并显示。
*
* 参数：
*	hInstance
*		当前应用程序的实例句柄。
*
*	cmdShow
*		控制窗体如何显示的一个标识。
*
* 返回值：
*	创建窗体成功，并成功显示成功，返回 TRUE，
*	否则返回 FALSE。
*/
BOOL CreateMainWindow(HINSTANCE hInstance, int cmdShow) {
	HWND mainWindowHwnd = NULL;
	// 创建一个窗体对象实例。
	mainWindowHwnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		mainWIndowClassName,
		TEXT("TextEditor"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (NULL == mainWindowHwnd) {
		DisplayError(TEXT("CreateWindowEx"), NULL);
		return FALSE;
	}

	// 由于返回值只是标识窗体是否已经显示，对于我们
	// 来说没有意义，所以这里丢弃返回值。
	ShowWindow(mainWindowHwnd, cmdShow);

	if (!UpdateWindow(mainWindowHwnd)) {
		DisplayError(TEXT("UpdateWindow"), mainWindowHwnd);
		return FALSE;
	}

	return TRUE;
}