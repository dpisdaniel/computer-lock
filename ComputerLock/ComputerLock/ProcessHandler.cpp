#include "stdafx.h"
#include "ProcessHandler.h"

ProcessHandler::ProcessHandler() {
	cout << "Process handler instance intialized" << endl;
}

void ProcessHandler::CheckOpenProcesses() {
	const string MTAG = ".MonitorProcesses";
	DWORD* numberOfProcessIdentifiers;
	DWORD* allProcessIdentifiers;
	allProcessIdentifiers = RetrieveOpenProcessesIDs(numberOfProcessIdentifiers); // Starts by retrieving the currently opened process IDs
	//Checks for the potential of all the currently opened processes
	for (unsigned long int i = 0; i < *numberOfProcessIdentifiers; i++) {
		HANDLE* currentProcess = NULL;
		if (CheckProcessPotential(allProcessIdentifiers[i], currentProcess)) {
			cout << currentProcess << " " << &currentProcess << endl;
			InjectHookDLL(currentProcess);
		}
	}
}

int ProcessHandler::InjectHookDLL(HANDLE hProcess)
{
	const string MTAG = ".InjectHookDLL";
	cout << "hey im here " << endl;
	HANDLE hThread;
	char  szLibPath[_MAX_PATH]; // size of the library path
	void*  pLibRemote = 0;	// the address (in the remote process) where szLibPath will be copied to
	HMODULE hKernel32 = GetModuleHandle(TEXT("Kernel32"));

	strcpy_s(szLibPath, "C:\\Users\\Daniel\\Documents\\Visual Studio 2015\\Projects\\ComputerLock\\ComputerLock\\Debug\\Trampoline.dll");
	cout << szLibPath << endl;

	// Allocates memory in the remote process for szLibPath and then
	// Writes szLibPath to the allocated memory
	pLibRemote = VirtualAllocEx(hProcess, NULL, sizeof(szLibPath), MEM_COMMIT, PAGE_READWRITE);
	if (pLibRemote == NULL) {
		cout << GetLastErrorAsString(GetLastError()) << endl;
		return false;
	}
	WriteProcessMemory(hProcess, pLibRemote, (void*)szLibPath, sizeof(szLibPath), NULL);


	// Load ""DLL_hook_test".dll" into the remote process 
	// via CreateRemoteThread & LoadLibrary
	cout << "Creating remote thread . . ." << endl;
	hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryA"), pLibRemote, 0, NULL);
	return true;
}

string ProcessHandler::GetLastErrorAsString(DWORD errorMessageID)
{
	const string MTAG = ".GetLastErrorAsString";
	//Get the error message, if any.
	if (errorMessageID == 0)
		return string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}

BOOL ProcessHandler::CheckProcessPotential(DWORD processID, HANDLE*& hProcess) {
	const string MTAG = ".CheckProcessPotential";
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, false, processID);
	// Get the process name.

	if (NULL != processHandle)
	{
		HMODULE hMod;
		DWORD cbNeeded;

		if (EnumProcessModules(processHandle, &hMod, sizeof(hMod), &cbNeeded))
		{
			GetModuleBaseName(processHandle, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
		}

		// Print the process name and identifier.
		_tprintf(TEXT("%s  (PID: %u)\n"), szProcessName, processID);
		if (_tcscmp(szProcessName, _T("firefox.exe")) == 0) {
			hProcess = (HANDLE*)processHandle;
			return true;
		}
	}
	else
		cout << "Couldn't get information on PID " << processID << ", error: " << GetLastErrorAsString(GetLastError()) << endl;

	// Releases the handle to the process.
	CloseHandle(processHandle);
	hProcess = nullptr;
	return false;
}

DWORD* ProcessHandler::RetrieveOpenProcessesIDs(DWORD*& numberOfProcessIdentifiers) {
	DWORD bytesReturned;
	DWORD allProcessIdentifiers[1024];
	if (!EnumProcesses(allProcessIdentifiers, sizeof(allProcessIdentifiers), &bytesReturned)) {
		cout << "Failed to retrieve the current open processes' id's" << endl;
	}
	else {
		*numberOfProcessIdentifiers = bytesReturned / sizeof(DWORD);
	}
	return allProcessIdentifiers;
}

void ProcessHandler::CheckSingleProcess(DWORD ProcessID) {
	HANDLE* hProcess = NULL;
	if (CheckProcessPotential(ProcessID, hProcess)) {
		cout << hProcess << " " << &hProcess << endl;
		InjectHookDLL(hProcess);
	}
	cout << "Process " << ProcessID << " is safe" << endl;
}