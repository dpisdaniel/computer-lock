// ComputerLock.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "EventSink.h"
#include "MonitorProcessCreation.h"
#include "ProcessHandler.h"
#include "CommonTools.h"
#include "ExternalDriveMonitor.h"
#include "Client.h"

#define SET_DEBUG_PRIVILEGE TRUE

int main() {
	UpdateLoop settingsUpdateClient(50, SERVER_IP, PORT);
	settingsUpdateClient.StartLoop();
	
	TCHAR szFileName[MAX_PATH + 1];

	GetModuleFileName(NULL, szFileName, MAX_PATH + 1);
	wcout << szFileName << endl;

	ProcessHandler procHandler;
	BOOL privSet = FALSE;

	//LogicalDriveRetriever* driveRetriever = new LogicalDriveRetriever();
	//driveRetriever->MonitorDrive(_T("C:\\"));

	if (SET_DEBUG_PRIVILEGE)
		privSet = common::SetPrivilegeByName(SE_DEBUG_NAME, TRUE);

	if(privSet){
		procHandler.CheckOpenProcesses();
		MonitorProcessCreation* processCallbackCreator = new MonitorProcessCreation();
		return 0;
	}

	cout << "could not set the privilege, terminating" << endl;
	return 1;
}
