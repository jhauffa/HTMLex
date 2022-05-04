
#include "PluginInterface.h"
#include "Exception.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mshtml.h>
#include <exdisp.h>


PluginInterface::PluginInterface(COMObject<IWebBrowser2> pBrowser,
								 COMObject<IHTMLElement> pElement)
{
	long elementIndex;
	HRESULT rc = pElement->get_sourceIndex(&elementIndex);
	if (FAILED(rc))
		throw COMException("could not get element index", rc);

	long token;
	rc = pBrowser->get_HWND(&token);
	if (FAILED(rc))
		throw COMException("could not get instance token", rc);

	sendRequest(elementIndex, token, &mElementData);
}


void PluginInterface::sendRequest(long elementIndex, long token, response_t *response)
{
	DWORD result;

	// connect to HTMLex plugin
	HANDLE pipe = CreateFile(HTMLEX_PIPE_NAME_W, GENERIC_READ | GENERIC_WRITE, 0, NULL,
	                         OPEN_EXISTING, 0, NULL);
	if (pipe == INVALID_HANDLE_VALUE)
		throw Exception("No connection to IE plugin.");	

	DWORD pipeMode = PIPE_READMODE_MESSAGE;
	SetNamedPipeHandleState(pipe, &pipeMode, NULL, NULL);

	// send message
	request_t requestBuf;
	requestBuf.token = token;
	requestBuf.elementIndex = elementIndex;
	if (!WriteFile(pipe, &requestBuf, sizeof(request_t), &result, NULL))
	{
		CloseHandle(pipe);
		throw Exception("error sending request");
	}

	// receive response
	if (!ReadFile(pipe, response, sizeof(response_t), &result, NULL))
	{
		CloseHandle(pipe);
		throw Exception("error reading response");
	}

	if (response->result != 0)
	{
		if (response->result == ERR_BAD_TOKEN)
			throw Exception("More than one instance of Internet Explorer is running.\n" \
			                "Close all instances and try again.");
		else
			throw Exception("COM error in plugin");
	}
	CloseHandle(pipe);
}
