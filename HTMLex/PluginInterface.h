#ifndef __PLUGIN_INTERFACE_H__
#define __PLUGIN_INTERFACE_H__

#include "PluginRemoteInterface.h"
#include "COMHelper.h"


struct IHTMLElement;
struct IWebBrowser2;


class PluginInterface {
public:
	PluginInterface(COMObject<IWebBrowser2> pBrowser,
	                COMObject<IHTMLElement> pElement);

	unsigned long getBackgroundColor() const
	{
		return mElementData.bgColor;
	}

	unsigned long getTextColor() const
	{
		return mElementData.textColor;
	}

	long getBlockDirection() const
	{
		return mElementData.blockDir;
	}

	const wchar_t *getFontName() const
	{
		return mElementData.fontName;
	}

	long getFontSize() const
	{
		return mElementData.fontSize;
	}

	long getFontWeight() const
	{
		return mElementData.fontWeight;
	}

private:
	response_t mElementData;

	void sendRequest(long elementIndex, long token, response_t *response);
};

#endif  // __PLUGIN_INTERFACE_H__
