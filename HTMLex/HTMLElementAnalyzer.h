#ifndef __HTML_ELEMENT_ANALYZER_H__
#define __HTML_ELEMENT_ANALYZER_H__

#include "PluginInterface.h"
#include "COMHelper.h"

#include <string>
#include <map>
#include <oaidl.h>  // VARIANT
#include <wtypes.h>  // BSTR


struct IHTMLElement;
struct IHTMLElement2;
struct IHTMLCurrentStyle;
struct IHTMLCurrentStyle3;
struct IWebBrowser2;


class HTMLElementAnalyzer {
public:
	HTMLElementAnalyzer(COMObject<IWebBrowser2> pBrowser,
	                    COMObject<IHTMLElement> pElement);

	void getBounds(long &left, long &top, long &right, long &bottom);
	std::string getAttribute(const std::string &name);
	void getText(std::string &text);

	static std::string getAttributeNames();

private:
	COMObject<IHTMLElement> mpElement;
	COMObject<IHTMLElement2> mpElement2;
	COMObject<IHTMLCurrentStyle> mpCurrentStyle;
	COMObject<IHTMLCurrentStyle3> mpCurrentStyle3;
	PluginInterface mPluginInterface;

	typedef HRESULT (*GetFunction)(HTMLElementAnalyzer &, VARIANT &);
	typedef std::map<std::string, GetFunction> AttributeMap;
	typedef std::map<std::wstring, std::wstring> ColorMap;
	static const AttributeMap::value_type mAttributeInfo[];
	static AttributeMap mAttr;
	static const ColorMap::value_type mColorNames[];
	static ColorMap mColorMap;

	static void canonicalizeHtmlColor(BSTR *colorStr);
	static void intToHtmlColor(long colorVal, BSTR *colorStr);

	static HRESULT getBackgroundColor(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getBlockDirection(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getColor(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getFontFamily(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getFontSize(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getFontStyle(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getFontWeight(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getLetterSpacing(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getLineHeight(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getTextAlign(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getTextDecoration(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getTextIndent(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getWordSpacing(HTMLElementAnalyzer &parent, VARIANT &v);

	static HRESULT getRealBackgroundColor(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getRealColor(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getRealBlockDirection(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getRealFontFamily(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getRealFontSize(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getRealFontStyle(HTMLElementAnalyzer &parent, VARIANT &v);
	static HRESULT getRealFontWeight(HTMLElementAnalyzer &parent, VARIANT &v);
};


#endif  // __HTML_ELEMENT_ANALYZER_H__
