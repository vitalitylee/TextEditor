#include "File.h"

/**
* 作用：
*	将给定的 byte 数组中的 bSize 个子接，写入 file 指定
*	的文件中。
*
* 参数：
*	bytes
*		要写入目标文件的 byte 数组。
*
*	bSize
*		要写入目标文件的字节数量。
*
*	file
*		要写入内容的目标文件名。
*
*	hwnd
*		出现错误时，本函数会弹出对话框，
*		此参数为对话框的父窗体句柄。
*
* 返回值：
*	无
*/
VOID WriteBytesToFile(
	PBYTE bytes,
	size_t bSize,
	PWSTR file,
	HWND hwnd
);

LPWSTR currentFileName = NULL;

// 支持的编辑文件类型，当前我们只支持文本文件(*.txt).
COMDLG_FILTERSPEC SUPPORTED_FILE_TYPES[] = {
	{ TEXT("text"), TEXT("*.txt") }
};

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
VOID EditFile(Func_PWSTR_HWND pfCallback, HWND hwnd, HWND hTextEditor) {
	// 每次调用之前，应该先初始化 COM 组件环境
	HRESULT hr = CoInitializeEx(
		NULL,
		COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE
	);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog* pFileOpen = NULL;

		// 创建一个 FileOpenDialog 实例
		hr = CoCreateInstance(
			&CLSID_FileOpenDialog,
			NULL,
			CLSCTX_ALL,
			&IID_IFileOpenDialog,
			&pFileOpen
		);

		if (SUCCEEDED(hr))
		{
			// 设置打开文件扩展名
			pFileOpen->lpVtbl->SetFileTypes(
				pFileOpen,
				_countof(SUPPORTED_FILE_TYPES),
				SUPPORTED_FILE_TYPES
			);
			// 显示选择文件对话框
			hr = pFileOpen->lpVtbl->Show(pFileOpen, hwnd);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = pFileOpen->lpVtbl->GetResult(pFileOpen, &pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->lpVtbl->GetDisplayName(
						pItem, SIGDN_FILESYSPATH, &pszFilePath);

					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						if (pfCallback) {
							pfCallback(pszFilePath, hwnd, hTextEditor);
						}
						CoTaskMemFree(pszFilePath);
					}
					pItem->lpVtbl->Release(pItem);
				}
			}
			pFileOpen->lpVtbl->Release(pFileOpen);
		}
		CoUninitialize();
	}
}

/**
* 作用：
*	从默认进程堆中分配给定大小的内存，大小的单位为 BYTE。
*	如，要分配 100 byte 的内存，可以通过如下方式调用：
*		NewMemory(100, NULL)
*
* 参数：
*	size
*		以 byte 为单位的内存大小。
*
*	hwnd
*		如果分配出错，弹出消息框的父窗体句柄。
*
* 返回值：
*	如果内存分配成功，返回分配内存的起始指针，否则返回 NULL。
*/
PBYTE NewMemory(size_t size, HWND hwnd) {
	HANDLE processHeap;
	PBYTE buff = NULL;
	if ((processHeap = GetProcessHeap()) == NULL) {
		DisplayError(TEXT("GetProcessHeap"), hwnd);
		return buff;
	}

	buff = (PBYTE)HeapAlloc(processHeap, HEAP_ZERO_MEMORY, size);
	if (NULL == buff) {
		// 由于 HeapAlloc 函数不设置错误码，所以这里
		// 只能直接弹出一个错误消息，但是并不知道具体
		// 错误原因。
		MessageBox(
			hwnd,
			TEXT("alloc memory error."),
			TEXT("Error"),
			MB_OK
		);
	}
	return buff;
}

