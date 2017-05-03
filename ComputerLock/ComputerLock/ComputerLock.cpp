// ComputerLock.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "EventSink.h"
#include "MonitorProcessCreation.h"
#include "ProcessHandler.h"
#include "CommonTools.h"
#include "ExternalDriveMonitor.h"

#define SET_DEBUG_PRIVILEGE TRUE

const string TAG = "ComputerLock";

int main() {
	TCHAR szFileName[MAX_PATH + 1];

	GetModuleFileName(NULL, szFileName, MAX_PATH + 1);
	wcout << szFileName << endl;
	ProcessHandler procHandler;
	BOOL privSet = FALSE;

	LogicalDriveRetriever* driveRetriever = new LogicalDriveRetriever();
	driveRetriever->MonitorDrive(_T("C:\\"));

	if (SET_DEBUG_PRIVILEGE)
		privSet = SetPrivilegeByName(SE_DEBUG_NAME, TRUE);

	if(privSet){
		procHandler.CheckOpenProcesses();
		MonitorProcessCreation* processCallbackCreator = new MonitorProcessCreation();
		return 0;
	}

	cout << "could not set the privilege, terminating" << endl;
	return 1;
}
