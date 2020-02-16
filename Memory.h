#pragma once
#include <Windows.h>
#include <string>
#include <tlhelp32.h>
#include <iostream>

DWORD FindProcessId(const std::wstring& processName);
void FuelPatch();