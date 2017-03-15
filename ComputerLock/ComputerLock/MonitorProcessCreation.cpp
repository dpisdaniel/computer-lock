#include "stdafx.h"
#include "MonitorProcessCreation.h"

MonitorProcessCreation::MonitorProcessCreation() {
	cout << "Ive been called" << endl;
	if (InitializeCOMLibrary() == TRUE) {
		cout << "hey" << endl;
		if (ObtainWMILocator() == TRUE) {
			cout << "hey2" << endl;
			if (ConnectWMINamespace() == TRUE) {
				cout << "hey3" << endl;
				if (setWMIConnectionSecurity() == TRUE) {
					cout << "We in" << endl;
					if (ReceiveEventNotifications() == TRUE) {
						cout << "All gucci" << endl;
					}
				}
			}
		}
	}
}

int MonitorProcessCreation::InitializeCOMLibrary() {
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		cout << "Failed to initialize COM library. Error code = 0x" << hex << hr << endl;
		CleanUp(cancelAsyncQuery);
		return FALSE;
	}
	hr = CoInitializeSecurity(
		NULL,                        // Security descriptor    
		-1,                          // COM negotiates authentication service
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication level for proxies
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation level for proxies
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities of the client or server
		NULL);
	return TRUE;
}

int MonitorProcessCreation::ObtainWMILocator() {
	hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);
	if (FAILED(hr))
	{
		cout << "Failed to create IWbemLocator object. Err code = 0x" << hex << hr << endl;
		CleanUp(cancelAsyncQuery);
		return FALSE;
	}
	return TRUE;
}

int MonitorProcessCreation::ConnectWMINamespace() {
	// Connect to the root\cimv2 namespace with the current user.
	hr = pLoc->ConnectServer(
		BSTR(L"ROOT\\CIMV2"),  //namespace
		NULL,       // User name 
		NULL,       // User password
		0,         // Locale 
		NULL,     // Security flags
		0,         // Authority 
		0,        // Context object 
		&pSvc);   // IWbemServices proxy


	if (FAILED(hr))
	{
		cout << "Could not connect. Error code = 0x" << hex << hr << endl;
		CleanUp(cancelAsyncQuery);
		return FALSE;
	}

	cout << "Connected to WMI" << endl;
	return TRUE;
}

int MonitorProcessCreation::setWMIConnectionSecurity() {
	hr = CoSetProxyBlanket(pSvc,
		RPC_C_AUTHN_WINNT,
		RPC_C_AUTHZ_NONE,
		NULL,
		RPC_C_AUTHN_LEVEL_CALL,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL,
		EOAC_NONE
	);
	if (FAILED(hr))
	{
		cout << "Could not set proxy blanket. Error code = 0x" << hex << hr << endl;
		CleanUp(cancelAsyncQuery);
		return FALSE;
	}
	cout << "WMI proxy blanket set" << endl;
	return TRUE;
}

int MonitorProcessCreation::ReceiveEventNotifications() {
	hr = CoCreateInstance(CLSID_UnsecuredApartment, NULL,
		CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment,
		(void**)&pUnsecApp);

	pSink->AddRef();

	pUnsecApp->CreateObjectStub(pSink, &pStubUnk);

	pStubUnk->QueryInterface(IID_IWbemObjectSink,
		(void **)&pStubSink);

	// The ExecNotificationQueryAsync method will call
	// The EventQuery::Indicate method when an event occurs
	hr = pSvc->ExecNotificationQueryAsync(_bstr_t("WQL"), _bstr_t("SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process'"), WBEM_FLAG_SEND_STATUS, NULL, pStubSink);

	if (FAILED(hr))
	{
		printf("ExecNotificationQueryAsync failed with = 0x%X\n", hr);
		CleanUp(cancelAsyncQuery);
		return FALSE;
	}
	string input;
	cout << "Enter anything to exit:" << endl;
	cin >> input;

	//hr = pSvc->CancelAsyncCall(pStubSink);
	cancelAsyncQuery = true;
	CleanUp(cancelAsyncQuery);
	return TRUE;
}

void MonitorProcessCreation::CleanUp(BOOL cancelAsyncQuery) {
	if (cancelAsyncQuery) {
		pSvc->CancelAsyncCall(pStubSink);
	}
	pSink->Release(); // EventSink implementation pointer

	if (pSvc != NULL) { // IWbemServices interface pointer
		pSvc->Release();
	}

	if (pLoc != NULL)  // IWbemLocator interface pointer
		pLoc->Release();

	if (pUnsecApp != NULL) // IUnsecuredApartment interface pointer
		pUnsecApp->Release();

	if (pStubUnk != NULL)  // Iunknown interface pointer (used for querying the EventSink interface)
		pStubUnk->Release();

	if (pStubSink != NULL) //  EventSink object pointer
		pStubSink->Release();

	CoUninitialize();  // Uninitializes COM 
}