
#include "COMHelper.h"

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <wchar.h>


void BString::convertToUtf8(const BSTR srcStr, std::string &destStr)
{
	// convert UTF-16 to UTF-8
	int bufSize = WideCharToMultiByte(CP_UTF8, 0, srcStr, -1, NULL, 0, NULL, NULL);
	if (bufSize > 0)
	{
		char *mbBuf = new char[bufSize];
		*mbBuf = '\0';
		WideCharToMultiByte(CP_UTF8, 0, srcStr, -1, mbBuf, bufSize, NULL, NULL);
		destStr.assign(mbBuf);
		delete[] mbBuf;
	}
	else
		destStr.clear();
}

BSTR BString::createFromAnsi(const char *str)
{
	int bufSize = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	if (bufSize > 0)
	{
		wchar_t *wideBuf = new wchar_t[bufSize];
		*wideBuf = 0;
		MultiByteToWideChar(CP_ACP, 0, str, -1, wideBuf, bufSize);
		BSTR tmp = SysAllocString(wideBuf);
		delete[] wideBuf;
		return tmp;
	}
	else
		return SysAllocString(L"");
}

bool BString::isEmptyAnsiString(const char *str)
{
	while (*str)
	{
		char c = *str++;
		if ((c > 0x20) && (c != ' ') && (c != 0x7f))
			return false;
	}

	return true;
}
