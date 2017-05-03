// ComputerLock.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "EventSink.h"
#include "MonitorProcessCreation.h"
#include "ProcessHandler.h"

const string TAG = "ComputerLock";

BOOL SetPrivilege(ProcessHandler procHandler){
	HANDLE hProcessToken = NULL;
	HANDLE hProcess = GetCurrentProcess();
	if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hProcessToken)) {
		cout << "Failed to retrieve this process's access token. error:" << procHandler.GetLastErrorAsString(GetLastError()) << endl;
		return FALSE;
	}
	return procHandler.SetPrivilege(hProcessToken, SE_DEBUG_NAME, TRUE);
}

int main() {
	TCHAR szFileName[MAX_PATH + 1];

	GetModuleFileName(NULL, szFileName, MAX_PATH + 1);
	wcout << szFileName << endl;
	ProcessHandler procHandler;
	if(SetPrivilege(procHandler)){
		procHandler.CheckOpenProcesses();
		MonitorProcessCreation* processCallbackCreator = new MonitorProcessCreation();
		return 0;
	}
	cout << "could not set the privilege" << endl;
	return 1;
}
