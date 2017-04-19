#include "stdafx.h"
#include "eventsink.h"
#include "ProcessHandler.h"

ULONG EventSink::AddRef(){
	return InterlockedIncrement(&m_lRef);
}

ULONG EventSink::Release(){
	LONG lRef = InterlockedDecrement(&m_lRef);
	if (lRef == 0)
		delete this;
	return lRef;
}

HRESULT EventSink::QueryInterface(REFIID riid, void** ppv){
	if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
	{
		*ppv = (IWbemObjectSink *) this;
		AddRef();
		return WBEM_S_NO_ERROR;
	}
	else return E_NOINTERFACE;
}


HRESULT EventSink::Indicate(LONG lObjectCount, IWbemClassObject **apObjArray)
{

	for (int i = 0; i < lObjectCount; i++)
	{
		_variant_t vtProp;
		HRESULT hr;
		hr = apObjArray[i]->Get(_bstr_t(L"TargetInstance"), 0, &vtProp, 0, 0); // vtProp will contain an IUnknown interface to call the Win32_Process class IWbemClassObject interface
		if (SUCCEEDED(hr)) {
			IUnknown* str = vtProp;
			hr = str->QueryInterface(IID_IWbemClassObject, reinterpret_cast< void** >(&apObjArray[i]));
			if (SUCCEEDED(hr))
			{
				LONG ProcessId = NULL;
				_variant_t cn;
				hr = apObjArray[i]->Get(L"ProcessId", 0, &cn, NULL, NULL);
				if (SUCCEEDED(hr))
				{
					if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
						wcout << "ProcessId : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
					else {
						//wcout << "ProcessId : " << cn.lVal << endl;
						ProcessId = cn.lVal;
					}
				}
				VariantClear(&cn);
				if (ProcessId != NULL) {
					ProcessHandler procHandler;
					procHandler.CheckSingleProcess(ProcessId);
				}
			}
		}
		else {
			wprintf(L"Error in getting specified object\n");
		}
		VariantClear(&vtProp);
		cout << "And moving on . . ." << endl;
	}
	return WBEM_S_NO_ERROR;
}

HRESULT EventSink::SetStatus(LONG lFlags, HRESULT hResult, BSTR strParam, IWbemClassObject __RPC_FAR *pObjParam){
	if (lFlags == WBEM_STATUS_COMPLETE)
	{
		printf("Call complete. hResult = 0x%X\n", hResult);
	}
	else if (lFlags == WBEM_STATUS_PROGRESS)
	{
		printf("Call in progress.\n");
	}

	return WBEM_S_NO_ERROR;
}   