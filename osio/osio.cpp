#ifndef UNICODE
#define UNICODE
#endif 

#include "framework.h"
#include "resource.h"
#include <windows.h>

BOOL CALLBACK mainWindowHandle(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR strCmdLine, int nCmdShow)
{
	DialogBox(hInstance,
		MAKEINTRESOURCE(IDD_MAIN),
		NULL,
		mainWindowHandle);

	return 0;
}

/*---------------------------------------------------------------------------*/

BOOL CALLBACK mainWindowHandle(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
		//case WM_INITDIALOG:
		//	return SetDlgMsgResult(hWnd, uMsg, HANDLE_WM_INITDIALOG((hWnd), (wParam), (lParam), (DialogOnInitDialog)));

		//case WM_COMMAND:
		//	return SetDlgMsgResult(hWnd, uMsg, HANDLE_WM_COMMAND((hWnd), (wParam), (lParam), (DialogOnCommand)));

		//case WM_TIMER:
		//	return SetDlgMsgResult(hWnd, uMsg, HANDLE_WM_TIMER((hWnd), (wParam), (lParam), (DialogOnTimer)));

	}

	return FALSE;

}

/*---------------------------------------------------------------------------*/