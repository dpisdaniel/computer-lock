#include "stdafx.h"
#include "ProcessHandler.h"

ProcessHandler::ProcessHandler() {
	cout << "Process handler instance intialized" << endl;
}

void ProcessHandler::CheckOpenProcesses() {
	const string MTAG = ".MonitorProcesses";
	DWORD numberOfProcessIdentifiers = 0;
	DWORD* allProcessIdentifiers;
	allProcessIdentifiers = RetrieveOpenProcessesIDs(&numberOfProcessIdentifiers); // Starts by retrieving the currently opened process IDs
	//Checks for the potential of all the currently opened processes
	for (unsigned long int i = 0; i < numberOfProcessIdentifiers; i++) {
		HANDLE* currentProcess = NULL;
		if (CheckProcessPotential(allProcessIdentifiers[i], currentProcess)) {
			cout << currentProcess << " " << &currentProcess << endl;
			InjectHookDLL(currentProcess);
			CloseHandle(currentProcess);
		}
	}
}

int ProcessHandler::InjectHookDLL(HANDLE hProcess)
{
	const string MTAG = ".InjectHookDLL";
	HANDLE hThread;
	char  szLibPath[_MAX_PATH]; // size of the library path
	void*  pLibRemote = 0;	// the address (in the remote process) where szLibPath will be copied to
	HMODULE hKernel32 = GetModuleHandle(TEXT("Kernel32"));

	strcpy_s(szLibPath, "C:\\Users\\Daniel\\Documents\\Visual Studio 2015\\Projects\\ComputerLock\\ComputerLock\\x64\\Debug\\Trampoline.dll");
	cout << szLibPath << endl;

	// Allocates memory in the remote process for szLibPath and then
	// Writes szLibPath to the allocated memory
	pLibRemote = VirtualAllocEx(hProcess, NULL, sizeof(szLibPath), MEM_COMMIT, PAGE_READWRITE);
	if (pLibRemote == NULL) {
		cout << GetLastErrorAsString(GetLastError()) << endl;
		return false;
	}
	WriteProcessMemory(hProcess, pLibRemote, (void*)szLibPath, sizeof(szLibPath), NULL);


	// Loads "Trampoline.dll" into the remote process 
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

	if (NULL != processHandle)
	{
		HMODULE hMod;
		DWORD cbNeeded;

		// Get the process name.
		if (EnumProcessModules(processHandle, &hMod, sizeof(hMod), &cbNeeded))
		{
			if (!GetModuleBaseName(processHandle, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR))) {
				cout << "Failed to retrieve the process name of process id " << processID << ". error: " << GetLastErrorAsString(GetLastError()) << endl;
				CloseHandle(processHandle);
				hProcess = nullptr;
				return false;
			}
		}
		else {
			cout << "Failed to retrieve the module list of process id " << processID << ". error: " << GetLastErrorAsString(GetLastError()) << endl;
		}

		// Print the process name and identifier.
		_tprintf(TEXT("%s  (PID: %u)\n"), szProcessName, processID);
		if (_tcscmp(szProcessName, _T("notepad.exe")) == 0) {
			hProcess = (HANDLE*)processHandle;
			return true;
		}
		//For debugging allows monitoring of all the processes
		hProcess = (HANDLE*)processHandle;
		return true;
	}
	else
		cout << "Couldn't get information on PID " << processID << ", error: " << GetLastErrorAsString(GetLastError()) << endl;

	// Releases the handle to the process.
	CloseHandle(processHandle);
	hProcess = nullptr;
	return false;
}

DWORD* ProcessHandler::RetrieveOpenProcessesIDs(DWORD* numberOfProcessIdentifiers) {
	DWORD bytesReturned;
	DWORD allProcessIdentifiers[1024];
	if (!EnumProcesses(allProcessIdentifiers, sizeof(allProcessIdentifiers), &bytesReturned)) {
		cout << "Failed to retrieve the current open processes' id's" << endl;
	}
	else {
		*numberOfProcessIdentifiers = bytesReturned / sizeof(DWORD);
	}
	DWORD* allProcessIdentifiersArrPointer = new DWORD[1024];
	for (DWORD i = 0; i < *numberOfProcessIdentifiers; i++) {
		allProcessIdentifiersArrPointer[i] = allProcessIdentifiers[i];
	}
	return allProcessIdentifiersArrPointer;
}

void ProcessHandler::CheckSingleProcess(DWORD ProcessID) {
	HANDLE* hProcess = NULL;
	if (CheckProcessPotential(ProcessID, hProcess)) {
		InjectHookDLL(hProcess);
		cout << "I have returned" << endl;
	}
	else
		cout << "Process " << ProcessID << " is safe" << endl;
}

wstring ProcessHandler::ExePath() {
	wchar_t buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	wstring::size_type pos = wstring(buffer).find_last_of(TEXT("\\/"));
	return wstring(buffer).substr(0, pos);
}

BOOL ProcessHandler::SetPrivilege(
	HANDLE hToken,          // access token handle
	LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
	BOOL bEnablePrivilege   // to enable or disable privilege
)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValue(
		NULL,            // lookup privilege on local system
		lpszPrivilege,   // privilege to lookup 
		&luid))        // receives LUID of privilege
	{
		cout << "LookupPrivilegeValue error: " << GetLastErrorAsString(GetLastError()) << endl;
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	// Enable the privilege or disable all privileges.

	if (!AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL))
	{
		cout << "AdjustTokenPrivileges error: " << GetLastErrorAsString(GetLastError()) << endl;
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

	{
		cout << "The token does not have the specified privilege" << endl;
		return FALSE;
	}

	return TRUE;
}