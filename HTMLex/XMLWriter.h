#ifndef __XML_WRITER_H__
#define __XML_WRITER_H__

#include "Writer.h"

#include <string>


class XMLWriter : public Writer {
public:
	XMLWriter(std::ostream &stream);
	virtual ~XMLWriter();

	void writeBlock(const HTMLBlock &block);

	static Writer *create(std::ostream &stream)
	{
		return new XMLWriter(stream);
	}
	
private:
	static const char mHeader[];

	std::string escapeString(const std::string &str) const;
};

#endif  // __XML_WRITER_H__
