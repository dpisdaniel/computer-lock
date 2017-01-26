// ComputerLock.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <windows.h>

using namespace std;

class ProcessHandler {
	public:
		void DetectNewProcess();
		void MonitorProcesses();
		
	private:
		void checkProcessPotential(HANDLE hPotentialProcess);


};

int main() {
	return 0;
}
