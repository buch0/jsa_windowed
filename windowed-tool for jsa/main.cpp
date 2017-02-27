#include <stdio.h>
#include <Windows.h>
#include "Memory.h"


HWND GetWindowHandle(
	const DWORD TargetID)
{
	HWND hWnd = GetTopWindow(NULL);
	do {
		if (GetWindowLong(hWnd, GWL_HWNDPARENT) != 0 || !IsWindowVisible(hWnd))
			continue;
		DWORD ProcessID;
		GetWindowThreadProcessId(hWnd, &ProcessID);
		if (TargetID == ProcessID)
			return hWnd;
	} while ((hWnd = GetNextWindow(hWnd, GW_HWNDNEXT)) != NULL);

	return NULL;
}


DWORD PID;
void SetDisplayMode()
{
	HWND hWnd = GetWindowHandle(PID);

	SetWindowText(hWnd, "Sudden Attack(Hooked)");
	
	SetWindowLongPtr(hWnd, GWL_STYLE, WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);

	UINT flags = SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_SHOWWINDOW;
	SetWindowPos(hWnd, 0, 0, 0, 0, 0, flags);
}


void kill_drivers()
{
	LoadLibrary("sechost.dll");

	Memory *memory = new Memory();
	HANDLE hProcess;
	
	hProcess = OpenProcess(
		PROCESS_ALL_ACCESS,
		FALSE,
		memory->GetProcessByName("suddenattack"));

	WriteProcessMemory(hProcess, (LPVOID)((DWORD)GetProcAddress(GetModuleHandle("sechost.dll"), "StartServiceA")), "\xB8\x01\x00\x00\x00\xC2\x0C\x00", 8, NULL);
	WriteProcessMemory(hProcess, (LPVOID)((DWORD)GetProcAddress(GetModuleHandle("sechost.dll"), "StartServiceW")), "\xB8\x01\x00\x00\x00\xC2\x0C\x00", 8, NULL);
}

int main()
{
	Memory *memory = new Memory();

	while (true)
	{
		if (memory->GetProcessByName("suddenattack") != NULL)
			break;
		Sleep(100);
	}
	Sleep(2000);

	kill_drivers();

	HANDLE hProcess;

	hProcess = OpenProcess(
		PROCESS_ALL_ACCESS,
		FALSE,
		memory->GetProcessByName("suddenattack"));


	PID = memory->GetProcessByName("suddenattack");

	printf("[LOG] -> BASE: %X\n", memory->GetModuleBase32("CrashRptE.dll", PID));
	BYTE hookb[17] ="\xB8\x00\x2B\x02\x10\xC7\x00\x01\x00\x00\x00\x31\xC0\xC2\x08\x00";

	DWORD b_addr = (DWORD)VirtualAllocEx(hProcess, 0, 4, MEM_COMMIT, PAGE_READWRITE);
	
	memcpy(&hookb[1], &b_addr, 4);
	WriteProcessMemory(hProcess, (LPVOID)(GetProcAddress(GetModuleHandle("USER32.dll"),"ChangeDisplaySettingsA")), hookb, 17, NULL); //Hook


	DWORD b_value = 0;
	DWORD w_value = 0;

	for (;;)
	{
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);

		ReadProcessMemory(hProcess, (LPCVOID)b_addr, &b_value, 4, NULL);
		if (b_value == 1)
		{
			printf("[LOG] -> Hooking\n");
			SetDisplayMode();
			WriteProcessMemory(hProcess, (LPVOID)b_addr, &w_value, 4, NULL);
		}
		Sleep(100);

		if (memory->GetProcessByName("suddenattack") == NULL)
			exit(0);
	}

	return 0;
}
