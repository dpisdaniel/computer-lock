#pragma once

#define SETTINGS_FILES
#define ADMIN_SETTINGS common::GetPathWithProgramFiles(L"\\Daniel\\ComputerLock 1.0\\admin", FOLDERID_ProgramFilesX64)
#define PROCESS_SETTINGS common::GetPathWithProgramFiles(L"\\Daniel\\ComputerLock 1.0\\processes", FOLDERID_ProgramFilesX64)
#define FILE_EXT_SETTINGS common::GetPathWithProgramFiles(L"\\Daniel\\ComputerLock 1.0\\file_extensions", FOLDERID_ProgramFilesX64)
#define FILE_PATHS_SETTINGS common::GetPathWithProgramFiles(L"\\Daniel\\ComputerLock 1.0\\file_paths", FOLDERID_ProgramFilesX64)
#define TRAMPOLINE_DLL_PATH common::GetPathWithProgramFiles(L"\\Daniel\\ComputerLock 1.0\\Trampoline.dll", FOLDERID_ProgramFilesX64)

#define SYSTEM_PATH(windir) GetWindowsDirectory(windir, MAX_PATH)

#define SETTINGS_DELIMITER '\n'

namespace common {
	/*This is where methods that are used but don't really belong anywhere are put*/

	//Turns the GetLastError method's output into a printable std::string
	string GetLastErrorAsString(DWORD errorMessageID);

	/*Sets a given privilege for the access token handle given according to the name of the privilege.
	Enables or disables the privilege if bEnablePrivilege is True or False respectively*/
	BOOL SetPrivilegeByToken(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);

	/*Enables or disables the privilege given by the privilege name in lpszPrivilege. if bEnablePrivilege is True, enables the privilege,
	else disables it. This method will set the given privilege for the current process's access token.
	To set a privilege for a different access token use SetPrivilegeByToken*/
	BOOL SetPrivilegeByName(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);

	/*Compares the magic numbers of a file and a string representation of the magic number. Returns
	TRUE if the magic numbers are the same and FALSE otherwise*/
	BOOL CompareFileMagic(HANDLE hFile, char* magicStr);

	// Turns an array of chars to an array of bytes
	void CharToByte(char* chars, byte* bytes, unsigned int count);

	// Turns an array of bytes to an array of chars
	void ByteToChar(byte* bytes, char* chars, unsigned int count);

	// Splits a string into the result using a given delimiter
	template<typename Out>
	void split(const string &s, char delim, Out result);

	// Returns a split string vector using a given delimiter
	vector<string> split(const string &s, char delim);

	// Rewrites a file with new data (used for settings)
	void RewriteFile(wstring path, string stringToAppend);

	// Retrieves the parameters from a file
	vector<string> GetParamsFromFile(wstring path);

	// Prepends the program files x64 path to the given path
	wstring GetPathWithProgramFiles(wstring path, const KNOWNFOLDERID rfid);
}
