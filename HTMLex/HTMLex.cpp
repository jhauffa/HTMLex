
#include "HTMLAnalyzer.h"
#include "HTMLElementAnalyzer.h"
#include "WriterFactory.h"

#define COM_NO_WINDOWS_H
#include <objbase.h>
#include <fcntl.h>  // _O_BINARY
#include <io.h>  // _setmode
#include <iostream>
#include <string>
#include <sstream>
#include <list>


static const WriterFactory factory;

static const char versionStr[] = 
	"HTMLex 1.1\n\n";

static const char optionsHelp[] =
	"possible options are:\n" \
	"  -r XxY     renders the web site at the specified resolution in pixels\n"\
	"             default: 1024x768\n" \
	"  -a attr    in addition to position and textual content, output the\n" \
	"             specified attributes for each block (multiple attributes\n" \
	"             are separated by a comma (',')\n" \
	"             example: -a fontFamily,color\n" \
    "  -f format  output format\n" \
	"             default: XML\n";


static void printUsage(const char *programName)
{
	std::cerr << versionStr;
	std::cerr << "Usage:\n";
	std::cerr << programName << " [options] URL\n\n";
	std::cerr << optionsHelp;
	std::cerr << "attributes:" << "\n\t"
	          << HTMLElementAnalyzer::getAttributeNames() << '\n';
	std::cerr << "output formats:" << "\n\t"
		      << WriterFactory::getNames() << '\n';
}

static void parseResolution(const char *str, int &resX, int &resY)
{
	std::istringstream s(str);
	s >> resX;
	s.ignore(1);  // skip the 'x'
	s >> resY;
}

static void parseAttributeList(const char *str, std::list<std::string> &attr)
{
	std::string tmp = str;
	std::string::size_type start = 0, end = tmp.find(',');
	
	while (end != std::string::npos)
	{
		attr.push_back(tmp.substr(start, end - start));
		start = end + 1;
		end = tmp.find(',', start);
	}
	
	attr.push_back(tmp.substr(start));
}

static bool parseOptions(int argCount, char *args[], int &resX, int &resY,
                         std::string &outputFormat,
						 std::list<std::string> &attr, std::string &url)
{
	int i = 1;
	while (i < argCount)
	{
		if (args[i][0] == '-')
		{
			switch(args[i][1])
			{
			case 'r' :
				i++;
				parseResolution(args[i], resX, resY);
				break;
			case 'a' :
				i++;
				parseAttributeList(args[i], attr);
				break;
			case 'f' :
				i++;
				outputFormat = args[i];
				break;
			default:
				return false;
			}
		}
		else
			url = args[i];
		i++;
	}

	return !url.empty();
}

int main(int argc, char *argv[])
{
	int resX = 1024, resY = 768;
	std::string outputFormat = "XML", url;
	std::list<std::string> attr;

	if (!parseOptions(argc, argv, resX, resY, outputFormat, attr, url))
	{
		printUsage(argv[0]);
		return 1;
	}

	CoInitialize(NULL);

	try
	{
		HTMLAnalyzer analyzer(resX, resY);

		// set binary mode for stdout/cout; disables conversion of line end characters,
		// which would corrupt WMF data
		_setmode(1, _O_BINARY);

		Writer *writer = WriterFactory::create(outputFormat, std::cout);
		analyzer.findBlocks(url, attr, *writer);
		delete writer;
	}
	catch (std::exception &ex)
	{
		std::cerr << "ERROR: " << ex.what() << std::endl;
		CoUninitialize();
		return 1;
	}

	CoUninitialize();
	return 0;
}
