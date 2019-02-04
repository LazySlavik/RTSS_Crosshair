#include "stdafx.h"
#include "RTSS_Crosshair.h"
#include "RTSSSharedMemory.h"

#define Map1 "RTSS_Crosshair_Overlay"
#define Map2 "RTSS_Crosshair_Placeholder"

typedef struct {
	HANDLE hMapFile;
	LPRTSS_SHARED_MEMORY pMem;
} THREAD_PARAM;

THREAD_PARAM param;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI ThreadProc(LPVOID param);
BOOL UpdateOSD(LPCSTR lpText, LPCSTR mapName);
void ReleaseOSD(LPCSTR mapName);
CHAR crosshair_char[256] = "+";
bool changeState;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	LPCWSTR szWindowClass = TEXT("RTSSCROSSHAIR");
	LPCWSTR szTitle = TEXT("RTSS Crosshair");

	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RTSSCROSSHAIR));
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;

	if (!RegisterClassExW(&wcex))
		return FALSE;

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0,
		425, GetSystemMetrics(SM_CYMIN) + 150,
		nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
		return FALSE;
	
	CreateWindowW(
		TEXT("EDIT"),
		TEXT("RightCtrl + Num0 - Default settings \r\n"
			"RightCtrl + Num5 - Center \r\n"
			"RightCtrl + Num1, Num4, Num6, Num8 - move by 1 pixel \r\n"
			"Num1, Num4, Num6, Num8 + RightShift - move by 50 pixels \r\n"
			"RightCtrl + Num+ - increase size by 10% \r\n"
			"RightCtrl + Num- - decrease size by 10% \r\n"
			"RightCtrl + Num. - save current position in registry"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_READONLY,
		2, 2, 400, 122, hWnd, (HMENU)1, hInstance, NULL);

	CreateWindowW(
		TEXT("EDIT"), TEXT("+"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
		2, 126, 100, 20, hWnd, (HMENU)2, hInstance, NULL);

	CreateWindowW(
		TEXT("BUTTON"), TEXT("Apply"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_CENTER | BS_VCENTER,
		104, 126, 100, 20, hWnd, (HMENU)3, hInstance, NULL);

	UpdateOSD("+", Map1);
	UpdateOSD("", Map2);

	DWORD dwThreadId;
	CreateThread(NULL, 0, ThreadProc, NULL, 0, &dwThreadId);

	char tempChar[256];
	DWORD tempSize = sizeof(tempChar);
	if (ERROR_SUCCESS == RegGetValueA(HKEY_CURRENT_USER, "Software\\RTSS_Crosshair", "char", RRF_RT_REG_SZ, NULL, &tempChar, &tempSize)) {
		wsprintfA(crosshair_char, "%s", tempChar);
		SetDlgItemTextA(hWnd, 2, crosshair_char);
		changeState = true;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case 3:
			GetDlgItemTextA(hWnd, 2, crosshair_char, 256);
			changeState = true;
			//if (!GetDlgItemTextA(hWnd, 2, crosshair_char, 256))
			//	MessageBoxA(NULL, "Error occured", "Oh, no...", MB_OK);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;

	//case WM_PAINT:
	//{
	//    PAINTSTRUCT ps;
	//    HDC hdc = BeginPaint(hWnd, &ps);
	//    // TODO: Add any drawing code that uses hdc here...
	//    EndPaint(hWnd, &ps);
	//}
	//break;

	case WM_DESTROY:
		ReleaseOSD(Map1);
		ReleaseOSD(Map2);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

DWORD WINAPI ThreadProc(LPVOID param)
{
	int crossX = 0, crossY = 0, crossSize = 100;
	changeState = false;
	CHAR crossFormat[256];

	DWORD tempVar;
	DWORD tempSize = sizeof(tempVar);
	if (ERROR_SUCCESS == RegGetValueA(HKEY_CURRENT_USER, "Software\\RTSS_Crosshair", "x_coord", RRF_RT_REG_DWORD, NULL, &tempVar, &tempSize)) {
		crossX = tempVar;
		changeState = true;
		tempSize = sizeof(tempVar);
	}

	if (ERROR_SUCCESS == RegGetValueA(HKEY_CURRENT_USER, "Software\\RTSS_Crosshair", "y_coord", RRF_RT_REG_DWORD, NULL, &tempVar, &tempSize)) {
		crossY = tempVar;
		changeState = true;
		tempSize = sizeof(tempVar);
	}

	if (ERROR_SUCCESS == RegGetValueA(HKEY_CURRENT_USER, "Software\\RTSS_Crosshair", "size", RRF_RT_REG_DWORD, NULL, &tempVar, &tempSize)) {
		crossSize = tempVar;
		changeState = true;
	}

	while (TRUE) {

		if (GetAsyncKeyState(VK_RCONTROL) && GetAsyncKeyState(VK_DECIMAL)) {
			RegSetKeyValueA(HKEY_CURRENT_USER, "Software\\RTSS_Crosshair", "x_coord", REG_DWORD, &crossX, (DWORD)sizeof(crossX));
			RegSetKeyValueA(HKEY_CURRENT_USER, "Software\\RTSS_Crosshair", "y_coord", REG_DWORD, &crossY, (DWORD)sizeof(crossY));
			RegSetKeyValueA(HKEY_CURRENT_USER, "Software\\RTSS_Crosshair", "size", REG_DWORD, &crossSize, (DWORD)sizeof(crossSize));
			RegSetKeyValueA(HKEY_CURRENT_USER, "Software\\RTSS_Crosshair", "char", REG_SZ, &crosshair_char, (DWORD)sizeof(crosshair_char));
		}

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
			wsprintfA(crossFormat, "<P=%d,%d><S=%d>%s", crossX, crossY, crossSize, crosshair_char);
			UpdateOSD(crossFormat, Map1);
			UpdateOSD("<P=0,0><S=100>", Map2);
		}

		Sleep(100);
	}
}

BOOL UpdateOSD(LPCSTR lpText, LPCSTR mapName)
{
	BOOL bResult = FALSE;

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
				for (DWORD dwPass = 0; dwPass < 2; dwPass++)
					//1st pass : find previously captured OSD slot
					//2nd pass : otherwise find the first unused OSD slot and capture it
				{
					for (DWORD dwEntry = 1; dwEntry < pMem->dwOSDArrSize; dwEntry++)
						//allow primary OSD clients (i.e. EVGA Precision / MSI Afterburner) to use the first slot exclusively, so third party
						//applications start scanning the slots from the second one
					{
						RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pEntry = (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset + dwEntry * pMem->dwOSDEntrySize);

						if (dwPass)
						{
							if (!lstrlenA(pEntry->szOSDOwner))
								lstrcpyA(pEntry->szOSDOwner, mapName);
						}

						if (!lstrcmpA(pEntry->szOSDOwner, mapName))
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
										lstrcpynA(pEntry->szOSDEx, lpText, sizeof(pEntry->szOSDEx) - 1);

										pMem->dwBusy = 0;
									}
								}
								else
									lstrcpynA(pEntry->szOSDEx, lpText, sizeof(pEntry->szOSDEx) - 1);

							}
							else
								lstrcpynA(pEntry->szOSD, lpText, sizeof(pEntry->szOSD) - 1);

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
				for (DWORD dwEntry = 1; dwEntry < pMem->dwOSDArrSize; dwEntry++)
				{
					RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pEntry = (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset + dwEntry * pMem->dwOSDEntrySize);

					if (!lstrcmpA(pEntry->szOSDOwner, mapName))
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
