#ifndef __PLUGIN_REMOTE_INTERFACE_H__
#define __PLUGIN_REMOTE_INTERFACE_H__

#include <wchar.h>

#define HTMLEX_PIPE_NAME     "\\\\.\\pipe\\HTMLexComm"
#define HTMLEX_PIPE_NAME_W  L"\\\\.\\pipe\\HTMLexComm"


struct request_t {
	long token;
	long elementIndex;
};


#define MAX_FONT_NAME	64

#define ERR_BAD_ELEMENT	-1
#define ERR_BAD_TOKEN	-2

struct response_t {
	unsigned long result;
	unsigned long bgColor;
	unsigned long textColor;
	long blockDir;
	wchar_t fontName[MAX_FONT_NAME];
	long fontSize;
	long fontWeight;
};


#endif  // __PLUGIN_REMOTE_INTERFACE_H__
