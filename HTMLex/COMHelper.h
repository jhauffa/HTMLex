#ifndef __COM_OBJECT_H__
#define __COM_OBJECT_H__

#include "Exception.h"

#include <wtypes.h>  // BSTR
#include <string>


template<typename T> class COMObject {
public:
	COMObject() : mObject(NULL) {}
	COMObject(T *object) : mObject(object) {}

	COMObject(const COMObject<T> &other) : mObject(NULL)
	{
		operator=(other);
	}

	~COMObject()
	{
		clear();
	}

	T *operator*()
	{
		return mObject;
	}
	
	T **operator&()
	{
		return &mObject;
	}

	T *operator->()
	{
		if (mObject)
			return mObject;
		else
			throw Exception("trying to dereference NULL object");
	}
	
	bool operator!() const
	{
		return (mObject == NULL);
	}
	
	COMObject<T> &operator=(const COMObject<T> &other)
	{
		clear();
		mObject = other.mObject;
		if (mObject)
			mObject->AddRef();
		return *this;
	}

private:
	T *mObject;
	
	void clear()
	{
		if (mObject)
		{
			mObject->Release();
			mObject = NULL;
		}
	}
};


class BString {
public:
	static void convertToUtf8(const BSTR srcStr, std::string &destStr);
	static BSTR createFromAnsi(const char *str);
	static bool isEmptyAnsiString(const char *str);
};


#endif
