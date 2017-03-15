// ComputerLock.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "EventSink.h"
#include "MonitorProcessCreation.h"
#include "ProcessHandler.h"

const string TAG = "ComputerLock";


int main() {
	//ProcessHandler procHandler;
	//procHandler.CheckOpenProcesses();
	MonitorProcessCreation* processCallbackCreator = new MonitorProcessCreation();
	return 0;
}
