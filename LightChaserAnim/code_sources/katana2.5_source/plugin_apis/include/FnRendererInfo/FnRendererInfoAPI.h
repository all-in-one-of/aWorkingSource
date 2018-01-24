// Copyright (c) 2016 The Foundry Visionmongers Ltd.
#ifndef KATANA_FNRENDERERINFO_FNRENDERERINFOAPI_H_
#define KATANA_FNRENDERERINFO_FNRENDERERINFOAPI_H_

#if defined(_WIN32)
#  if defined(FNRENDERERINFO_APIEXPORT)
#    define FNRENDERERINFO_API __declspec(dllexport)
#  elif !defined(FNRENDERERINFO_STATIC)
#    define FNRENDERERINFO_API __declspec(dllimport)
#  else
#    define FNRENDERERINFO_API
#  endif  // FNRENDERERINFO_APIEXPORT

#elif defined(FNRENDERERINFO_APIEXPORT)
#  define FNRENDERERINFO_API __attribute__((visibility("default")))
#else
#  define FNRENDERERINFO_API
#endif  // _WIN32

#endif  // KATANA_FNRENDERERINFO_FNRENDERERINFOAPI_H_
