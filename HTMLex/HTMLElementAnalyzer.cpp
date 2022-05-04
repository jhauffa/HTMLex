
#include "HTMLElementAnalyzer.h"
#include "Exception.h"

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <mshtml.h>
#include <exdisp.h>
#include <sstream>
#include <iomanip>


const HTMLElementAnalyzer::AttributeMap::value_type
	HTMLElementAnalyzer::mAttributeInfo[] = {
	AttributeMap::value_type("backgroundColor",     getBackgroundColor    ),
	AttributeMap::value_type("fontFamily",          getFontFamily         ),
	AttributeMap::value_type("fontSize",            getFontSize           ),
	AttributeMap::value_type("fontStyle",           getFontStyle          ),
	AttributeMap::value_type("fontWeight",          getFontWeight         ),
	AttributeMap::value_type("letterSpacing",       getLetterSpacing      ),
	AttributeMap::value_type("lineHeight",          getLineHeight         ),
	AttributeMap::value_type("realBackgroundColor", getRealBackgroundColor),
	AttributeMap::value_type("realTextColor",       getRealColor          ),
	AttributeMap::value_type("realTextDirection",   getRealBlockDirection ),
	AttributeMap::value_type("realFontFamily",      getRealFontFamily     ),
	AttributeMap::value_type("realFontSize",        getRealFontSize       ),
	AttributeMap::value_type("realFontWeight",      getRealFontWeight     ),
	AttributeMap::value_type("textAlign",           getTextAlign          ),
	AttributeMap::value_type("textColor",           getColor              ),
	AttributeMap::value_type("textDecoration",      getTextDecoration     ),
	AttributeMap::value_type("textDirection",       getBlockDirection     ),
	AttributeMap::value_type("textIndent",          getTextIndent         ),
	AttributeMap::value_type("wordSpacing",         getWordSpacing        )
};

HTMLElementAnalyzer::AttributeMap HTMLElementAnalyzer::mAttr(mAttributeInfo,
	mAttributeInfo + (sizeof(mAttributeInfo) / sizeof(mAttributeInfo[0])));

#include "ColorMap.inc"

HTMLElementAnalyzer::ColorMap HTMLElementAnalyzer::mColorMap(mColorNames,
	mColorNames + (sizeof(mColorNames) / sizeof(mColorNames[0])));


HTMLElementAnalyzer::HTMLElementAnalyzer(COMObject<IWebBrowser2> pBrowser,
										 COMObject<IHTMLElement> pElement)
	: mpElement(pElement), mpElement2(), mpCurrentStyle(), mpCurrentStyle3(),
	  mPluginInterface(pBrowser, pElement)
{
	HRESULT rc;

	// get IHTMLCurrentStyle
	rc = mpElement->QueryInterface(IID_IHTMLElement2,
	                               reinterpret_cast<LPVOID *>(&mpElement2));
	if (FAILED(rc))
		throw COMException("error creating IHTMLElement2 from IHTMLElement", rc);

	rc = mpElement2->get_currentStyle(&mpCurrentStyle);
	if (FAILED(rc) || !mpCurrentStyle)
		throw COMException("could not get current style", rc);

	rc = mpCurrentStyle->QueryInterface(IID_IHTMLCurrentStyle3,
	                                    reinterpret_cast<LPVOID *>(&mpCurrentStyle3));
	if (FAILED(rc))
		throw COMException("error creating IHTMLCurrentStyle3 from IHTMLCurrentStyle", rc);
}

void HTMLElementAnalyzer::getBounds(long &left, long &top, long &right,
									long &bottom)
{
	COMObject<IHTMLRect> pBoundingRect;
	HRESULT rc = mpElement2->getBoundingClientRect(&pBoundingRect);
	if (FAILED(rc) || !pBoundingRect)
		throw COMException("could not get bounding rect for element", rc);

	pBoundingRect->get_left(&left);
	pBoundingRect->get_top(&top);
	pBoundingRect->get_right(&right);
	pBoundingRect->get_bottom(&bottom);
}

