
#include "HTMLexPlugin.h"

#include <exdispid.h>
#include <shlwapi.h>
#include <tchar.h>

#include <new>  // std::bad_alloc


static long globalRefCount = 0;
static HINSTANCE instance;

static const CLSID CLSID_HTMLexPlugin =
	{0xa427e58d, 0x5f0d, 0x40ec, 0xaf, 0x41, 0x36, 0xe1, 0xc6, 0x9e, 0xb9, 0x42};
#define CLSID_HTMLexPlugin_str  _T("{A427E58D-5F0D-40ec-AF41-36E1C69EB942}")

static const TCHAR regPathCLSID[] =
	_T("CLSID\\") CLSID_HTMLexPlugin_str;
static const TCHAR regPathInprocServer32[] =
	_T("CLSID\\") CLSID_HTMLexPlugin_str _T("\\InProcServer32");
static const TCHAR regPathBHO[] =
	_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\") \
	CLSID_HTMLexPlugin_str;


BOOL APIENTRY DllMain(HMODULE hInstance, DWORD dwReason, LPVOID)
{
	instance = hInstance;
	if (dwReason == DLL_PROCESS_ATTACH)
		DisableThreadLibraryCalls(hInstance);
    return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvOut)
{
	static ClassFactory factory;
	if (rclsid == CLSID_HTMLexPlugin)
		return factory.QueryInterface(riid, ppvOut);
	else
	{
		*ppvOut = NULL;
		return CLASS_E_CLASSNOTAVAILABLE;
	}
}

STDAPI DllCanUnloadNow()
{
	return (globalRefCount > 0) ? S_FALSE : S_OK;
}

STDAPI DllRegisterServer()
{
	TCHAR fileName[MAX_PATH];
	DWORD rc = GetModuleFileName(instance, fileName, MAX_PATH);
	if ((rc == 0) || (rc == MAX_PATH))
		return E_UNEXPECTED;

	if (SHSetValue(HKEY_CLASSES_ROOT, regPathCLSID, _T(""), REG_SZ, _T("BHO"),
		           4 * sizeof(TCHAR)) != ERROR_SUCCESS)
		goto error;
	if (SHSetValue(HKEY_CLASSES_ROOT, regPathInprocServer32, _T(""), REG_SZ, fileName,
				   (static_cast<DWORD>(_tcslen(fileName)) + 1) * sizeof(TCHAR)) != ERROR_SUCCESS)
		goto error;
	if (SHSetValue(HKEY_CLASSES_ROOT, regPathInprocServer32, _T("ThreadingModel"), REG_SZ,
		           _T("Apartment"), 10 * sizeof(TCHAR)) != ERROR_SUCCESS)
		goto error;
	if (SHSetValue(HKEY_LOCAL_MACHINE, regPathBHO, _T(""), REG_SZ, _T(""),
				   sizeof(TCHAR)) != ERROR_SUCCESS)
		goto error;
	if (SHSetValue(HKEY_LOCAL_MACHINE, regPathBHO, _T("NoExplorer"), REG_SZ, _T("1"),
				   sizeof(TCHAR)) != ERROR_SUCCESS)
		goto error;

	return S_OK;

error:
	SHDeleteKey(HKEY_CLASSES_ROOT, regPathCLSID);
	return SELFREG_E_CLASS;
}

STDAPI DllUnregisterServer()
{
	SHDeleteKey(HKEY_LOCAL_MACHINE, regPathBHO);
	if (SHDeleteKey(HKEY_CLASSES_ROOT, regPathCLSID) != ERROR_SUCCESS)
		return SELFREG_E_CLASS;
	return S_OK;
}


const DWORD HTMLexPlugin::timeOutMs = 1000;


HTMLexPlugin::HTMLexPlugin() : mRefCount(1), mWebBrowser(NULL),
		mConnectionPointContainer(NULL), mThreadState(1)
{
	mMutex = CreateMutex(NULL, TRUE, NULL);  // create and acquire
	mCommThread = CreateThread(NULL, 0, &commThreadProc, this, 0, NULL);
	if (!mMutex || !mCommThread)
		OutputDebugString(_T("HTMLexPlugin: Initialization failed."));
}

HTMLexPlugin::~HTMLexPlugin()
{
	clear();
	mThreadState = 0;
	WaitForSingleObject(mCommThread, INFINITE);
	CloseHandle(mMutex);
}

