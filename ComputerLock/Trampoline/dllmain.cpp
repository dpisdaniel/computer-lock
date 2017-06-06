// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "mhook.h"
#include "../ComputerLock/CommonTools.h"
//#include "../ComputerLock/Client.h"

#define globvar static

globvar wstring PROCESSES_SETTINGS = PROCESS_SETTINGS;
globvar wstring FILE_EXTENSION_SETTINGS = FILE_EXT_SETTINGS;
globvar wstring FILE_PATH_SETTINGS = FILE_PATHS_SETTINGS;
globvar const int fileDataBufferLength = 10240;

typedef HGDIOBJ(WINAPI * _CreateFile) (LPCTSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);


_CreateFile ActualCreateFile = (_CreateFile)GetProcAddress(GetModuleHandle(L"kernel32"), "CreateFileW");


char* encode(const wchar_t* wstr, unsigned int codePage)
{
	int sizeNeeded = WideCharToMultiByte(codePage, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* encodedStr = new char[sizeNeeded];
	WideCharToMultiByte(codePage, 0, wstr, -1, encodedStr, sizeNeeded, NULL, NULL);
	return encodedStr;
}

inline bool EndsWith(std::string const & value, std::string const & ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

BOOL IsImportant(const string& filePath, const wstring& settingsPath) {
	HANDLE hFile = ActualCreateFile(settingsPath.c_str(), GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	char fileDataBuffer[fileDataBufferLength];
	DWORD nRead;
	BOOL res = ReadFile(hFile, fileDataBuffer, fileDataBufferLength - 1, &nRead, NULL);
	fileDataBuffer[nRead] = '\0';
	string importantSettings(fileDataBuffer);
	vector<string> extensions = common::split(importantSettings, SETTINGS_DELIMITER);
	for (int i = 0; i < extensions.size(); i++) {
		extensions[i].erase(remove_if(extensions[i].begin(), extensions[i].end(), isspace), extensions[i].end());
		if (EndsWith(filePath, extensions[i])) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL IsImportantFile(const string& filePath) {
	// Check extensions first
	if (IsImportant(filePath, FILE_EXTENSION_SETTINGS))
		return TRUE;
	// Check file paths
	//if (IsImportant(filePath, FILE_PATH_SETTINGS))
		//return TRUE;
	return FALSE;
}

HGDIOBJ WINAPI HookCreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	char file_path[1024];
	strcpy_s(file_path, encode(lpFileName, CP_UTF8)); //encode to utf-8
	
	if (IsImportantFile(string(file_path)))
	{
		//MessageBoxA(NULL, (LPCSTR)file_path, NULL, NULL);
		//return INVALID_HANDLE_VALUE;
		//Gets the attached process name, in order to figure out which windows process handles file transfers

		HANDLE processHandle = GetCurrentProcess();
		TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
		HMODULE hMod;
		DWORD cbNeeded;
		// Get the process name.
		if (EnumProcessModules(processHandle, &hMod, sizeof(hMod), &cbNeeded))
		{
			GetModuleBaseName(processHandle, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
		}

		MessageBoxW(NULL, (LPCWSTR)szProcessName, NULL, NULL);
		return INVALID_HANDLE_VALUE;
	}
	return ActualCreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

void AttachHook() {
	if (Mhook_SetHook((PVOID*)&ActualCreateFile, HookCreateFile) == false)
	{
		MessageBoxA(NULL, (LPCSTR)"Error hook", NULL, NULL);
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		AttachHook();
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

