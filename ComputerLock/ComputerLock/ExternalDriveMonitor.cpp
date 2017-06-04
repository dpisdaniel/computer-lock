#include "stdafx.h"
#include "ExternalDriveMonitor.h"
#include "CommonTools.h"

LogicalDriveRetriever::LogicalDriveRetriever() {
	// buffer for unique volume identifiers
	char buf[MAX_PATH];

	//The maximum length of a path in the volume
	DWORD lpMaximumComponentLength;

	// flags that describe the file system
	DWORD dwSysFlags;

	char FileSysNameBuf[MAX_PATH];
	BOOL test;
	// handle for the volume search (this is a search handle to use for subsequent calls and NOT a directory handle)
	HANDLE hVol;

	// Open a scan for volumes.
	hVol = FindFirstVolume((LPWSTR)buf, MAX_PATH);
	if (hVol == INVALID_HANDLE_VALUE)
	{
		::printf("No volumes found!\n");
		return ;
	}

	TCHAR VolumeName[MAX_PATH];
	// If all the requested information is retrieved, the return value is nonzero.
	// If not all the requested information is retrieved, the return value is 0 (zero).
	test = GetVolumeInformation(
		(LPCWSTR)buf,
		VolumeName,
		MAX_PATH,
		NULL,
		&lpMaximumComponentLength,
		&dwSysFlags,
		(LPWSTR)FileSysNameBuf,
		MAX_PATH
	);

	if (test) {
		::printf("The first volume found: %S\n", buf);
		::printf("The volume name is: %S \n", VolumeName);
		while (FindNextVolume(hVol, (LPWSTR)buf, MAX_PATH)) {
			test = GetVolumeInformation(
				(LPCWSTR)buf,
				VolumeName,
				MAX_PATH,
				NULL,
				&lpMaximumComponentLength,
				&dwSysFlags,
				(LPWSTR)FileSysNameBuf,
				MAX_PATH
			);

			if (test) {
				::printf("The volume found: %S\n", buf);
				::printf("The volume name is: %S\n", VolumeName);
			}
		}
	}
	
	
	::printf("The buffer for volume name: %d\n", MAX_PATH);
	::printf("The max component length: %d\n", lpMaximumComponentLength);
	::printf("The file system flag: %d\n", dwSysFlags);
	::printf("The file system: %S\n", FileSysNameBuf);
	::printf("The buffer for file system name: %d\n", MAX_PATH);
	if (FindVolumeClose(hVol) != 0)
		::printf("Handle for the %S closed successfully!\n", buf);
	else
		::printf("%S handle failed to close!\n");
}

BOOL LogicalDriveRetriever::MonitorDrive(LPCWSTR driveLetter) {
	HANDLE hDir = CreateFile(driveLetter, FILE_LIST_DIRECTORY, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (hDir == INVALID_HANDLE_VALUE) {
		cout << "Failed to open the drive for monitoring. error: " << common::GetLastErrorAsString(GetLastError()) << endl;
		return FALSE;
	}

	DWORD dwBuffer[1024];
	LPVOID lpBuffer = &dwBuffer;
	DWORD dwBytesReturned = 0;
	cout << "monitoring volume C:" << endl;
	while (true) {
		if (ReadDirectoryChangesW(hDir, lpBuffer, sizeof(dwBuffer), TRUE,
			FILE_NOTIFY_CHANGE_CREATION | 
			FILE_NOTIFY_CHANGE_FILE_NAME | 
			FILE_NOTIFY_CHANGE_DIR_NAME | 
			FILE_NOTIFY_CHANGE_SIZE | 
			FILE_NOTIFY_CHANGE_SECURITY |
			FILE_NOTIFY_CHANGE_ATTRIBUTES |
			FILE_NOTIFY_CHANGE_LAST_WRITE |
			FILE_NOTIFY_CHANGE_LAST_ACCESS, &dwBytesReturned, NULL, NULL) == 0) {
		}
		else {
			PFILE_NOTIFY_INFORMATION pStrFileNotifyInfo = (PFILE_NOTIFY_INFORMATION)lpBuffer;
			wchar_t* pwszFileName = pStrFileNotifyInfo->FileName;
			pwszFileName[pStrFileNotifyInfo->FileNameLength / 2] = '\0'; // We divide by 2 because FileNameLength is in bytes (size 1) and FileName is in WCHAR (size 2) so the length given is two times longer.
			
			wchar_t pswzFullFilePath[MAX_PATH];
			wcscpy_s(pswzFullFilePath, driveLetter);
			wcsncat_s(pswzFullFilePath, pwszFileName, MAX_PATH); //gets us the full file path (not just relative to the directory we are monitoring

			if (pStrFileNotifyInfo->Action == FILE_ACTION_ADDED) {
				//HANDLE hFile = CreateFile(pswzFullFilePath, GENERIC_ALL, FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, NULL, NULL);
				//if (hFile != INVALID_HANDLE_VALUE) {
				//if CompareFileMagic(hFile,"");

				//}
				wcout << _T("gonna delete dat file ") << endl;
				if (DeleteFile(pswzFullFilePath))
					wcout << _T("File ") << pswzFullFilePath << _T(" Successfully deleted") << endl;
				else {
					wcout << _T("Error deleting file ") << pswzFullFilePath << endl;
					cout << "Error: " << common::GetLastErrorAsString(GetLastError()) << endl;
				}
			}
			
			while (pStrFileNotifyInfo->NextEntryOffset != NO_MORE_ENTRIES) {
				//gotta convert pStrFileNotifyInfo to bytes so when we add to it it'll add sizeof(byte) and not sizeof(PFILE_NOTIFY_INFORMATION)
				pStrFileNotifyInfo = (PFILE_NOTIFY_INFORMATION)((BYTE*)pStrFileNotifyInfo + pStrFileNotifyInfo->NextEntryOffset); 
				wchar_t pswzFullFilePath[MAX_PATH];
				wcscpy_s(pswzFullFilePath, driveLetter);
				wcsncat_s(pswzFullFilePath, pwszFileName, MAX_PATH); //gets us the full file path (not just relative to the directory we are monitoring

				if (pStrFileNotifyInfo->Action == FILE_ACTION_ADDED) {
					//HANDLE hFile = CreateFile(pswzFullFilePath, GENERIC_ALL, FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, NULL, NULL);
					//if (hFile != INVALID_HANDLE_VALUE) {
						//if CompareFileMagic(hFile,"");

					//}
					wcout << _T("gonna delete dat file ") << endl;
					if (DeleteFile(pswzFullFilePath))
						wcout << _T("File ") << pswzFullFilePath << _T(" Successfully deleted") << endl;
					else {
						wcout << _T("Error deleting file ") << pswzFullFilePath << endl;
						cout << "Error: " << common::GetLastErrorAsString(GetLastError()) << endl;
					}
				}
				wchar_t* pwszFileName = pStrFileNotifyInfo->FileName;
				pwszFileName[pStrFileNotifyInfo->FileNameLength / 2] = '\0';
				
			}
		}
	}
}