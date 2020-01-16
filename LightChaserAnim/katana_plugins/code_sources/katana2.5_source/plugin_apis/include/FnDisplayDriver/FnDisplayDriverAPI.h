// Copyright (c) 2016 The Foundry Visionmongers Ltd.

#ifndef KATANA_FNDISPLAYDRIVER_FNDISPLAYDRIVERAPI_H_
#define KATANA_FNDISPLAYDRIVER_FNDISPLAYDRIVERAPI_H_

#if defined(_WIN32)
#  if defined(FNDISPLAYDRIVER_APIEXPORT)
#    define FNDISPLAYDRIVER_API __declspec(dllexport)
#  elif !defined(FNDISPLAYDRIVER_STATIC)
#    define FNDISPLAYDRIVER_API __declspec(dllimport)
#  else
#    define FNDISPLAYDRIVER_API
#  endif  // FNDISPLAYDRIVER_APIEXPORT

#elif defined(FNDISPLAYDRIVER_APIEXPORT)
#  define FNDISPLAYDRIVER_API __attribute__((visibility("default")))
#else
#  define FNDISPLAYDRIVER_API
#endif  // _WIN32

#endif  // KATANA_FNDISPLAYDRIVER_FNDISPLAYDRIVERAPI_H_
