#ifndef __HTMLEX_PLUGIN_H__
#define __HTMLEX_PLUGIN_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ocidl.h>
#include <exdisp.h>
#include <mshtml.h>

#include "PluginRemoteInterface.h"


class HTMLexPlugin : public IObjectWithSite, public IDispatch {
public:
	HTMLexPlugin();
	~HTMLexPlugin();

	void clear();


	// methods from IUnknown

	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppvObject);


	// methods from IObjectWithSite

	HRESULT STDMETHODCALLTYPE GetSite(REFIID riid, LPVOID *ppvSite);
	HRESULT STDMETHODCALLTYPE SetSite(IUnknown *pUnknown);


	// methods from IDispatch

	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(unsigned int *pctinfo);
	HRESULT STDMETHODCALLTYPE GetTypeInfo(unsigned int iTInfo, LCID lcid,
		ITypeInfo **ppTInfo);

	HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, 
		unsigned int cNames, LCID lcid, DISPID *rgDispId);

	HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
		WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult,
		EXCEPINFO *pExcepInfo, unsigned int *puArgErr); 


private:
	long mRefCount;
	IWebBrowser2 *mWebBrowser;
	IConnectionPointContainer *mConnectionPointContainer;
	HANDLE mCommThread, mMutex;
	volatile unsigned int mThreadState;

	static const DWORD timeOutMs;

	static DWORD WINAPI commThreadProc(LPVOID param);
	static void processRequest(HTMLexPlugin *parent, const request_t &request,
		response_t &response);
	static IHTMLComputedStyle *getComputedStyle(HTMLexPlugin *parent, long elementIndex);
};


class ClassFactory : public IClassFactory {
public:
	ClassFactory();


	// methods from IUnknown

	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppvObject);


	// methods from IClassFactory

	HRESULT STDMETHODCALLTYPE LockServer(BOOL b);
	HRESULT STDMETHODCALLTYPE CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID *ppvObj);


private:
	long mRefCount;
};


#endif  // __HTMLEX_PLUGIN_H__
