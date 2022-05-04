
#include "WriterFactory.h"
#include "Exception.h"
#include "TextWriter.h"
#include "WMFWriter.h"
#include "XMLWriter.h"


WriterFactory::WriterInfo WriterFactory::mWriters[] = {
	{ "text", TextWriter::create },
	{ "WMF",  WMFWriter::create  },
	{ "XML",  XMLWriter::create  }
};


Writer *WriterFactory::create(const std::string &type,
                              std::ostream &stream)
{
	int numWriters = sizeof(mWriters) / sizeof(WriterInfo);
	for (int i = 0; i < numWriters; i++)
		if (mWriters[i].name == type)
			return (mWriters[i].c)(stream);
	throw Exception("unknown output format");
}

std::string WriterFactory::getNames()
{
	int numWriters = sizeof(mWriters) / sizeof(WriterInfo);
	std::string tmp = mWriters[0].name;
	for (int i = 1; i < numWriters; i++)
	{
		tmp += ", ";
		tmp += mWriters[i].name;
	}
	return tmp;
}
