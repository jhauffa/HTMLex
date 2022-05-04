#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <exception>
#include <string>
#include <sstream>


class Exception : public std::exception {
public:
	Exception() : std::exception() {}
	Exception(const std::string &msg) : std::exception(), mMsg(msg) {}

	const char *what() const
	{
		return mMsg.c_str();
	}

protected:
	std::string mMsg;
};

class COMException : public Exception {
public:
	COMException(const std::string &msg) : Exception(msg) {}

	COMException(const std::string &msg, int rc) : Exception()
	{
		std::ostringstream s;
		s << msg << " (" << std::showbase << std::hex << rc << ")";
		mMsg = s.str();
	}
};


#endif  // __EXCEPTION_H__
