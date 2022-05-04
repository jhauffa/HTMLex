#ifndef __TEXT_WRITER_H__
#define __TEXT_WRITER_H__

#include "Writer.h"

#include <string>


class TextWriter : public Writer {
public:
	TextWriter(std::ostream &stream) : Writer(stream) {}

	void writeBlock(const HTMLBlock &block);

	static Writer *create(std::ostream &stream)
	{
		return new TextWriter(stream);
	}

private:
	static const char mDelim[];
	
	std::string escapeString(const std::string &str) const;
};

#endif  // __TEXT_WRITER_H__
