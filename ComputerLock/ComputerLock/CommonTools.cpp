#include "stdafx.h"
#include "CommonTools.h"

namespace common {
	string GetLastErrorAsString(DWORD errorMessageID)
	{
		const string MTAG = ".GetLastErrorAsString";
		//Get the error message, if any.
		if (errorMessageID == 0)
			return string(); //No error message has been recorded

		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		string message(messageBuffer, size);

		//Free the buffer.
		LocalFree(messageBuffer);

		return message;
	}

	BOOL SetPrivilegeByToken(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege) {
		TOKEN_PRIVILEGES tp;
		LUID luid;

		if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
		{
			cout << "LookupPrivilegeValue error: " << GetLastErrorAsString(GetLastError()) << endl;
			return FALSE;
		}

		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		if (bEnablePrivilege)
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		else
			tp.Privileges[0].Attributes = 0;

		// Enable the privilege or disable all privileges.
		if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL)) {
			cout << "AdjustTokenPrivileges error: " << GetLastErrorAsString(GetLastError()) << endl;
			return FALSE;
		}

		if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
			cout << "The token does not have the specified privilege" << endl;
			return FALSE;
		}

		return TRUE;
	}

	BOOL SetPrivilegeByName(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege) {
		HANDLE hProcessToken = NULL;
		HANDLE hProcess = GetCurrentProcess();
		if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hProcessToken)) {
			cout << "Failed to retrieve this process's access token. error:" << GetLastErrorAsString(GetLastError()) << endl;
			return FALSE;
		}
		return SetPrivilegeByToken(hProcessToken, lpszPrivilege, bEnablePrivilege);
	}

	BOOL CompareFileMagic(HANDLE hFile, char* magicStr) {
		return FALSE;
	}

	void CharToByte(char* chars, byte* bytes, unsigned int count) {
		for (unsigned int i = 0; i < count; i++)
			bytes[i] = (byte)chars[i];
	}

	void ByteToChar(byte* bytes, char* chars, unsigned int count) {
		for (unsigned int i = 0; i < count; i++)
			chars[i] = (char)bytes[i];
	}

	template<typename Out>
	void split(const string &s, char delim, Out result) {
		stringstream ss;
		ss.str(s);
		string item;
		while (std::getline(ss, item, delim)) {
			*(result++) = item;
		}
	}

	vector<string> split(const string &s, char delim) {
		vector<string> elems;
		split(s, delim, back_inserter(elems));
		return elems;
	}

	void RewriteFile(wstring path, string stringToAppend) {
		ofstream outfile;
		outfile.open(path);
		outfile << stringToAppend;
	}

	vector<string> GetParamsFromFile(wstring path) {
		ifstream infile;
		infile.open(path);
		string paramsString("");
		infile >> paramsString;
		return split(paramsString, SETTINGS_DELIMITER);
	}

	wstring GetPathWithProgramFiles(wstring path, const KNOWNFOLDERID rfid) {
		PWSTR programFilesPath;
		HRESULT res = SHGetKnownFolderPath(rfid, KF_FLAG_DEFAULT, NULL, &programFilesPath);
		if (res != S_OK)
			throw "Program Files path could not be found";
		return wstring(programFilesPath) + path;
	}
}
