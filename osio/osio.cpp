#include <fstream>
#include <iomanip>
#include <sstream>

#include "framework.h"
using namespace std;
#include "resource.h"
#include <Windows.h>
#include <string>
#include <regex>

HWND hwndMain;
HINSTANCE hInstanceMain;
string tempPath;

BOOL CALLBACK mainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK renameWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK editWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK attrsWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

WIN32_FIND_DATA* findFile(string path, bool countDir = false);
bool fileExists(string path, bool showDialog = true);
void alert(string msg, string title = "Alert!", UINT uType = MB_OK | MB_ICONINFORMATION);
string getStringFromId(int id, HWND hwnd);
string getPath();
void clearPath();
vector<string> splitString(string s, string delimiter);
string sysTimeToString(SYSTEMTIME st);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR strCmdLine, int nCmdShow)
{
	hInstanceMain = hInstance;
	DialogBox(hInstanceMain,
		MAKEINTRESOURCE(IDD_MAIN),
		NULL,
		mainWindowProc);

	return 0;
}

// MAIN WINDOW HANDLER -----------------------------------------------------------

BOOL CALLBACK mainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
		{
			auto path = getPath();
			if (!fileExists(path)) {
				HANDLE hF = CreateFile(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hF == INVALID_HANDLE_VALUE) alert("Ошибка при создании файла", "Ошибка", MB_OK | MB_ICONERROR);
				else {
					alert("Файл успешно создан", "Информация", MB_OK | MB_ICONINFORMATION);
					CloseHandle(hF);
				}
			}
			break;
		}
		case IDD_B_DELETE:
		{
			auto path = getPath();
			if (fileExists(path, false))
			{
				auto h = DeleteFileA(path.c_str());
				if (h) alert("Файл успешно удален", "Информация", MB_OK | MB_ICONINFORMATION);
				else alert("Ошибка при удалении файла", "Ошибка", MB_OK | MB_ICONERROR);
			}
			else alert("Файл не существует", "Ошибка", MB_OK | MB_ICONERROR);
			break;
		}
		case IDD_B_COPY:
		{
			auto path = getPath();
			if (!fileExists(path, false))
			{
				alert("Файл не существует", "Ошибка", MB_OK | MB_ICONERROR);
				break;
			}
			auto newPath = path;
			newPath = regex_replace(newPath, regex("\\."), "(copy).");

			CopyFileA(path.c_str(), newPath.c_str(), true);

			alert("Файл успешно скопирован по пути " + newPath, "Информация");
				
			break;
		}
		case IDD_B_RENAME:
		{
			auto path = getPath();
			if (!fileExists(path, false))
			{
				alert("Файл не существует", "Ошибка", MB_OK | MB_ICONERROR);
				break;
			}

			tempPath = path;

			DialogBox(hInstanceMain, MAKEINTRESOURCE(IDD_RENAME), NULL, renameWindowProc);
				
			break;
		}
		case IDD_B_LINK:
		{
			auto path = getPath();
			if (!fileExists(path, false))
			{
				alert("Файл не существует", "Ошибка", MB_OK | MB_ICONERROR);
				break;
			}

			auto newName = regex_replace(splitString(path, "/").back(), regex("\\."), "(link)."); ;
			auto newPath = "c:/users/artgl/desktop/" + newName;

			auto r = CreateHardLink(newPath.c_str(), path.c_str(), NULL);

			if (r == 0) alert("Не удалось создать ссылку на файл", "Ошибка", MB_OK | MB_ICONERROR);
			else alert("Ссылка на файл успешна создана по пути " + newPath, "Информация");
			
			break;
		}
		case IDD_B_EDIT:
		{
			auto path = getPath();
			if (!fileExists(path, false))
			{
				alert("Файл не существует", "Ошибка", MB_OK | MB_ICONERROR);
				break;
			}

			tempPath = path;
			DialogBox(hInstanceMain, MAKEINTRESOURCE(IDD_EDIT), NULL, editWindowProc);
			break;
		}
		case IDD_B_INFO:
		{
			auto path = getPath();
			auto f = findFile(path);
			if(f == nullptr)
			{
				alert("Файл не существует", "Ошибка", MB_OK | MB_ICONERROR);
				break;
			}

			auto h = CreateFile(path.c_str(),
				GENERIC_READ,
				0,
				NULL,
				OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

			DWORD sz;
			GetFileSize(h, &sz);

			FILETIME dc, de;
			SYSTEMTIME dcUtc, deUtc;

			GetFileTime(h, &dc, NULL, &de);
			FileTimeToSystemTime(&dc, &dcUtc);
			FileTimeToSystemTime(&de, &deUtc);

			string info = "Путь: " + path + "\n";
			info += "Дата создания: " + sysTimeToString(dcUtc) + "\n";
			info += "Дата изменения: " + sysTimeToString(deUtc) + "\n";
			info += "Размер: " + to_string(sz) + " байт\n";
				
			alert(info, "Информация о файле");
				
			break;
		}
		case IDD_B_ATTRS:
		{
			auto path = getPath();
			if (!fileExists(path, false))
			{
				alert("Файл не существует", "Ошибка", MB_OK | MB_ICONERROR);
				break;
			}

			tempPath = path;
			DialogBox(hInstanceMain, MAKEINTRESOURCE(IDD_ATTRS), NULL, attrsWindowProc);
			break;
		}
		default:
			break;
		}
	}

	return FALSE;
}