/**
* 作用：
*	从内存 buff 中读取字符串，并将其转换为 UTF16 编码，
*	返回编码后的宽字符字符。
*
* 参数：
*	buff
*		文本原始内容。
*
*	hwnd
*		操作出错时，弹框的父窗体句柄。
*
* 返回值：
*	无论原始内容是否为 UTF16 编码字符串，本函数均会
*	重新分配内存，并返回新内存。
*/
PTSTR Normalise(PBYTE buff, HWND hwnd) {
	PWSTR pwStr;
	PTSTR ptText;
	size_t size;

	pwStr = (PWSTR)buff;
	// 检查BOM头
	if (*pwStr == 0xfffe || *pwStr == 0xfeff) {
		// 如果是大端序，要转换为小端序
		if (*pwStr == 0xfffe) {
			WCHAR wc;
			for (; (wc = *pwStr); pwStr++) {
				*pwStr = (wc >> 8) | (wc << 8);
			}
			// 跳过 BOM 头
			pwStr = (PWSTR)(buff + 2);
		}
		size = (wcslen(pwStr) + 1) * sizeof(WCHAR);
		ptText = (PWSTR)NewMemory(size, hwnd);
		if (!ptText) {
			return NULL;
		}
		memcpy_s(ptText, size, pwStr, size);
		return ptText;
	}

	size =
		MultiByteToWideChar(
			CP_UTF8,
			0,
			buff,
			-1,
			NULL,
			0
		);

	ptText = (PWSTR)NewMemory(size * sizeof(WCHAR), hwnd);

	if (!ptText) {
		return NULL;
	}

	MultiByteToWideChar(
		CP_UTF8,
		0,
		buff,
		-1,
		ptText,
		size
	);

	return ptText;
}

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
VOID OpenNewFile(PWSTR fileName, HWND hwnd, HWND hTextEditor) {
	LARGE_INTEGER size;
	PBYTE buff = NULL;
	HANDLE processHeap = NULL;
	DWORD readSize = 0;
	HANDLE hFile = CreateFile(
		fileName,
		GENERIC_ALL,
		0,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (INVALID_HANDLE_VALUE == hFile) {
		DisplayError(TEXT("CreateFile"), hwnd);
		return;
	}

	if (!GetFileSizeEx(hFile, &size)) {
		DisplayError(TEXT("GetFileSizeEx"), hwnd);
		goto Exit;
	}

	if ((processHeap = GetProcessHeap()) == NULL) {
		DisplayError(TEXT("GetProcessHeap"), hwnd);
		goto Exit;
	}

	buff = (PBYTE)HeapAlloc(
		processHeap,
		HEAP_ZERO_MEMORY,
		(SIZE_T)(size.QuadPart + 8));
	if (NULL == buff) {
		MessageBox(
			hwnd,
			TEXT("alloc memory error."),
			TEXT("Error"),
			MB_OK
		);
		goto Exit;
	}

	if (!ReadFile(
		hFile, buff,
		(DWORD)size.QuadPart,
		&readSize,
		NULL
	)) {
		MessageBox(
			hwnd,
			TEXT("ReadFile error."),
			TEXT("Error"),
			MB_OK
		);
		goto FreeBuff;
	}

	// 因为对话框关闭之后，将会释放掉文件路径的内存
	// 所以这里，我们重新分配内存，并拷贝一份路径
	// 在这之前，需要判断当前文件名是否指向了一个地址，
	// 如果有指向，应将其释放。
	if (currentFileName) {
		HeapFree(GetProcessHeap(), 0, currentFileName - 1);
	}
	size_t bsize = (wcslen(fileName) + 2) * sizeof(WCHAR);
	currentFileName = (PWSTR)NewMemory(bsize, hwnd);
	if (!currentFileName) {
		goto FreeBuff;
	}
	currentFileName[0] = (WCHAR)'*';
	currentFileName = ((PWCHAR)currentFileName) + 1;

	StringCbCopy(currentFileName, bsize, fileName);

	PTSTR str = Normalise(buff, hwnd);
	SendMessage(hTextEditor, WM_SETTEXT, 0, (WPARAM)str);
	SendMessage(hwnd, WM_SETTEXT, 0, (WPARAM)currentFileName);

	if (str) {
		HeapFree(processHeap, 0, str);
	}

FreeBuff:
	HeapFree(processHeap, 0, buff);

Exit:
	CloseHandle(hFile);
}

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
VOID SaveFile(HWND hwnd, HWND hTextEditor) {
	size_t cch = 0;
	size_t bSize = 0;
	PWCHAR buffWStr = NULL;
	PBYTE utf8Buff = NULL;

	// 如果当前没有打开任何文件，则调用另存为逻辑，
	// 让用户选择一个文件名进行保存，然后退出。
	if (!currentFileName) {
		SaveFileAs(SaveFileTo, hwnd, hTextEditor);
		return;
	}

	// 获取文本编辑器的文本字符数量。
	cch = SendMessage(
		hTextEditor, WM_GETTEXTLENGTH, 0, 0);
	// 获取字符时，我们是通过 UTF16 格式（WCHAR)获取，
	// 我们要在最后添加一个空白结尾标志字符
	buffWStr = (PWCHAR)NewMemory(
		cch * sizeof(WCHAR) + sizeof(WCHAR), hwnd);

	if (buffWStr == NULL) {
		return;
	}
	// 获取到编辑器的文本
	SendMessage(
		hTextEditor,
		WM_GETTEXT,
		cch + 1,
		(WPARAM)buffWStr
	);

	// 获取将文本以 UTF8 格式编码后所需的内存大小（BYTE）
	bSize = WideCharToMultiByte(
		CP_UTF8,
		0,
		buffWStr,
		cch,
		NULL,
		0,
		NULL,
		NULL
	);

	utf8Buff = NewMemory(bSize, hwnd);
	if (utf8Buff == NULL) {
		goto Exit;
	}
	// 将文本格式化到目标缓存
	WideCharToMultiByte(
		CP_UTF8,
		0,
		buffWStr,
		cch,
		utf8Buff,
		bSize,
		NULL,
		NULL
	);

	// 将内容覆盖到目标文件。
	WriteBytesToFile(
		utf8Buff, bSize, currentFileName, hwnd);

	// 保存完成之后，设置文本变更标识为 FALSE，
	// 并设置主窗体标题为当前文件路径。
	SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)currentFileName);

	HeapFree(GetProcessHeap(), 0, utf8Buff);
