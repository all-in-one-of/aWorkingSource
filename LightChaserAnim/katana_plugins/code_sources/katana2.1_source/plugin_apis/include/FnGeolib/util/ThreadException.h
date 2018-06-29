// Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef INCLUDED_FNGEOLIBUTIL_THREADEXCEPTION_H
#define INCLUDED_FNGEOLIBUTIL_THREADEXCEPTION_H

#include <string>
#include <stdexcept>

FNGEOLIBUTIL_NAMESPACE_ENTER
{
	class ThreadException : public std::runtime_error
	{
	public:
		ThreadException(const std::string &message, const std::string &systemError)
		: std::runtime_error("ThreadException: '" + message + "' - system error: '" + systemError + "'")
		{}
	};

	class LockException : public ThreadException
	{
	public:
		LockException(const std::string &message, const std::string &systemError)
		: ThreadException(message, systemError)
		{}
	};
}
FNGEOLIBUTIL_NAMESPACE_EXIT

#endif
