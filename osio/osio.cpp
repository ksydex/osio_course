#include "framework.h"
using namespace std;
#include "resource.h"
#include <windows.h>
#include <string>

HWND hwndMain;

BOOL CALLBACK mainWindowHandle(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

WIN32_FIND_DATA* findFile(string path, bool countDir = false);
bool fileExists(string path, bool showDialog = true);
void alert(string msg, string title = "Alert!", UINT uType = MB_OK | MB_ICONINFORMATION);
string getPath();
void clearPath();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR strCmdLine, int nCmdShow)
{
	DialogBox(hInstance,
		MAKEINTRESOURCE(IDD_MAIN),
		NULL,
		mainWindowHandle);

	return 0;
}

// MAIN WINDOW HANDLER -----------------------------------------------------------

BOOL CALLBACK mainWindowHandle(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	hwndMain = hWnd;

	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDD_B_CREATE:
			auto path = getPath();
			if (!fileExists(path)) {
				HANDLE hF = CreateFile(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if(hF == INVALID_HANDLE_VALUE) alert("Ошибка при создании файла", "Ошибка", MB_OK | MB_ICONERROR);
				else { 
					alert("Файл успешно создан", "Информация", MB_OK | MB_ICONINFORMATION); 
					CloseHandle(hF);
				}
			}
			break;
		}
		//case WM_INITDIALOG:
		//	return SetDlgMsgResult(hWnd, uMsg, HANDLE_WM_INITDIALOG((hWnd), (wParam), (lParam), (DialogOnInitDialog)));

		//case WM_COMMAND:
		//	return SetDlgMsgResult(hWnd, uMsg, HANDLE_WM_COMMAND((hWnd), (wParam), (lParam), (DialogOnCommand)));

		//case WM_TIMER:
		//	return SetDlgMsgResult(hWnd, uMsg, HANDLE_WM_TIMER((hWnd), (wParam), (lParam), (DialogOnTimer)));

	}

	return FALSE;
}

// HELPER METHODS -------------------------------------------------------------

WIN32_FIND_DATA* findFile(string path, bool countDir) {
	if (!isalpha(path.back())) alert("Неправильный путь", "Ошибка", MB_OK | MB_ICONERROR);

	WIN32_FIND_DATA info;
	HANDLE res = FindFirstFile(path.c_str(), &info);
	if (res == INVALID_HANDLE_VALUE) return nullptr;
	return !countDir && info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? nullptr : &info;
}

bool fileExists(string path, bool showDialog) {
	auto f = findFile(path, true);
	if (showDialog && f != NULL) alert("Файл или директория по такому пути уже существует", "Ошибка", MB_OK | MB_ICONERROR);
	return f != nullptr;
}

void alert(string msg, string title, UINT uType)
{
	MessageBoxA(NULL, msg.c_str(), title.c_str(), uType);
}

string getPath() {
	char msg[256];
	GetDlgItemText(hwndMain, IDC_PATH, (LPSTR)&msg, sizeof(msg) * sizeof(char));
	return string(msg);
}

void clearPath() {
	SetDlgItemText(hwndMain, IDC_PATH, "");
}