Exit:
	HeapFree(GetProcessHeap(), 0, buffWStr);
}

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
VOID SaveFileTo(PWSTR fileName, HWND hwnd, HWND hTextEditor) {
	size_t len = lstrlenW(fileName) + 1;
	int bSize = len * sizeof(WCHAR);
	int appendSuffix = !(
		fileName[len - 4] == '.' &&
		fileName[len - 3] == 't' &&
		fileName[len - 2] == 'x' &&
		fileName[len - 1] == 't');

	if (appendSuffix) {
		bSize += 5 * sizeof(WCHAR);
	}

	if (currentFileName) {
		HeapFree(GetProcessHeap(), 0, currentFileName - 1);
		currentFileName = NULL;
	}

	currentFileName = (PWSTR)NewMemory(bSize, hwnd);
	if (!currentFileName) {
		return;
	}
	currentFileName[0] = (WCHAR)'*';
	currentFileName = currentFileName + 1;
	StringCbCopy(currentFileName, bSize, fileName);
	if (appendSuffix) {
		currentFileName[len + 0] = '.';
		currentFileName[len + 1] = 't';
		currentFileName[len + 2] = 'x';
		currentFileName[len + 3] = 't';
		currentFileName[len + 4] = '\0';
	}

	SaveFile(hwnd, hTextEditor);
}

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
VOID SaveFileAs(Func_PWSTR_HWND pfCallback, HWND hwnd, HWND hTextEditor) {
	// 每次调用之前，应该先初始化 COM 组件环境
	HRESULT hr = CoInitializeEx(
		NULL,
		COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE
	);
	if (SUCCEEDED(hr))
	{
		IFileSaveDialog* pFileSave = NULL;

		// 创建一个 FileOpenDialog 实例
		hr = CoCreateInstance(
			&CLSID_FileSaveDialog,
			NULL,
			CLSCTX_ALL,
			&IID_IFileSaveDialog,
			&pFileSave
		);

		if (SUCCEEDED(hr))
		{
			// 设置打开文件扩展名
			pFileSave->lpVtbl->SetFileTypes(
				pFileSave,
				_countof(SUPPORTED_FILE_TYPES),
				SUPPORTED_FILE_TYPES
			);
			// 显示选择文件对话框
			hr = pFileSave->lpVtbl->Show(pFileSave, hwnd);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = pFileSave->lpVtbl->GetResult(pFileSave, &pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->lpVtbl->GetDisplayName(
						pItem, SIGDN_FILESYSPATH, &pszFilePath);

					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						if (pfCallback) {
							pfCallback(pszFilePath, hwnd, hTextEditor);
						}
						CoTaskMemFree(pszFilePath);
					}
					pItem->lpVtbl->Release(pItem);
				}
			}
			pFileSave->lpVtbl->Release(pFileSave);
		}
		CoUninitialize();
	}
}

/**
* 作用：
*	将给定的 byte 数组中的 bSize 个子接，写入 file 指定
*	的文件中。
*
* 参数：
*	bytes
*		要写入目标文件的 byte 数组。
*
*	bSize
*		要写入目标文件的字节数量。
*
*	file
*		要写入内容的目标文件名。
*
*	hwnd
*		出现错误时，本函数会弹出对话框，
*		此参数为对话框的父窗体句柄。
*
* 返回值：
*	无
*/
VOID WriteBytesToFile(
	PBYTE bytes,
	size_t bSize,
	PWSTR file,
	HWND hwnd
) {
	DWORD numberOfBytesWritten = 0;
	HANDLE hFile = CreateFile(
		file,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (INVALID_HANDLE_VALUE == hFile) {
		DisplayError(TEXT("CreateFile"), hwnd);
		return;
	}

	if (!WriteFile(
		hFile,
		bytes,
		bSize,
		&numberOfBytesWritten,
		NULL
	)) {
		DisplayError(TEXT("WriteFile"), hwnd);
		goto Exit;
	}

Exit:
	CloseHandle(hFile);
}