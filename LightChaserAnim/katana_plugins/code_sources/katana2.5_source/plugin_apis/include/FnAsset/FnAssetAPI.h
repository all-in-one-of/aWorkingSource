// Copyright (c) 2016 The Foundry Visionmongers Ltd.
#ifndef KATANA_FNASSET_FNASSETAPI_H_
#define KATANA_FNASSET_FNASSETAPI_H_

#if defined(_WIN32)
#  if defined(FNASSET_APIEXPORT)
#    define FNASSET_API __declspec(dllexport)
#  elif !defined(FNASSET_STATIC)
#    define FNASSET_API __declspec(dllimport)
#  else
#    define FNASSET_API
#  endif  // FNASSET_APIEXPORT

#elif defined(FNASSET_APIEXPORT)
#  define FNASSET_API __attribute__((visibility("default")))
#else
#  define FNASSET_API
#endif  // _WIN32

#endif  // KATANA_FNASSET_FNASSETAPI_H_
