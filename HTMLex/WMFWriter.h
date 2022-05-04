#ifndef __WMF_WRITER_H__
#define __WMF_WRITER_H__

#include "Writer.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


class WMFWriter : public Writer {
public:
	WMFWriter(std::ostream &stream);
	virtual ~WMFWriter();

	void writeBlock(const HTMLBlock &block);

	static Writer *create(std::ostream &stream)
	{
		return new WMFWriter(stream);
	}
	
private:
	HDC mMetaDC;
	HPEN mPen;
	HBRUSH mBrush;
};

#endif  // __WMF_WRITER_H__
