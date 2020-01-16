// Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.
#ifndef INCLUDED_FNGEOLIBUTIL_THREADEXCEPTION_H
#define INCLUDED_FNGEOLIBUTIL_THREADEXCEPTION_H
#include <string>
#include <stdexcept>

#include <FnGeolib/FnGeolibAPI.h>
#include <FnGeolib/util/ns.h>

#include <FnPlatform/internal/Portability.h>

FNGEOLIBUTIL_NAMESPACE_ENTER
{
	class FNGEOLIB_API ThreadException : public std::runtime_error
	{
	public:
		FNKAT_DEPRECATED ThreadException(const std::string &message, const std::string &systemError)
		: std::runtime_error("ThreadException: '" + message + "' - system error: '" + systemError + "'")
		{}
	};

	class FNGEOLIB_API LockException : public ThreadException
	{
	public:
		FNKAT_DEPRECATED LockException(const std::string &message, const std::string &systemError)
		: ThreadException(message, systemError)
		{}
	};
}
FNGEOLIBUTIL_NAMESPACE_EXIT

#endif
