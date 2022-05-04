#ifndef __WRITER_H__
#define __WRITER_H__

#include "Exception.h"

#include <ostream>


struct HTMLBlock;

class Writer {
public:
	Writer(std::ostream &stream) : mStream(stream)
	{
		if (!mStream)
			throw Exception("I/O error");
	}
	
	virtual ~Writer()
	{
		mStream.flush();
	}
	
	virtual void writeBlock(const HTMLBlock &block) = 0;

	static Writer *create(std::ostream &)
	{
		return NULL;
	}
	
protected:
	std::ostream &mStream;
};

#endif  // __WRITER_H__
