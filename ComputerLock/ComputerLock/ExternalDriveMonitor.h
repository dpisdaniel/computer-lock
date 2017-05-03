#pragma once

namespace LogicalDriveMonitor {
	class LogicalDriveRetriever {
	public:
		//Class constructor
		//It should store the current logical drives in the logicalDrives member
		LogicalDriveRetriever();

		void AddDriveMonitor(int driveLetter);

	private:
		vector<DWORD> logicalDrives;

		void WaitForNewExternalDrives();

		void RetrieveCurrentLogicalDrives();

		/*Detects if the given logical drive is an external drive*/
		void DetectExternalDrives(int driveLetter);
	};
}