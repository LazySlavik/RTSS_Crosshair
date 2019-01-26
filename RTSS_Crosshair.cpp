#include <windows.h>
#include "RTSSSharedMemory.h"
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/NODEFAULTLIB")
#pragma comment(linker, "/INCREMENTAL:NO")

#define Map1 "RTSS_Crosshair_Overlay"
#define Map2 "RTSS_Crosshair_Placeholder"

typedef struct {
	HANDLE hMapFile;
	LPRTSS_SHARED_MEMORY pMem;
} THREAD_PARAM;

THREAD_PARAM param;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI ThreadProc(LPVOID param);
BOOL UpdateOSD(LPCSTR lpText, LPCSTR mapName);
void ReleaseOSD(LPCSTR mapName);

void WinMainCRTStartup()
{
	LPCTSTR pClassName = TEXT("RTSS_Crosshair");
	HINSTANCE hinst;
	MSG msg;
	HWND hwnd;

	WNDCLASSEX wc;

	hinst = GetModuleHandle(NULL);

	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WindowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hinst;
	wc.hIcon         = NULL;
	wc.hIconSm       = NULL;
	wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = pClassName;

	if (!RegisterClassEx(&wc)) return;

	hwnd = CreateWindow(
		pClassName, 
		TEXT("RTSS Crosshair"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 
		CW_USEDEFAULT,
		425, 
		GetSystemMetrics(SM_CYMIN) + 126,
		NULL, 
		NULL, 
		hinst, 
		NULL );
	if (!hwnd) return;

	ShowWindow(hwnd, SW_SHOW);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return;
}

DWORD WINAPI ThreadProc(LPVOID param)
{
	int crossX = 0, crossY = 0, crossSize = 100;
	bool changeState = false;
	TCHAR crossFormat[256];
	//HKEY hKey;
	//RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Unwinder\\RTSS", &hKey)
	//RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Unwinder\\RTSS", 0, KEY_READ, &hKey) KEY_READ KEY_WRITE
	
	while (TRUE) {

		if (GetAsyncKeyState(VK_RCONTROL) && GetAsyncKeyState(VK_NUMPAD5)) {
			RECT rect;
			if (GetClientRect(GetForegroundWindow(), &rect)) //GetClientRect GetWindowRect
			{
				crossX = (rect.right - rect.left) / 2;
				crossY = (rect.bottom - rect.top) / 2;
				changeState = true;
			}
		}
		
		if (GetAsyncKeyState(VK_RCONTROL) && GetAsyncKeyState(VK_NUMPAD0)) {
			crossX = 0;
			crossY = 0;
			crossSize = 100;
			changeState = true;
		}
		
		if (GetAsyncKeyState(VK_RSHIFT) && GetAsyncKeyState(VK_NUMPAD4)) {
			crossX -= 50;
			changeState = true;
		}
		
		if (GetAsyncKeyState(VK_RSHIFT) && GetAsyncKeyState(VK_NUMPAD6)) {
			crossX += 50;
			changeState = true;
		}
		
		if (GetAsyncKeyState(VK_RSHIFT) && GetAsyncKeyState(VK_NUMPAD8)) {
			crossY -= 50;
			changeState = true;
		}
		
		if (GetAsyncKeyState(VK_RSHIFT) && GetAsyncKeyState(VK_NUMPAD2)) {
			crossY += 50;
			changeState = true;
		}
		
		if (GetAsyncKeyState(VK_RCONTROL) && GetAsyncKeyState(VK_NUMPAD4)) {
			crossX--;
			changeState = true;
		}
		
		if (GetAsyncKeyState(VK_RCONTROL) && GetAsyncKeyState(VK_NUMPAD6)) {
			crossX++;
			changeState = true;
		}
		
		if (GetAsyncKeyState(VK_RCONTROL) && GetAsyncKeyState(VK_NUMPAD8)) {
			crossY--;
			changeState = true;
		}
		
		if (GetAsyncKeyState(VK_RCONTROL) && GetAsyncKeyState(VK_NUMPAD2)) {
			crossY++;
			changeState = true;
		}
		
		if (GetAsyncKeyState(VK_RCONTROL) && GetAsyncKeyState(VK_ADD)) {
			crossSize += 5;
			changeState = true;
		}
	
		if (GetAsyncKeyState(VK_RCONTROL) && GetAsyncKeyState(VK_SUBTRACT)) {
			crossSize -= 5;
			changeState = true;
		}
		
		if (changeState) {
			changeState = false;
			wsprintf(crossFormat, TEXT("<P=%d,%d><S=%d>+"), crossX, crossY, crossSize);
			UpdateOSD(crossFormat, Map1);
			UpdateOSD("<P=0,0><S=100>", Map2);
		}
		
		Sleep(100);
	}
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_DESTROY:
		ReleaseOSD(Map1);
		ReleaseOSD(Map2);
		ExitProcess(0);
		break;

	case WM_CREATE:
		CreateWindow(
			TEXT("EDIT"), 
			"RightCtrl + Num0 - Default settings \r\n"
			"RightCtrl + Num5 - Center \r\n"
			"RightCtrl + Num1, Num4, Num6, Num8 - move by 1 pixel \r\n"
			"Num1, Num4, Num6, Num8 + RightShift - move by 50 pixels \r\n"
			"RightCtrl + Num+ - increase size by 10% \r\n"
			"RightCtrl + Num- - decrease size by 10% ",
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_READONLY, // 
			2, 2, 400, 122,
			hwnd, (HMENU)1,
			((LPCREATESTRUCT)(lp))->hInstance, NULL
		);

		UpdateOSD("+", Map1);
		UpdateOSD("", Map2);

		DWORD dwThreadId;
		CreateThread(NULL, 0, ThreadProc, NULL, 0, &dwThreadId);
		break;

	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
BOOL UpdateOSD(LPCSTR lpText, LPCSTR mapName)
{
	BOOL bResult	= FALSE;

	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, TEXT("RTSSSharedMemoryV2"));

	if (hMapFile)
	{
		LPVOID pMapAddr				= MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		LPRTSS_SHARED_MEMORY pMem	= (LPRTSS_SHARED_MEMORY)pMapAddr;

		if (pMem)
		{
			if ((pMem->dwSignature == 'RTSS') && 
				(pMem->dwVersion >= 0x00020000))
			{
				for (DWORD dwPass=0; dwPass<2; dwPass++)
					//1st pass : find previously captured OSD slot
					//2nd pass : otherwise find the first unused OSD slot and capture it
				{
					for (DWORD dwEntry=1; dwEntry<pMem->dwOSDArrSize; dwEntry++)
						//allow primary OSD clients (i.e. EVGA Precision / MSI Afterburner) to use the first slot exclusively, so third party
						//applications start scanning the slots from the second one
					{
						RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pEntry = (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset + dwEntry * pMem->dwOSDEntrySize);

						if (dwPass)
						{
							if (!lstrlen(pEntry->szOSDOwner))
								lstrcpy(pEntry->szOSDOwner, mapName);
						}

						if (!lstrcmp(pEntry->szOSDOwner, mapName))
						{
							if (pMem->dwVersion >= 0x00020007)
								//use extended text slot for v2.7 and higher shared memory, it allows displaying 4096 symbols
								//instead of 256 for regular text slot
							{
								if (pMem->dwVersion >= 0x0002000e)
									//OSD locking is supported on v2.14 and higher shared memory
								{
									DWORD dwBusy = _interlockedbittestandset(&pMem->dwBusy, 0);
									//bit 0 of this variable will be set if OSD is locked by renderer and cannot be refreshed
									//at the moment

									if (!dwBusy)
									{
										lstrcpyn(pEntry->szOSDEx, lpText, sizeof(pEntry->szOSDEx) - 1);

										pMem->dwBusy = 0;
									}
								}
								else
									lstrcpyn(pEntry->szOSDEx, lpText, sizeof(pEntry->szOSDEx) - 1);

							}
							else
								lstrcpyn(pEntry->szOSD, lpText, sizeof(pEntry->szOSD) - 1);

							pMem->dwOSDFrame++;

							bResult = TRUE;

							break;
						}
					}

					if (bResult)
						break;
				}
			}

			UnmapViewOfFile(pMapAddr);
		}

		CloseHandle(hMapFile);
	}

	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
void ReleaseOSD(LPCSTR mapName)
{
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, TEXT("RTSSSharedMemoryV2"));

	if (hMapFile)
	{
		LPVOID pMapAddr = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		LPRTSS_SHARED_MEMORY pMem = (LPRTSS_SHARED_MEMORY)pMapAddr;

		if (pMem)
		{
			if ((pMem->dwSignature == 'RTSS') && 
				(pMem->dwVersion >= 0x00020000))
			{
				for (DWORD dwEntry=1; dwEntry<pMem->dwOSDArrSize; dwEntry++)
				{
					RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pEntry = (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset + dwEntry * pMem->dwOSDEntrySize);

					if (!lstrcmp(pEntry->szOSDOwner, mapName))
					{
						SecureZeroMemory(pEntry, pMem->dwOSDEntrySize);
						pMem->dwOSDFrame++;
					}
				}
			}

			UnmapViewOfFile(pMapAddr);
		}

		CloseHandle(hMapFile);
	}
}