// RENAME WINDOW HANDLER -----------------------------------------------------------

BOOL CALLBACK renameWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case RENAME_B_OK:
		{
			auto newPath = getStringFromId(RENAME_INPUT, hWnd);
			int r = MoveFile(tempPath.c_str(), newPath.c_str());
			if (r == 0) alert("Не удалось переместить файл", "Ошибка", MB_OK | MB_ICONERROR);
			else alert("Файл успешно перемещен/переименован", "Информация");
			EndDialog(hWnd, 0);
		}
		}
	}
	default:
		break;
	}

	return FALSE;
}

// EDIT WINDOW HANDLER -----------------------------------------------------------

BOOL CALLBACK editWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;
	case WM_INITDIALOG:
	{
		ifstream s(tempPath);
		stringstream buffer;
		buffer << s.rdbuf();

		string content = buffer.str();
		auto r = SetDlgItemText(hWnd, EDIT_INPUT, content.c_str());
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case EDIT_OK:
		{
			auto newData = getStringFromId(EDIT_INPUT, hWnd);

			auto h = CreateFile(tempPath.c_str(),
				GENERIC_WRITE,          
				0,                      
				NULL,                   
				OPEN_ALWAYS,             
				FILE_ATTRIBUTE_NORMAL,  
				NULL);
				
			DWORD bytesWritten = 0;
		
			auto r = WriteFile(
				 h,
				 newData.c_str(),
				 (DWORD)strlen(newData.c_str()),
				 &bytesWritten, 
				 NULL);
				
			 CloseHandle(h);

			 alert("Изменение файла успешно!", "Информация");
			 EndDialog(hWnd, 0);
		}
		}
	}
	default:
		break;
	}

	return FALSE;
}

// ATTRS WINDOW HANDLER -----------------------------------------------------------

void updateCheckboxes(string path, HWND hwnd) 
{
	DWORD attr = GetFileAttributes(path.c_str());

	if (attr & FILE_ATTRIBUTE_READONLY) SendDlgItemMessage(hwnd, ATTRS_RO, BM_SETCHECK, BST_CHECKED, 0);
	if (attr & FILE_ATTRIBUTE_HIDDEN) SendDlgItemMessage(hwnd, ATTRS_H, BM_SETCHECK, BST_CHECKED, 0);
	if (attr & FILE_ATTRIBUTE_ARCHIVE) SendDlgItemMessage(hwnd, ATTRS_A, BM_SETCHECK, BST_CHECKED, 0);
	if (attr & FILE_ATTRIBUTE_NORMAL) SendDlgItemMessage(hwnd, ATTRS_N, BM_SETCHECK, BST_CHECKED, 0);
	if (attr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) SendDlgItemMessage(hwnd, ATTRS_NCI, BM_SETCHECK, BST_CHECKED, 0);
	if (attr & FILE_ATTRIBUTE_OFFLINE) SendDlgItemMessage(hwnd, ATTRS_OFF, BM_SETCHECK, BST_CHECKED, 0);
	if (attr & FILE_ATTRIBUTE_SYSTEM) SendDlgItemMessage(hwnd, ATTRS_SYS, BM_SETCHECK, BST_CHECKED, 0);
	if (attr & FILE_ATTRIBUTE_TEMPORARY) SendDlgItemMessage(hwnd, ATTRS_TMP, BM_SETCHECK, BST_CHECKED, 0);
}

