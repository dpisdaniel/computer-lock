// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "mhook.h"


typedef HGDIOBJ(WINAPI * _CreateFile) (LPCTSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);


_CreateFile ActualCreateFile = (_CreateFile)GetProcAddress(GetModuleHandle(L"kernel32"), "CreateFileW");


char* encode(const wchar_t* wstr, unsigned int codePage)
{
	int sizeNeeded = WideCharToMultiByte(codePage, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* encodedStr = new char[sizeNeeded];
	WideCharToMultiByte(codePage, 0, wstr, -1, encodedStr, sizeNeeded, NULL, NULL);
	return encodedStr;
}

HGDIOBJ WINAPI HookCreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	char file_path[1024];
	strcpy_s(file_path, encode(lpFileName, CP_UTF8)); //encode to utf-8
	if (strstr(file_path, ".txt"))
	{
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
		MessageBoxA(NULL, (LPCSTR)"okayy", NULL, NULL);
		AttachHook();
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

