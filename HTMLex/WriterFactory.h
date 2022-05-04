#ifndef __WRITER_FACTORY_H__
#define __WRITER_FACTORY_H__

#include <string>
#include <ostream>


class Writer;

class WriterFactory {
public:
	static Writer *create(const std::string &type, std::ostream &stream);
	static std::string getNames();

private:
	typedef Writer *(*WriterConstructorFunc)(std::ostream &);
	struct WriterInfo {
		const char *name;
		WriterConstructorFunc c;
	};
	
	static WriterInfo mWriters[];
};

#endif  // __WRITER_FACTORY_H__
