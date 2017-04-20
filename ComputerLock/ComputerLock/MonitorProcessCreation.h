#pragma once
#include "EventSink.h"

class MonitorProcessCreation {
	// private interface pointers used to communicate with the wmi repositories.
	HRESULT hr;
	IWbemLocator *pLoc = NULL;
	IWbemServices *pSvc = NULL;
	// Use an unsecured apartment for security
	IUnsecuredApartment* pUnsecApp = NULL;
	IUnknown *pStubUnk = NULL;
	EventSink *pSink = new EventSink;
	IWbemObjectSink *pStubSink = NULL;
	BOOL cancelAsyncQuery = false;

public:
	// Constructor for the class
	MonitorProcessCreation();

private:
	// Initializes the COM library for this process and its security settings
	int InitializeCOMLibrary();

	// Retrieves a WbemLocator interface pointer 
	int ObtainWMILocator();

	// Connect to the root\cimv2 namespace with the current user.
	int ConnectWMINamespace();

	// Sets the WMI proxy blanket
	int setWMIConnectionSecurity();

	// Begins receiving notifications when __InstanceCreationEvent happens for Win32Process instances
	int ReceiveEventNotifications();

	/* Deletes all the currently used interface pointers and
	uninitializes the COM library for this process
	*/
	void CleanUp(BOOL cancelAsyncQuery);
};