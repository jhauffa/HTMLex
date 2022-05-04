#ifndef __HTML_BLOCK_H__
#define __HTML_BLOCK_H__

#include <string>
#include <map>


typedef std::map<std::string, std::string> AttributeMap;

struct HTMLBlock {
	int depth;
	long left, top, right, bottom;
	AttributeMap attributes;
	std::string text;
};

#endif  // __HTML_BLOCK_H__
