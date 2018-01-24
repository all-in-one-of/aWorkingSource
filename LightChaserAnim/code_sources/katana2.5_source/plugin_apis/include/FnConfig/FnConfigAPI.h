// Copyright (c) 2016 The Foundry Visionmongers Ltd.
#ifndef KATANA_FNCONFIG_FNCONFIGAPI_H_
#define KATANA_FNCONFIG_FNCONFIGAPI_H_

#if defined(_WIN32)
#  if defined(FNCONFIG_APIEXPORT)
#    define FNCONFIG_API __declspec(dllexport)
#  elif !defined(FNCONFIG_STATIC)
#    define FNCONFIG_API __declspec(dllimport)
#  else
#    define FNCONFIG_API
#  endif  // FNCONFIG_APIEXPORT

#elif defined(FNCONFIG_APIEXPORT)
#  define FNCONFIG_API __attribute__((visibility("default")))
#else
#  define FNCONFIG_API
#endif  // _WIN32

#endif  // KATANA_FNCONFIG_FNCONFIGAPI_H_
