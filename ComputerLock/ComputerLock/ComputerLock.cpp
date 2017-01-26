// ComputerLock.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;

class ProcessHandler {
	public:
		ProcessHandler() {
			cout << "Process handler object built" << endl;
			DWORD bytesReturned;

			//Retrieves and 
			if (!EnumProcesses(allProcessIdentifiers, sizeof(allProcessIdentifiers), &bytesReturned)){
				cout << "Failed to retrieve the current open processes' id's" << endl;
			}
			else {
				numberOfProcessIdentifiers = bytesReturned / sizeof(DWORD);
			}
		}
		void DetectNewProcess();
		void MonitorProcesses() {
			// Starts by checking for the potential of all the currently opened processes
			for (int i = 0; i < numberOfProcessIdentifiers; i++) {
				checkProcessPotential(allProcessIdentifiers[i]);
			}
		}
		void InjectTrampoline();
		
	private:
		void checkProcessPotential(DWORD processID) {
			TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, processID);
			// Get the process name.

			if (NULL != hProcess)
			{
				HMODULE hMod;
				DWORD cbNeeded;

				if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
					&cbNeeded))
				{
					GetModuleBaseName(hProcess, hMod, szProcessName,
						sizeof(szProcessName) / sizeof(TCHAR));
				}

				// Print the process name and identifier.
				_tprintf(TEXT("%s  (PID: %u)\n"), szProcessName, processID);
			}
			else
				cout << GetLastError() << endl;

			// Release the handle to the process.

			CloseHandle(hProcess);

		}
		DWORD allProcessIdentifiers[1024], numberOfProcessIdentifiers;
};

int main() {
	ProcessHandler procHandler;
	procHandler.MonitorProcesses();
	return 0;
}