void HTMLexPlugin::clear()
{
	if (mWebBrowser)
	{
		mWebBrowser->Release();
		mWebBrowser = NULL;
	}
	if (mConnectionPointContainer)
	{
		mConnectionPointContainer->Release();
		mConnectionPointContainer = NULL;
	}
}


ULONG STDMETHODCALLTYPE HTMLexPlugin::AddRef()
{
	InterlockedIncrement(&globalRefCount);
	return InterlockedIncrement(&mRefCount);
}

ULONG STDMETHODCALLTYPE HTMLexPlugin::Release()
{
	long c = InterlockedDecrement(&mRefCount);
	if (c == 0)
		delete this;
	InterlockedDecrement(&globalRefCount);
	return c;
}

HRESULT STDMETHODCALLTYPE HTMLexPlugin::QueryInterface(REFIID riid, LPVOID *ppvObject)
{
	if ((riid == IID_IUnknown) ||
		(riid == IID_IObjectWithSite) ||
		(riid == IID_IDispatch))
	{
		AddRef();
		if (riid == IID_IDispatch)
			*ppvObject = static_cast<IDispatch *>(this);
		else
			*ppvObject = this;
		return S_OK;
	}

	*ppvObject = NULL;
	return E_NOINTERFACE;
}


HRESULT STDMETHODCALLTYPE HTMLexPlugin::GetSite(REFIID riid, LPVOID *ppvSite)
{
	*ppvSite = NULL;
	if (!mWebBrowser)
		return E_FAIL;
	return mWebBrowser->QueryInterface(riid, ppvSite);
}

HRESULT STDMETHODCALLTYPE HTMLexPlugin::SetSite(IUnknown *pUnknown)
{
	clear();

	if (!pUnknown)
	{
		// we're supposed to release the currently set site, which we have already done
		return S_OK;
	}

	// save web browser interface
	if (FAILED(pUnknown->QueryInterface(IID_IWebBrowser2,
		reinterpret_cast<LPVOID *>(&mWebBrowser))))
		goto error;

	// register to receive browser events
	if (FAILED(pUnknown->QueryInterface(IID_IConnectionPointContainer,
		reinterpret_cast<LPVOID *>(&mConnectionPointContainer))))
		goto error;

	IConnectionPoint *cp;
	HRESULT rc = mConnectionPointContainer->FindConnectionPoint(DIID_DWebBrowserEvents2, &cp);
	if (FAILED(rc) || !cp)
		goto error;

	DWORD cookie;
	rc = cp->Advise(static_cast<IDispatch *>(this), &cookie);
	cp->Release();
	if (FAILED(rc) || (cookie == 0))
		goto error;

	return S_OK;

error:
	clear();
	return S_OK;  // MSDN: "Returns S_OK in all circumstances."
}


HRESULT STDMETHODCALLTYPE HTMLexPlugin::GetTypeInfoCount(unsigned int *)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE HTMLexPlugin::GetTypeInfo(unsigned int, LCID, ITypeInfo **)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE HTMLexPlugin::GetIDsOfNames(REFIID, OLECHAR **, unsigned int,
	LCID, DISPID *)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE HTMLexPlugin::Invoke(DISPID dispIdMember, REFIID,
	LCID, WORD, DISPPARAMS *, VARIANT *, EXCEPINFO *, unsigned int *)
{
	switch (dispIdMember)
	{
	case DISPID_DOCUMENTCOMPLETE:
		// ready to process requests; no guaranteed sequence of DISPID notifications, so
		// the mutex might have been locked recursively -> unlock completely
		while (ReleaseMutex(mMutex))
			;
		break;
	case DISPID_BEFORENAVIGATE2:
		// document will be unloaded
		WaitForSingleObject(mMutex, INFINITE);  // acquire mutex
		break;
	}

	return S_OK;
}


