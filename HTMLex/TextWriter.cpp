
#include "TextWriter.h"
#include "HTMLBlock.h"

#include <string>


const char TextWriter::mDelim[] = "####";


std::string TextWriter::escapeString(const std::string &str) const
{
	std::string tmp = str;

	std::string::size_type pos = tmp.find(mDelim);
	while (pos != std::string::npos)
	{
		tmp[pos + (sizeof(mDelim) - 2)] = ' ';
		pos = tmp.find(mDelim, pos + (sizeof(mDelim) - 1));
	}

	return tmp;
}

void TextWriter::writeBlock(const HTMLBlock &block)
{
	mStream << block.left << ' ' << block.top << ' ' << block.right << ' '
	        << block.bottom << ' ' << block.depth << '\n';

	for (AttributeMap::const_iterator it = block.attributes.begin();
	     it != block.attributes.end(); ++it)
	{
		mStream << escapeString(it->first) << '=' << escapeString(it->second)
		        << '\n';
	}

	mStream << escapeString(block.text) << '\n';
	mStream << mDelim << '\n';
}
