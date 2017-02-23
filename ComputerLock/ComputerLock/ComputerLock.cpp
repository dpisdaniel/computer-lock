// ComputerLock.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;
using namespace EventSink;

const string TAG = "ComputerLock";

class ProcessHandler {
	public:
		const string CTAG = ".ProcessHandler";

		ProcessHandler() {
			cout << "Process handler object built" << endl;
			DWORD bytesReturned;

			if (!EnumProcesses(allProcessIdentifiers, sizeof(allProcessIdentifiers), &bytesReturned)){
				cout << "Failed to retrieve the current open processes' id's" << endl;
			}
			else {
				numberOfProcessIdentifiers = bytesReturned / sizeof(DWORD);
			}
		}

		void DetectNewProcess();

		void MonitorProcesses() {
			const string MTAG = ".MonitorProcesses";
			// Starts by checking for the potential of all the currently opened processes
			for (unsigned long int i = 0; i < numberOfProcessIdentifiers; i++) {
				HANDLE* currentProcess = NULL;
				if (CheckProcessPotential(this->allProcessIdentifiers[i], currentProcess)) {
					cout << currentProcess << " " << &currentProcess << endl;
					InjectHookDLL(currentProcess);
				}
			}
		}
		
	private:
		DWORD allProcessIdentifiers[1024], numberOfProcessIdentifiers;

		/*
		checks if files can potentially be sent from the process related to the given processID. returns true if the process has that potential
		and false otherwise. outputs either potential process handle back to hProcess if the process has that potential or null if it doesn't. 
		*/
		BOOL CheckProcessPotential(DWORD processID, HANDLE*& hProcess) {
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

		string GetLastErrorAsString(DWORD errorMessageID)
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

		int InjectHookDLL(HANDLE hProcess)
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
			hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE) GetProcAddress(hKernel32, "LoadLibraryA"), pLibRemote, 0, NULL);
			return true;
		}
};

class ProcessCreationCallback {
	ProcessCreationCallback() {

	}
	private:
		HRESULT hr;
		IWbemLocator *pLoc = NULL;
		IWbemServices *pSvc = NULL;
		int InitializeCOMLibrary() {
			hr = CoInitializeEx(0, COINIT_MULTITHREADED);
			if (FAILED(hr))
			{
				cout << "Failed to initialize COM library. Error code = 0x"
					<< hex << hr << endl;
				CoUninitialize();
				return 1; //program failed
			}
			hr = CoInitializeSecurity(
				NULL,                        // Security descriptor    
				-1,                          // COM negotiates authentication service
				NULL,                        // Authentication services
				NULL,                        // Reserved
				RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication level for proxies
				RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation level for proxies
				NULL,                        // Authentication info
				EOAC_NONE,                   // Additional capabilities of the client or server
				NULL);
			return 0;
		}
		int ObtainWMILocator() {
			hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);
			if (FAILED(hr))
			{
				cout << "Failed to create IWbemLocator object. Err code = 0x"
					<< hex << hr << endl;
				CoUninitialize();
				return 1;
			}
			return 0;
		}
		int ConnectWMINamespace() {
			// Connect to the root\default namespace with the current user.
			hr = pLoc->ConnectServer(
				BSTR(L"ROOT\\CIMV2"),  //namespace
				NULL,       // User name 
				NULL,       // User password
				0,         // Locale 
				NULL,     // Security flags
				0,         // Authority 
				0,        // Context object 
				&pSvc);   // IWbemServices proxy


			if (FAILED(hr))
			{
				cout << "Could not connect. Error code = 0x"
					<< hex << hr << endl;
				pLoc->Release();
				CoUninitialize();
				return 1;      // Program has failed.
			}

			cout << "Connected to WMI" << endl;
			return 0;
		}
		int setWMIConnectionSecurity() {
			hr = CoSetProxyBlanket(pSvc,
				RPC_C_AUTHN_WINNT,
				RPC_C_AUTHZ_NONE,
				NULL,
				RPC_C_AUTHN_LEVEL_CALL,
				RPC_C_IMP_LEVEL_IMPERSONATE,
				NULL,
				EOAC_NONE
			);
			if (FAILED(hr))
			{
				cout << "Could not set proxy blanket. Error code = 0x"
					<< hex << hr << endl;
				pSvc->Release();
				pLoc->Release();
				CoUninitialize();
				return 1;      // Program has failed.
			}
			cout << "WMI proxy blanket set" << endl;
			return 0;
		}

};

int main() {
	ProcessHandler procHandler;
	procHandler.MonitorProcesses();
	return 0;
}