DWORD WINAPI HTMLexPlugin::commThreadProc(LPVOID param)
{
	HTMLexPlugin *parent = reinterpret_cast<HTMLexPlugin *>(param);

	HANDLE pipe = CreateNamedPipe(_T(HTMLEX_PIPE_NAME),
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		1, 0, 0, 0, NULL);
	if (pipe == INVALID_HANDLE_VALUE)
	{
		OutputDebugString(_T("HTMLexPlugin: Could not create named pipe."));
		return 1;
	}

	OVERLAPPED pipeStatus;
	pipeStatus.Offset = pipeStatus.OffsetHigh = 0;
	pipeStatus.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	DWORD bytesTransf;
	BOOL success;
	request_t requestBuf;
	response_t responseBuf;
	enum { STATE_CONNECT, STATE_READ, STATE_PROCESS, STATE_RESPOND, STATE_WAIT, STATE_ERROR }
		serverState = STATE_CONNECT, nextServerState;

	while (parent->mThreadState)
	{
		switch (serverState)
		{
		case STATE_WAIT:
			if (WaitForSingleObject(pipeStatus.hEvent, timeOutMs) != WAIT_OBJECT_0)
				break;
			success = GetOverlappedResult(pipe, &pipeStatus, &bytesTransf, FALSE);
			ResetEvent(pipeStatus.hEvent);
			if (!success &&
				(GetLastError() != ERROR_IO_PENDING) &&
				(GetLastError() != ERROR_PIPE_CONNECTED))
				serverState = STATE_ERROR;
			else
				serverState = nextServerState;
			break;

		case STATE_CONNECT:
			// wait for client to connect
			ConnectNamedPipe(pipe, &pipeStatus);
			serverState = STATE_WAIT;
			nextServerState = STATE_READ;
			break;

		case STATE_READ:
			if (ReadFile(pipe, &requestBuf, sizeof(request_t), NULL, &pipeStatus) ||
				(GetLastError() == ERROR_IO_PENDING))
			{
				serverState = STATE_WAIT;
				nextServerState = STATE_PROCESS;
			}
			else
				serverState = STATE_ERROR;
			break;

		case STATE_PROCESS:
			if (WaitForSingleObject(parent->mMutex, timeOutMs) == WAIT_OBJECT_0)
			{
				// acquired mutex, process request
				processRequest(parent, requestBuf, responseBuf);
				serverState = STATE_RESPOND;
			}
			break;

		case STATE_RESPOND:
			// send response synchronously
			WriteFile(pipe, &responseBuf, sizeof(response_t), NULL, &pipeStatus);
			success = GetOverlappedResult(pipe, &pipeStatus, &bytesTransf, TRUE);
			ResetEvent(pipeStatus.hEvent);
			if (success && (bytesTransf == sizeof(response_t)))
				serverState = STATE_READ;
			else
				serverState = STATE_ERROR;
			break;

		case STATE_ERROR:
		default:
			// An error occurred, most likely the pipe has been closed by the client.
			// Disconnect pipe and wait for another client to connect.
			DisconnectNamedPipe(pipe);
			serverState = STATE_CONNECT;
			break;
		}
	}

	CloseHandle(pipe);
	return 0;
}


void HTMLexPlugin::processRequest(HTMLexPlugin *parent, const request_t &request,
								  response_t &response)
{
	// check whether we are the intended recipient
	long instanceToken = 0;
	parent->mWebBrowser->get_HWND(&instanceToken);
	if (request.token != instanceToken)
	{
		response.result = ERR_BAD_TOKEN;
		return;
	}

	IHTMLComputedStyle *pComputedStyle = getComputedStyle(parent, request.elementIndex);
	if (pComputedStyle)
	{
		response.result = 0;

		response.result |= pComputedStyle->get_backgroundColor(&response.bgColor);

		VARIANT_BOOL valueBool;
		response.result |= pComputedStyle->get_blockDirection(&valueBool);
		response.blockDir = (valueBool == VARIANT_TRUE ? ~0 : 0);

		// There appears to be no way to tell get_fontName about the size of the provided
		// buffer, so if the size of the font name exceeds MAX_FONT_NAME characters, the stack
		// might be corrupted. The following code provides a crude safeguard.
		response.fontName[MAX_FONT_NAME - 1] = 0;
		response.result |= pComputedStyle->get_fontName(
			reinterpret_cast<TCHAR *>(&response.fontName));
		if (response.fontName[MAX_FONT_NAME - 1] != 0)
		{
			OutputDebugString(_T("HTMLexPlugin: Stack corruption detected."));
			ExitThread(23);
		}

		response.result |= pComputedStyle->get_fontSize(&response.fontSize);
		response.result |= pComputedStyle->get_fontWeight(&response.fontWeight);
		response.result |= pComputedStyle->get_textColor(&response.textColor);
	}
	else
		response.result = ERR_BAD_ELEMENT;
}

