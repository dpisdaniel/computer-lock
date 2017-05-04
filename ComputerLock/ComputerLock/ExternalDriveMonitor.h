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
	//vector<DWORD> logicalDrives;

	BOOL checkAction(DWORD action);

	void WaitForNewExternalDrives();

	void RetrieveCurrentLogicalDrives();

	/*Detects if the given logical drive is an external drive*/
	void DetectExternalDrives(int driveLetter);
};

class LogicalDrive {
/*
	This class represents a logical drive. It contains all the different "representations" for the drive 
	and provides an interface that makes monitoring the drive easier
*/
public:
	LPCWSTR driveLetter; // The volume root path null terminated string e.g C:/ 
	HANDLE hVol; // The handle to the volume
	LPWSTR lpszVolumeName; // The volume GUID path
	LPDWORD lpVolumeSerialNumber; // The serial number of the volume

	/*
	Receives the volume GUID path null terminated format. The constructor then retrieves and stores all the different 
	representations of the drive (like its root path, handle, etc)
	*/
	LogicalDrive(LPWSTR lpszVolumeName);
		
};
