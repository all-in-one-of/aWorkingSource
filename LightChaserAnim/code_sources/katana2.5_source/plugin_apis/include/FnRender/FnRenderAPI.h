// Copyright (c) 2016 The Foundry Visionmongers Ltd.
#ifndef KATANA_FNRENDER_FNRENDERAPI_H_
#define KATANA_FNRENDER_FNRENDERAPI_H_

#if defined(_WIN32)
#  if defined(FNRENDER_APIEXPORT)
#    define FNRENDER_API __declspec(dllexport)
#  elif !defined(FNRENDER_STATIC)
#    define FNRENDER_API __declspec(dllimport)
#  else
#    define FNRENDER_API
#  endif  // FNRENDER_APIEXPORT

#elif defined(FNRENDER_APIEXPORT)
#  define FNRENDER_API __attribute__((visibility("default")))
#else
#  define FNRENDER_API
#endif  // _WIN32

#endif  // KATANA_FNRENDER_FNRENDERAPI_H_
