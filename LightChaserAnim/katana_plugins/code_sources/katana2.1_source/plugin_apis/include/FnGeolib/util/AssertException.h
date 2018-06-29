// Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.
#ifndef INCLUDED_FNGEOLIBUTIL_ASSERTEXCEPTION_H
#define INCLUDED_FNGEOLIBUTIL_ASSERTEXCEPTION_H

#include "ns.h"

#include <exception>

FNGEOLIBUTIL_NAMESPACE_ENTER
{

//! A class to represent an assertion error.
/*! Thrown when an assertion fails.   */
class AssertException : public std::exception
{
public:
    //! Constructor
    /*!
      @param msg Pointer to a character string litteral describing the error
     */
    AssertException(const char* msg) throw() : msg_(msg) {}

    //! Get the error string.
    /*!
      @return Pointer to a character string.*/
    virtual const char* what() const throw() { return msg_; }

private:
    //! An error message formatted by the code below
    const char* msg_;
};

// macro magic to ensure assert message is always constant
#define FNGEOLIBUTIL_STRINGIZE_DETAIL(x)           #x
#define FNGEOLIBUTIL_STRINGIZE(x)                  FNGEOLIBUTIL_STRINGIZE_DETAIL(x)
#define FNGEOLIBUTIL_EXCEPTION_ASSERT_STRING(msg)  ("assertion(" msg ") failed at " __FILE__ ": " FNGEOLIBUTIL_STRINGIZE(__LINE__))

#define exception_assert(x) do { if (!(x)) throw FNGEOLIBUTIL_NAMESPACE::AssertException(FNGEOLIBUTIL_EXCEPTION_ASSERT_STRING(#x)); } while (0)
//#define exception_assert(x) (void)sizeof((x)) // use this version to disable without introducing warnings

}
FNGEOLIBUTIL_NAMESPACE_EXIT

#endif // INCLUDED_FNGEOLIBUTIL_ASSERTEXCEPTION_H

