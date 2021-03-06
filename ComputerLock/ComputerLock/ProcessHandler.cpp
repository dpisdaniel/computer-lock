#include "stdafx.h"
#include "ProcessHandler.h"
#include "CommonTools.h"

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
	wchar_t  szLibPath[_MAX_PATH]; // size of the library path
	void*  pLibRemote = 0;	// the address (in the remote process) where szLibPath will be copied to
	HMODULE hKernel32 = GetModuleHandle(TEXT("Kernel32"));

	wcscpy_s(szLibPath, TRAMPOLINE_DLL_PATH.c_str());
	cout << szLibPath << endl;

	// Allocates memory in the remote process for szLibPath and then
	// Writes szLibPath to the allocated memory
	pLibRemote = VirtualAllocEx(hProcess, NULL, sizeof(szLibPath), MEM_COMMIT, PAGE_READWRITE);
	if (pLibRemote == NULL) {
		cout << common::GetLastErrorAsString(GetLastError()) << endl;
		return false;
	}
	WriteProcessMemory(hProcess, pLibRemote, (void*)szLibPath, sizeof(szLibPath), NULL);


	// Loads "Trampoline.dll" into the remote process 
	// via CreateRemoteThread & LoadLibrary
	cout << "Creating remote thread . . ." << endl;
	hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryW"), pLibRemote, 0, NULL);
	if (hThread == NULL) {
		cout << common::GetLastErrorAsString(GetLastError()) << endl;
	}
	return true;
}

BOOL ProcessHandler::CheckProcessPotential(DWORD processID, HANDLE*& hProcess) {
	const string MTAG = ".CheckProcessPotential";
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, false, processID);

	if (NULL != processHandle) {
		HMODULE hMod;
		DWORD cbNeeded;

		// Get the process name.
		if (EnumProcessModules(processHandle, &hMod, sizeof(hMod), &cbNeeded)) {
			if (!GetModuleBaseName(processHandle, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR))) {
				cout << "Failed to retrieve the process name of process id " << processID << ". error: " << common::GetLastErrorAsString(GetLastError()) << endl;
				CloseHandle(processHandle);
				hProcess = nullptr;
				return false;
			}
		}
		else {
			cout << "Failed to retrieve the module list of process id " << processID << ". error: " << common::GetLastErrorAsString(GetLastError()) << endl;
		}

		// Print the process name and identifier.
		wcout << szProcessName << " (PID: " << processID << ")" << endl;
		
		BOOL isSafe = FALSE;
		if (IsSystemProcess(processHandle))
			isSafe = TRUE;
		if (IsProcessSafe(szProcessName))
			isSafe = TRUE;
		if (isSafe == FALSE) {
			wcout << "process is not safe: " << szProcessName << endl;
			hProcess = (HANDLE*)processHandle;
			return TRUE;
		}
		//For debugging allows monitoring of all the processes
		//hProcess = (HANDLE*)processHandle;
		//return true;
	}
	else
		cout << "Couldn't get information on PID " << processID << ", error: " << common::GetLastErrorAsString(GetLastError()) << endl;

	// Releases the handle to the process.
	wcout << "process is safe: " << szProcessName << endl;
	CloseHandle(processHandle);
	hProcess = nullptr;
	return FALSE;
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
		std::this_thread::sleep_for(std::chrono::seconds(1));
		InjectHookDLL(hProcess);
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

BOOL ProcessHandler::IsSystemProcess(HANDLE processHandle) {
	TCHAR filename[MAX_PATH];

	if (GetModuleFileNameEx(processHandle, NULL, filename, MAX_PATH) == 0) {
		cerr << "Failed to get module filename." << endl;
	}
	TCHAR windir[MAX_PATH];
	SYSTEM_PATH(windir);
	if(wstring(filename).find(wstring(windir)) != std::string::npos)
		return TRUE;
	return FALSE;
}

BOOL ProcessHandler::IsProcessSafe(TCHAR* szProcessName) {
	vector<string> safeProcesses = common::GetParamsFromFile(PROCESS_SETTINGS);
	BOOL isSafe = FALSE;
	for (int i = 0; i < safeProcesses.size() && !isSafe; i++) {
		wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		wstring procName = converter.from_bytes(safeProcesses[i]); // gotta convert the string types to wstring for comparison
		if (_tcscmp(procName.c_str(), szProcessName) == 0) //_tcscmp returns 0 if the comparison was true
			isSafe = TRUE;
	}
	return isSafe;
}