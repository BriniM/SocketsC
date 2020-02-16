#include "Memory.h"

DWORD FindProcessId(const std::wstring& processName)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE) {
		return 0;
	}

	Process32First(processesSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile))
	{
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processesSnapshot, &processInfo))
	{
		if (!processName.compare(processInfo.szExeFile))
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(processesSnapshot);
	return 0;
}

void FuelPatch() {
	/* Basically LFS check whether the viewed player
	Is Local or Remote, we're removing that so 
	regardless of the player nature it sends Fuel data
	through outgauge!
	*/
	HANDLE LFS = OpenProcess(PROCESS_ALL_ACCESS, false, FindProcessId(L"LFS.exe"));
	if (LFS != NULL) {
		byte buffer[2];
		buffer[0] = 144;
		buffer[1] = 144;

		WriteProcessMemory(LFS, (LPVOID)0x4BC1E6, buffer, 2, NULL);

		CloseHandle(LFS);
	}
}