std::string HTMLElementAnalyzer::getAttribute(const std::string &name)
{
	AttributeMap::iterator pos = mAttr.find(name);
	if (pos == mAttr.end())
		throw Exception("unknown attribute: " + name);

	VARIANT v;
	v.vt = VT_BSTR;  // most getter functions set v.bstrVal directly
	HRESULT rc = (pos->second)(*this, v);
	if (FAILED(rc))
		throw COMException("error retrieving attribute " + name, rc);

	// convert to string; use a fixed locale to get consistent behavior on all systems
	VariantChangeTypeEx(&v, &v, 0x0409, 0, VT_BSTR);
	std::string tmp;
	BString::convertToUtf8(v.bstrVal, tmp);
	SysFreeString(v.bstrVal);
	return tmp;
}

void HTMLElementAnalyzer::getText(std::string &text)
{
	BSTR bstr;
	mpElement->get_innerText(&bstr);
	BString::convertToUtf8(bstr, text);
	SysFreeString(bstr);
}

std::string HTMLElementAnalyzer::getAttributeNames()
{
	AttributeMap::const_iterator it = mAttr.begin();
	std::string tmp = it->first;
	int i = 1;
	for (++it; it != mAttr.end(); ++it)
	{
		if ((i++ % 4) == 0)  // insert line breaks for better readability
			tmp += "\n\t";
		else
			tmp += ", ";
		tmp += it->first;
	}
	return tmp;
}


void HTMLElementAnalyzer::canonicalizeHtmlColor(BSTR *colorStr)
{
	static const std::wstring defaultColor(L"#000000");
	std::wstring colorList(*colorStr);
	std::wstring canonicalColor;

	// extract single color identifiers separated by whitespace
	std::wstring::size_type start = 0, len;
	do
	{
		len = colorList.find(' ', start);
		if (len == std::wstring::npos)
			len = colorList.length();
		len -= start;

		if (len > 0)
		{
			std::wstring color = colorList.substr(start, len);

			if (start != 0)
				canonicalColor += ' ';

			if (color[0] == '#')  // hexadecimal color value
			{
				if (len == 4)
				{
					// convert #rgb to #rrggbb
					canonicalColor += '#';
					for (int i = 0; i < 3; i++)
					{
						canonicalColor += color[i + 1];
						canonicalColor += color[i + 1];
					}
				}
				else
					canonicalColor += color;
			}
			else  // named color
			{
				// use a map to find the associated RGB values; fortunately, IE always gives
				// us lower case color names
				ColorMap::iterator pos = mColorMap.find(color);
				if (pos != mColorMap.end())
					canonicalColor += pos->second;
				else
					canonicalColor += defaultColor;
			}
		}

		start += len + 1;
	} while (start < colorList.length());

	// convert canonicalColor to BSTR
	SysFreeString(*colorStr);
	*colorStr = SysAllocString(canonicalColor.c_str());
}

void HTMLElementAnalyzer::intToHtmlColor(long colorVal, BSTR *colorStr)
{
	std::wostringstream wos;
	wos.fill('0');
	wos << '#' << std::hex << std::setw(6) << (colorVal & 0xffffff);
	*colorStr = SysAllocString(wos.str().c_str());
}


HRESULT HTMLElementAnalyzer::getBackgroundColor(HTMLElementAnalyzer &parent, VARIANT &v)
{
	HRESULT rc = parent.mpCurrentStyle->get_backgroundColor(&v);
	canonicalizeHtmlColor(&v.bstrVal);
	return rc;
}

HRESULT HTMLElementAnalyzer::getBlockDirection(HTMLElementAnalyzer &parent, VARIANT &v)
{
	return parent.mpCurrentStyle->get_blockDirection(&v.bstrVal);
}

