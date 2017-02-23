#include "eventsink.h"

namespace EventSink {
	ULONG EventSink::AddRef()
	{
		return InterlockedIncrement(&m_lRef);
	}

	ULONG EventSink::Release()
	{
		LONG lRef = InterlockedDecrement(&m_lRef);
		if (lRef == 0)
			delete this;
		return lRef;
	}

	HRESULT EventSink::QueryInterface(REFIID riid, void** ppv)
	{
		if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
		{
			*ppv = (IWbemObjectSink *) this;
			AddRef();
			return WBEM_S_NO_ERROR;
		}
		else return E_NOINTERFACE;
	}


	HRESULT EventSink::Indicate(long lObjectCount,
		IWbemClassObject **apObjArray)
	{
		HRESULT hres = S_OK;

		for (int i = 0; i < lObjectCount; i++)
		{
			printf("Event occurred\n");
		}

		return WBEM_S_NO_ERROR;
	}

	HRESULT EventSink::SetStatus(
		/* [in] */ LONG lFlags,
		/* [in] */ HRESULT hResult,
		/* [in] */ BSTR strParam,
		/* [in] */ IWbemClassObject __RPC_FAR *pObjParam
	)
	{
		if (lFlags == WBEM_STATUS_COMPLETE)
		{
			printf("Call complete. hResult = 0x%X\n", hResult);
		}
		else if (lFlags == WBEM_STATUS_PROGRESS)
		{
			printf("Call in progress.\n");
		}

		return WBEM_S_NO_ERROR;
	}    // end of EventSink.cpp
}