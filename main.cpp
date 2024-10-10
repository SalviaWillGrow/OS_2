#undef UNICODE
#include <windows.h>
#include <stdio.h>
#include "main.h"
#include "resource.h"

BOOL CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
HANDLE processHandle = NULL;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, DLGPROC(MainWndProc));
	return 0;
}

void DisplayTime(HWND hWindow) {
	FILETIME creationTime, exitTime, kernelTime, userTime;
	if (GetProcessTimes(
		processHandle,
		&creationTime,
		&exitTime,
		&kernelTime,
		&userTime
	) ) {
		char buf[100];
		DWORD size = 100;

		FILETIME localTime;
		FileTimeToLocalFileTime(&creationTime, &localTime);
		SYSTEMTIME localSystemTime;
		FileTimeToSystemTime(&localTime, &localSystemTime);
		sprintf_s(buf, "%u.%u.%u %02d:%02d:%02d",
			localSystemTime.wDay,
			localSystemTime.wMonth,
			localSystemTime.wYear,
			localSystemTime.wHour,
			localSystemTime.wMinute,
			localSystemTime.wSecond);
		SetDlgItemText(hWindow, IDC_STARTTIME, buf);

		FileTimeToLocalFileTime(&exitTime, &localTime);
		FileTimeToSystemTime(&localTime, &localSystemTime);
		sprintf_s(buf, "%u.%u.%u %02d:%02d:%02d",
			localSystemTime.wDay,
			localSystemTime.wMonth,
			localSystemTime.wYear,
			localSystemTime.wHour,
			localSystemTime.wMinute,
			localSystemTime.wSecond);
		SetDlgItemText(hWindow, IDC_FINISHTIME, buf);
	}

}


bool RunProcess(HWND hWindow, DWORD &processId) {

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;

	CHAR command[99];
	GetDlgItemTextA(
		hWindow,
		IDC_COMMANDLINE,
		command,
		99 );

	if (CreateProcessA(
		NULL,
		command,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi ) ) {

		CloseHandle(pi.hThread);
		if (processHandle != NULL) CloseHandle(processHandle);
		processHandle = pi.hProcess;
		processId = pi.dwProcessId;

		return TRUE;
	}
	return FALSE;
}

VOID Timer(HWND hWindow) {

	if (processHandle != NULL) {
		DWORD exitCode;
		GetExitCodeProcess(processHandle, &exitCode);
		if (exitCode != STILL_ACTIVE) {
			DisplayTime(hWindow);
			SetDlgItemText(hWindow, IDC_STATE, "Finished");
			EnableWindow(GetDlgItem(hWindow, IDC_START), TRUE);
			EnableWindow(GetDlgItem(hWindow, IDC_TERMINATE), FALSE);
			CloseHandle(processHandle);
			processHandle = NULL;
		}
	}
}

BOOL BrowseFileName(HWND hWindow, char* ptrFileName) {

	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = hWindow;
	ofn.lpstrFilter = "All Files\0*.*\0\0";
	ofn.lpstrFile = ptrFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = "exe";
	ofn.Flags = OFN_FILEMUSTEXIST;

	return GetOpenFileName(&ofn);
}

BOOL CALLBACK MainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg) {
	case WM_INITDIALOG:
		SetTimer(hWnd, NULL, 250, NULL);
		SetDlgItemText(hWnd, IDC_COMMANDLINE, "notepad.exe");
		EnableWindow(GetDlgItem(hWnd, IDC_TERMINATE), FALSE);
		return TRUE;

	case WM_TIMER:
		Timer(hWnd);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			
		case IDC_BROWSE:
		{
			char fileName[MAX_PATH] = "notepad.exe";
			if (BrowseFileName(hWnd, fileName)) {
				SetDlgItemText(hWnd, IDC_COMMANDLINE, fileName);
			}
			return TRUE;
		}

		case IDC_START:

			DWORD pId;
			RunProcess(hWnd, pId);
			DisplayTime(hWnd);

			SetDlgItemText(hWnd, IDC_FINISHTIME, "");
			SetDlgItemText(hWnd, IDC_STATE, "Running");
			SetDlgItemInt(hWnd, IDC_PID, pId, FALSE);

			EnableWindow(GetDlgItem(hWnd, IDC_START), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_TERMINATE), TRUE);

			return TRUE;

		case IDC_TERMINATE:
			if (processHandle != NULL)
				TerminateProcess(processHandle, NULL);
			return TRUE;

		case IDOK:
			if (processHandle != NULL) {
				CloseHandle(processHandle);
			}
			DestroyWindow(hWnd);
			return TRUE;

		}
		return FALSE;

	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}