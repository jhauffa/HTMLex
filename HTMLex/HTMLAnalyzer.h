#ifndef __HTML_ANALYZER_H__
#define __HTML_ANALYZER_H__

#include "HTMLBlock.h"
#include "Writer.h"
#include "COMHelper.h"

#include <string>
#include <list>


struct IWebBrowser2;
struct IHTMLDocument2;
struct IHTMLDOMNode;
struct IHTMLElement;


class HTMLAnalyzer {
public:
	HTMLAnalyzer(int resX, int resY);
	~HTMLAnalyzer();
	
	void findBlocks(const std::string &url, const std::list<std::string> &attr,
	                Writer &writer);

private:
	COMObject<IWebBrowser2> mpBrowser;

	COMObject<IHTMLDocument2> retrieveDocument(const std::string &url);
	void parseDOMNode(COMObject<IHTMLDOMNode> pNode, const std::list<std::string> &attr,
	                  Writer &writer, int depth) const;
	void parseHTMLElement(COMObject<IHTMLElement> pElement, const std::list<std::string> &attr,
						  Writer &writer, int depth) const;
};

#endif  // __HTML_ANALYZER_H__
