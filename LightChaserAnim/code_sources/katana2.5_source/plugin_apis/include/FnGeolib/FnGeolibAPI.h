// Copyright (c) 2016 The Foundry Visionmongers Ltd.
#ifndef KATANA_FNGEOLIB_FNGEOLIBAPI_H_
#define KATANA_FNGEOLIB_FNGEOLIBAPI_H_

#if defined(_WIN32)
#  if defined(FNGEOLIB_APIEXPORT)
#    define FNGEOLIB_API __declspec(dllexport)
#  elif !defined(FNGEOLIB_STATIC)
#    define FNGEOLIB_API __declspec(dllimport)
#  else
#    define FNGEOLIB_API
#  endif  // FNGEOLIB_APIEXPORT

#elif defined(FNGEOLIB_APIEXPORT)
#  define FNGEOLIB_API __attribute__((visibility("default")))
#else
#  define FNGEOLIB_API
#endif  // _WIN32

#endif  // KATANA_FNGEOLIB_FNGEOLIBAPI_H_