bool ifChecked(HWND hwnd, UINT id) {
	return SendDlgItemMessage(hwnd, id, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

BOOL CALLBACK attrsWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;
	case WM_INITDIALOG:
	{
		updateCheckboxes(tempPath, hWnd);
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ATTRS_SAVE:
		{
			DWORD attrs = GetFileAttributes(tempPath.c_str());

			attrs = ifChecked(hWnd, ATTRS_RO) ? attrs | FILE_ATTRIBUTE_READONLY : attrs & ~FILE_ATTRIBUTE_READONLY;
			attrs = ifChecked(hWnd, ATTRS_H) ? attrs | FILE_ATTRIBUTE_HIDDEN : attrs & ~FILE_ATTRIBUTE_HIDDEN;
			attrs = ifChecked(hWnd, ATTRS_A) ? attrs | FILE_ATTRIBUTE_ARCHIVE : attrs & ~FILE_ATTRIBUTE_ARCHIVE;
			attrs = ifChecked(hWnd, ATTRS_N) ? attrs | FILE_ATTRIBUTE_NORMAL : attrs & ~FILE_ATTRIBUTE_NORMAL;
			attrs = ifChecked(hWnd, ATTRS_NCI) ? attrs | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED : attrs & ~FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
			attrs = ifChecked(hWnd, ATTRS_OFF) ? attrs | FILE_ATTRIBUTE_OFFLINE : attrs & ~FILE_ATTRIBUTE_OFFLINE;
			attrs = ifChecked(hWnd, ATTRS_SYS) ? attrs | FILE_ATTRIBUTE_SYSTEM : attrs & ~FILE_ATTRIBUTE_SYSTEM;
			attrs = ifChecked(hWnd, ATTRS_TMP) ? attrs | FILE_ATTRIBUTE_TEMPORARY : attrs & ~FILE_ATTRIBUTE_TEMPORARY;

			SetFileAttributes(tempPath.c_str(), attrs);
			EndDialog(hWnd, 0);

			break;
		}
		}
	}
	default:
		break;
	}

	return FALSE;
}

// HELPER METHODS -------------------------------------------------------------

WIN32_FIND_DATA* findFile(string path, bool countDir) {
	if (path.length() == 0 || !isalpha(path.back()))
	{
		alert("Неправильный путь", "Ошибка", MB_OK | MB_ICONERROR);
		return nullptr;
	}
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

string getPath()
{
	return getStringFromId(IDC_PATH, hwndMain);
}

string getStringFromId(int id, HWND hwnd)
{
	char msg[256];
	GetDlgItemText(hwnd, id, (LPSTR)&msg, sizeof(msg) * sizeof(char));
	return string(msg);
}

void clearPath() {
	SetDlgItemText(hwndMain, IDC_PATH, "");
}

vector<string> splitString(string s, string delimiter) {
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	string token;
	vector<string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
		token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}

string sysTimeToString(SYSTEMTIME st)
{
	ostringstream ossMessage;

	ossMessage << st.wYear << "-"
		<< setw(2) << setfill('0') << st.wMonth << "-"
		<< setw(2) << setfill('0') << st.wDay << " "
		<< setw(2) << setfill('0') << st.wHour << ":"
		<< setw(2) << setfill('0') << st.wMinute << ":"
		<< setw(2) << setfill('0') << st.wSecond << "."
		<< setw(3) << setfill('0') << st.wMilliseconds;

	return  ossMessage.str();
}