HRESULT HTMLElementAnalyzer::getColor(HTMLElementAnalyzer &parent, VARIANT &v)
{
	HRESULT rc = parent.mpCurrentStyle->get_color(&v);
	canonicalizeHtmlColor(&v.bstrVal);
	return rc;
}

HRESULT HTMLElementAnalyzer::getFontFamily(HTMLElementAnalyzer &parent, VARIANT &v)
{
	return parent.mpCurrentStyle->get_fontFamily(&v.bstrVal);
}

HRESULT HTMLElementAnalyzer::getFontSize(HTMLElementAnalyzer &parent, VARIANT &v)
{
	return parent.mpCurrentStyle->get_fontSize(&v);
}

HRESULT HTMLElementAnalyzer::getFontStyle(HTMLElementAnalyzer &parent, VARIANT &v)
{
	return parent.mpCurrentStyle->get_fontStyle(&v.bstrVal);
}

HRESULT HTMLElementAnalyzer::getFontWeight(HTMLElementAnalyzer &parent, VARIANT &v)
{
	return parent.mpCurrentStyle->get_fontWeight(&v);
}

HRESULT HTMLElementAnalyzer::getLetterSpacing(HTMLElementAnalyzer &parent, VARIANT &v)
{
	return parent.mpCurrentStyle->get_letterSpacing(&v);
}

HRESULT HTMLElementAnalyzer::getLineHeight(HTMLElementAnalyzer &parent, VARIANT &v)
{
	return parent.mpCurrentStyle->get_lineHeight(&v);
}

HRESULT HTMLElementAnalyzer::getTextAlign(HTMLElementAnalyzer &parent, VARIANT &v)
{
	return parent.mpCurrentStyle->get_textAlign(&v.bstrVal);
}

HRESULT HTMLElementAnalyzer::getTextDecoration(HTMLElementAnalyzer &parent, VARIANT &v)
{
	return parent.mpCurrentStyle->get_textDecoration(&v.bstrVal);
}

HRESULT HTMLElementAnalyzer::getTextIndent(HTMLElementAnalyzer &parent, VARIANT &v)
{
	return parent.mpCurrentStyle->get_textIndent(&v);
}

HRESULT HTMLElementAnalyzer::getWordSpacing(HTMLElementAnalyzer &parent, VARIANT &v)
{
	return parent.mpCurrentStyle3->get_wordSpacing(&v);
}


HRESULT HTMLElementAnalyzer::getRealBackgroundColor(HTMLElementAnalyzer &parent, VARIANT &v)
{
	intToHtmlColor(parent.mPluginInterface.getBackgroundColor(), &v.bstrVal);
	return 0;
}

HRESULT HTMLElementAnalyzer::getRealColor(HTMLElementAnalyzer &parent, VARIANT &v)
{
	intToHtmlColor(parent.mPluginInterface.getTextColor(), &v.bstrVal);
	return 0;
}

HRESULT HTMLElementAnalyzer::getRealBlockDirection(HTMLElementAnalyzer &parent, VARIANT &v)
{
	if (parent.mPluginInterface.getBlockDirection() == 0)
		v.bstrVal = SysAllocString(L"ltr");
	else
		v.bstrVal = SysAllocString(L"rtl");
	return 0;
}

HRESULT HTMLElementAnalyzer::getRealFontFamily(HTMLElementAnalyzer &parent, VARIANT &v)
{
	v.bstrVal = SysAllocString(parent.mPluginInterface.getFontName());
	return 0;
}

HRESULT HTMLElementAnalyzer::getRealFontSize(HTMLElementAnalyzer &parent, VARIANT &v)
{
	v.vt = VT_R4;
	v.fltVal = parent.mPluginInterface.getFontSize() / 20.0f;
	return 0;
}

HRESULT HTMLElementAnalyzer::getRealFontWeight(HTMLElementAnalyzer &parent, VARIANT &v)
{
	v.vt = VT_I4;
	v.lVal = parent.mPluginInterface.getFontWeight();
	return 0;
}
