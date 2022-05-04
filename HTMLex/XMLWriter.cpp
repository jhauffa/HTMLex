
#include "XMLWriter.h"
#include "HTMLBlock.h"


const char XMLWriter::mHeader[] =
	"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n" \
	"<!DOCTYPE Page [\n" \
	"<!ELEMENT Page (Block)*>\n" \
	"<!ELEMENT Block (Attributes, Text)>\n" \
	"<!ATTLIST Block\n" \
	"  left   CDATA   #REQUIRED\n" \
	"  top    CDATA   #REQUIRED\n" \
	"  right  CDATA   #REQUIRED\n" \
	"  bottom CDATA   #REQUIRED\n" \
	"  depth  CDATA   #REQUIRED\n" \
	">\n" \
	"<!ELEMENT Attributes (Attribute)*>\n" \
	"<!ELEMENT Attribute EMPTY>\n" \
	"<!ATTLIST Attribute\n" \
	"  name   NMTOKEN #REQUIRED\n" \
	"  value  CDATA   #REQUIRED\n" \
	">\n" \
	"<!ELEMENT Text (#PCDATA)>\n" \
	"]>\n\n";


XMLWriter::XMLWriter(std::ostream &stream) : Writer(stream)
{
	mStream << mHeader << "<Page>\n\n";
}

XMLWriter::~XMLWriter()
{
	mStream << "\n</Page>\n";
}

std::string XMLWriter::escapeString(const std::string &str) const
{
	std::string tmp;

	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
	{
		switch (*it)
		{
		case '<' :
			tmp += "&lt;";
			break;
		case '>' :
			tmp += "&gt;";
			break;
		case '&' :
			tmp += "&amp;";
			break;
		case '"' :
			tmp += "&quot;";
			break;
		case '\'' :
			tmp += "&apos;";
			break;
		default :
			tmp += *it;
		}
	}

	return tmp;
}

void XMLWriter::writeBlock(const HTMLBlock &block)
{
	mStream << "  <Block left=\"" << block.left << "\" top=\"" << block.top
	        << "\" right=\"" << block.right << "\" bottom=\"" << block.bottom
	        << "\" depth=\"" << block.depth << "\">\n";

	mStream << "    <Attributes>\n";
	for (AttributeMap::const_iterator it = block.attributes.begin();
	     it != block.attributes.end(); ++it)
	{
		mStream << "      <Attribute name=\"" << escapeString(it->first)
		        << "\" value=\"" << escapeString(it->second) << "\" />\n";
	}
	mStream << "    </Attributes>\n";

	mStream << "    <Text>" << escapeString(block.text) << "</Text>\n";
	mStream << "  </Block>\n";
}
