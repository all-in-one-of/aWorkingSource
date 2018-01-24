// Copyright (c) 2016 The Foundry Visionmongers Ltd.
#ifndef KATANA_PYSTRING_PYSTRINGAPI_H_
#define KATANA_PYSTRING_PYSTRINGAPI_H_

#if defined(_WIN32)
#  if defined(PYSTRING_APIEXPORT)
#    define PYSTRING_API __declspec(dllexport)
#  elif !defined(PYSTRING_STATIC)
#    define PYSTRING_API __declspec(dllimport)
#  else
#    define PYSTRING_API
#  endif  // PYSTRING_APIEXPORT

#elif defined(PYSTRING_APIEXPORT)
#  define PYSTRING_API __attribute__((visibility("default")))
#else
#  define PYSTRING_API
#endif  // _WIN32

#endif  // KATANA_PYSTRING_PYSTRINGAPI_H_
