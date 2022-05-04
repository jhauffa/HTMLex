
#include "HTMLAnalyzer.h"
#include "HTMLElementAnalyzer.h"
#include "Exception.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mshtml.h>
#include <exdisp.h>


HTMLAnalyzer::HTMLAnalyzer(int resX, int resY)
{
	HRESULT rc;
	
	CLSID clsid;
	CLSIDFromProgID(OLESTR("InternetExplorer.Application"), &clsid);
	COMObject<IUnknown> pUnknown;
	rc = CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IUnknown,
	                      reinterpret_cast<LPVOID *>(&pUnknown));
	if (FAILED(rc))
		throw COMException("error creating IUnknown", rc);

    rc = pUnknown->QueryInterface(IID_IWebBrowser2,
	                              reinterpret_cast<LPVOID *>(&mpBrowser));
	if (FAILED(rc))
		throw COMException("error creating IWebBrowser2", rc);

	mpBrowser->put_Width(resX);
	mpBrowser->put_Height(resY);
	mpBrowser->put_Visible(FALSE);
}

HTMLAnalyzer::~HTMLAnalyzer()
{
	mpBrowser->Quit();
}


COMObject<IHTMLDocument2>
	HTMLAnalyzer::retrieveDocument(const std::string &url)
{
	HRESULT rc;

	VARIANT vEmpty;
	VariantInit(&vEmpty);
	BSTR urlBStr = BString::createFromAnsi(url.c_str());
	mpBrowser->Navigate(urlBStr, &vEmpty, &vEmpty, &vEmpty, &vEmpty);
	SysFreeString(urlBStr);

	READYSTATE state;
	do
	{
		Sleep(500);
		mpBrowser->get_ReadyState(&state);
	} while (state != READYSTATE_COMPLETE);

	COMObject<IDispatch> pDisp;
	rc = mpBrowser->get_Document(&pDisp);
	if (FAILED(rc) || !pDisp)
		throw COMException("error retrieving URL", rc);

	COMObject<IHTMLDocument2> pDocument;
	rc = pDisp->QueryInterface(IID_IHTMLDocument2,
		                       reinterpret_cast<LPVOID *>(&pDocument));
	if (FAILED(rc))
		throw COMException("error creating IHTMLDocument2", rc);

	return pDocument;
}

void HTMLAnalyzer::parseHTMLElement(COMObject<IHTMLElement> pElement,
                                    const std::list<std::string> &attr,
                                    Writer &writer, int depth) const
{
	HTMLBlock block;
	block.depth = depth;
	block.left = block.top = block.right = block.bottom = 0;

	HTMLElementAnalyzer ea(mpBrowser, pElement);
	ea.getBounds(block.left, block.top, block.right, block.bottom);
	for (std::list<std::string>::const_iterator it = attr.begin();
	     it != attr.end(); ++it)
		block.attributes[*it] = ea.getAttribute(*it);
	ea.getText(block.text);

	if ((((block.right - block.left) > 0) &&
	     ((block.bottom - block.top) > 0)) &&
		!BString::isEmptyAnsiString(block.text.c_str()))
		writer.writeBlock(block);
}

void HTMLAnalyzer::parseDOMNode(COMObject<IHTMLDOMNode> pNode,
                                const std::list<std::string> &attr,
                                Writer &writer, int depth) const
{
	HRESULT rc;

	long nodeType = 0;
	pNode->get_nodeType(&nodeType);
	if (nodeType != 1)	// unknown node type?
		return;

	COMObject<IHTMLDOMNode> pChildNode;
	rc = pNode->get_firstChild(&pChildNode);
	bool isTextNode = false;
	while (SUCCEEDED(rc) && (*pChildNode) && !isTextNode)
	{
		nodeType = 0;
		pChildNode->get_nodeType(&nodeType);
		if (nodeType == 3)
			isTextNode = true;

		COMObject<IHTMLDOMNode> pTmp;
		rc = pChildNode->get_nextSibling(&pTmp);
		pChildNode = pTmp;
	}

	if (isTextNode)
	{
		COMObject<IHTMLElement> pElement;
		rc = pNode->QueryInterface(IID_IHTMLElement,
			                       reinterpret_cast<LPVOID *>(&pElement));
		if (FAILED(rc))
			throw COMException("error creating IHTMLElement from IHTMLDOMNode",
			                   rc);

		parseHTMLElement(pElement, attr, writer, depth);
	}
	else
	{
		rc = pNode->get_firstChild(&pChildNode);
		while (SUCCEEDED(rc) && (*pChildNode))
		{
			parseDOMNode(pChildNode, attr, writer, depth + 1);

			COMObject<IHTMLDOMNode> pTmp;
			rc = pChildNode->get_nextSibling(&pTmp);
			pChildNode = pTmp;
		}
	}
}

void HTMLAnalyzer::findBlocks(const std::string &url,
                              const std::list<std::string> &attr,
                              Writer &writer)
{
	HRESULT rc;
	COMObject<IHTMLDocument2> pDoc = retrieveDocument(url);

	COMObject<IHTMLElement> pBodyElement;
	rc = pDoc->get_body(&pBodyElement);
	if (FAILED(rc) || !pBodyElement)
		throw COMException("could not get body element of HTML document", rc);

	COMObject<IHTMLDOMNode> pBodyNode;
	rc = pBodyElement->QueryInterface(IID_IHTMLDOMNode,
		reinterpret_cast<LPVOID *>(&pBodyNode));
	if (FAILED(rc))
		throw COMException("error creating IHTMLDOMNode from IHTMLElement", rc);

	int depth = 0;
	parseDOMNode(pBodyNode, attr, writer, depth);
}
