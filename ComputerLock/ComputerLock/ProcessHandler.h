#pragma once

class ProcessHandler {
/*
Provides methods for checking information on processes
*/
public:
	//Constructor for our class.
	ProcessHandler();

	//Iterates over the currently opened processes, checks which ones belong to the list of potentially dangerous processes and injects our hook DLL into the dangerous processes
	void CheckOpenProcesses();

	//Checks a single process' danger potential
	void CheckSingleProcess(DWORD ProcessID);

	//Enables or disables a privilege for a given access token
	BOOL ProcessHandler::SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);	

	//Turns the GetLastError method's output into a printable std::string
	string GetLastErrorAsString(DWORD errorMessageID);

private:

	/*
	checks if files can potentially be sent from the process related to the given processID. returns true if the process has that potential
	and false otherwise. outputs either potential process handle back to hProcess if the process has that potential or null if it doesn't.
	*/
	BOOL CheckProcessPotential(DWORD processID, HANDLE*& hProcess);

	//Injects our hook DLL into the given process handle
	int InjectHookDLL(HANDLE hProcess);

	//Retrieves all the process IDs of the currently opened processes and puts them in the allProcessIdentifiers array
	DWORD *RetrieveOpenProcessesIDs(DWORD* numberOfProcessIdentifiers);

	//Retrieves the current working directory of the executable (without the executable's file name)
	wstring ExePath();

};