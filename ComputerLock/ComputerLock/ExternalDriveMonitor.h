#pragma once

#define NO_MORE_ENTRIES 0

class LogicalDriveRetriever {
public:
	//Class constructor
	//It should store the current logical drives in the logicalDrives member
	LogicalDriveRetriever();

	void AddDriveMonitor(int driveLetter);

	BOOL MonitorDrive(LPCWSTR driveLetter);

private:
	vector<DWORD> logicalDrives;

	BOOL checkAction(DWORD action);

	void WaitForNewExternalDrives();

	void RetrieveCurrentLogicalDrives();

	/*Detects if the given logical drive is an external drive*/
	void DetectExternalDrives(int driveLetter);
};

class LogicalDrive {
public:
	LPCWSTR driveLetter;
	HANDLE hDir; 
	LogicalDrive(LPCWSTR driveLetter);


};
