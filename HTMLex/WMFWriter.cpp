
#include "WMFWriter.h"
#include "HTMLBlock.h"
#include "Exception.h"


WMFWriter::WMFWriter(std::ostream &stream) : Writer(stream)
{
	mMetaDC = CreateEnhMetaFileA(NULL, NULL, NULL, NULL);
	if (!mMetaDC)
		throw Exception("error creating metafile");

	mPen = CreatePen(PS_INSIDEFRAME, 2, 0);
	SelectObject(mMetaDC, mPen);
//	mBrush = CreateSolidBrush(0xffffff);
	mBrush = static_cast<HBRUSH>(GetStockObject(HOLLOW_BRUSH));
	SelectObject(mMetaDC, mBrush);
}

WMFWriter::~WMFWriter()
{
	HENHMETAFILE metaFile = CloseEnhMetaFile(mMetaDC);
	DeleteObject(mPen);
	DeleteObject(mBrush);

	// write metafile to stream
	UINT bufSize = GetEnhMetaFileBits(metaFile, 0, NULL);
	LPBYTE buffer = new BYTE[bufSize];
	if (GetEnhMetaFileBits(metaFile, bufSize, buffer) == 0)
	{
		delete[] buffer;
		DeleteEnhMetaFile(metaFile);
		throw Exception("could not access meta file data");
	}
	mStream.write(reinterpret_cast<const char *>(buffer), bufSize);
	delete[] buffer;

	DeleteEnhMetaFile(metaFile);
}

void WMFWriter::writeBlock(const HTMLBlock &block)
{
	Rectangle(mMetaDC, block.left, block.top, block.right, block.bottom);
}
