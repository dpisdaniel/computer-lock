#pragma once

#define SETTINGS_FILES
#define ADMIN_SETTINGS "C:\\Users\\Daniel\\Documents\\Visual Studio 2015\\Projects\\ComputerLock\\ComputerLock\\x64\\Debug\\admin"
#define PROCESS_SETTINGS "C:\\Users\\Daniel\\Documents\\Visual Studio 2015\\Projects\\ComputerLock\\ComputerLock\\x64\\Debug\\processes"
#define FILE_EXT_SETTINGS "C:\\Users\\Daniel\\Documents\\Visual Studio 2015\\Projects\\ComputerLock\\ComputerLock\\x64\\Debug\\file_extensions"
#define FILE_PATHS_SETTINGS "C:\\Users\\Daniel\\Documents\\Visual Studio 2015\\Projects\\ComputerLock\\ComputerLock\\x64\\Debug\\file_paths"

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

	//Turns an array of bytes to an array of chars
	void ByteToChar(byte* bytes, char* chars, unsigned int count);

	template<typename Out>
	void split(const string &s, char delim, Out result);

	vector<string> split(const string &s, char delim);

	void RewriteFile(string path, string stringToAppend);

	vector<string> GetParamsFromFile(string path);
}
