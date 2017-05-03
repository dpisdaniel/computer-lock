#include "stdafx.h"
#include "ExternalDriveMonitor.h"
#include "CommonTools.h"

LogicalDriveRetriever::LogicalDriveRetriever() {

	return;
}

BOOL LogicalDriveRetriever::MonitorDrive(LPCWSTR driveLetter) {
	HANDLE hDir = CreateFile(_T("C:/"), FILE_LIST_DIRECTORY, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (hDir == INVALID_HANDLE_VALUE) {
		cout << "Failed to open the drive for monitoring. error: " << GetLastErrorAsString(GetLastError()) << endl;
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
			//wcout << _T("Reading Directory Change") << endl;
		}
		else {
			PFILE_NOTIFY_INFORMATION pStrFileNotifyInfo = (PFILE_NOTIFY_INFORMATION)lpBuffer;
			if (pStrFileNotifyInfo->Action == FILE_ACTION_ADDED) {
				cout << "File added" << endl;
				wchar_t* pwszFileName = pStrFileNotifyInfo->FileName;
				pwszFileName[pStrFileNotifyInfo->FileNameLength / 2] = '\0';
				wcout << pwszFileName << endl;
			}
			wcout << _T("offset to next entry: ") << pStrFileNotifyInfo->NextEntryOffset << endl;
			wcout << _T("Action: ") << pStrFileNotifyInfo->Action << endl;
			while (pStrFileNotifyInfo->NextEntryOffset != NO_MORE_ENTRIES) {

				//gotta convert pStrFileNotifyInfo to bytes so when we add to it it'll add sizeof(byte) and not sizeof(PFILE_NOTIFY_INFORMATION)
				pStrFileNotifyInfo = (PFILE_NOTIFY_INFORMATION)((BYTE*)pStrFileNotifyInfo + pStrFileNotifyInfo->NextEntryOffset); 

				if (pStrFileNotifyInfo->Action == FILE_ACTION_ADDED) {
					cout << "File added" << endl;
				}
				wchar_t* pwszFileName = pStrFileNotifyInfo->FileName;
				pwszFileName[pStrFileNotifyInfo->FileNameLength / 2] = '\0'; // We divide by 2 because FileNameLength is in bytes (size 1) and FileName is in WCHAR (size 2) so the length given is two times longer.
				wcout << pwszFileName << endl;
				wcout << _T("offset to next entry: ") << pStrFileNotifyInfo->NextEntryOffset << endl;
				wcout << _T("Action: ") << pStrFileNotifyInfo->Action << endl;
			}
		}
	}
}