IHTMLComputedStyle *HTMLexPlugin::getComputedStyle(HTMLexPlugin *parent, long elementIndex)
{
	HRESULT rc;

	// get current document
	IDispatch *pDisp = NULL;
	rc = parent->mWebBrowser->get_Document(&pDisp);
	if (FAILED(rc) || !pDisp)
		return NULL;

	IHTMLDocument2 *pDoc;
	rc = pDisp->QueryInterface(IID_IHTMLDocument2, reinterpret_cast<LPVOID *>(&pDoc));
	if (FAILED(rc))
		goto error1;

	// get IHTMLElement from index
	IHTMLElementCollection *pCollection = NULL;
	rc = pDoc->get_all(&pCollection);
	pDoc->Release();
	if (FAILED(rc) || !pCollection)
		goto error1;

	IDispatch *pItemDisp = NULL;
	VARIANT varIndex, varNull;
	varIndex.vt = VT_I4;
	varIndex.lVal = elementIndex;
	varNull.vt = VT_I4;
	varNull.lVal = 0;
	rc = pCollection->item(varIndex, varNull, &pItemDisp);
	pCollection->Release();
	if (FAILED(rc) || !pItemDisp)
		goto error1;

	IHTMLElement *pElement;
	rc = pItemDisp->QueryInterface(IID_IHTMLElement, reinterpret_cast<LPVOID *>(&pElement));
	pItemDisp->Release();
	if (FAILED(rc))
		goto error1;

	// get IDisplayServices and IMarkupServices
	IDisplayServices *pDisplayServices;
	rc = pDisp->QueryInterface(IID_IDisplayServices,
		reinterpret_cast<LPVOID *>(&pDisplayServices));
	if (FAILED(rc))
		goto error1;

	IMarkupServices *pMarkupServices;
	rc = pDisp->QueryInterface(IID_IMarkupServices,
		reinterpret_cast<LPVOID *>(&pMarkupServices));
	pDisp->Release();
	if (FAILED(rc))
		goto error2;

	// get IHTMLComputedStyle from IHTMLElement
	IMarkupPointer *pMarkupPointer = NULL;
	rc = pMarkupServices->CreateMarkupPointer(&pMarkupPointer);
	pMarkupServices->Release();
	if (FAILED(rc) || !pMarkupPointer)
		goto error2;

	pMarkupPointer->MoveAdjacentToElement(pElement, ELEM_ADJ_AfterBegin);
	pElement->Release();

	IHTMLComputedStyle *pComputedStyle = NULL;
	rc = pDisplayServices->GetComputedStyle(pMarkupPointer, &pComputedStyle);
	if (FAILED(rc))
		pComputedStyle = NULL;
	pMarkupPointer->Release();
	pDisplayServices->Release();

	return pComputedStyle;

error1:
	pDisp->Release();
	return NULL;
error2:
	pDisplayServices->Release();
	return NULL;
}


ClassFactory::ClassFactory() : mRefCount(0)
{
}

ULONG STDMETHODCALLTYPE ClassFactory::AddRef()
{
	InterlockedIncrement(&globalRefCount);
	return InterlockedIncrement(&mRefCount);
}

ULONG STDMETHODCALLTYPE ClassFactory::Release()
{
	int c = InterlockedDecrement(&mRefCount);
	InterlockedDecrement(&globalRefCount);
	return c;
}

HRESULT STDMETHODCALLTYPE ClassFactory::QueryInterface(REFIID riid, LPVOID *ppvObject)
{
	if ((riid == IID_IUnknown) ||
		(riid == IID_IClassFactory))
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
		
	*ppvObject = NULL;
	return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE ClassFactory::LockServer(BOOL b)
{
	if (b)
		InterlockedIncrement(&globalRefCount);
	else
		InterlockedDecrement(&globalRefCount);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE ClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid,
	LPVOID *ppvObject)
{
	*ppvObject = NULL;
	if (pUnkOuter)
		return CLASS_E_NOAGGREGATION;

	HTMLexPlugin *plugin;
	try
	{
		plugin = new HTMLexPlugin();
	}
	catch (const std::bad_alloc &)
	{
		return E_OUTOFMEMORY;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}

	plugin->AddRef();
	HRESULT rc = plugin->QueryInterface(riid, ppvObject);
	plugin->Release();
	return rc;
}
