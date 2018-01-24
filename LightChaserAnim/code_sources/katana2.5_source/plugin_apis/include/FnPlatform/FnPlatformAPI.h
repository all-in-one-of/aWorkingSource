// Copyright (c) 2016 The Foundry Visionmongers Ltd.
#ifndef KATANA_FNPLATFORM_FNPLATFORMAPI_H_
#define KATANA_FNPLATFORM_FNPLATFORMAPI_H_

#if defined(_WIN32)
#  if defined(FNPLATFORM_APIEXPORT)
#    define FNPLATFORM_API __declspec(dllexport)
#  elif !defined(FNPLATFORM_STATIC)
#    define FNPLATFORM_API __declspec(dllimport)
#  else
#    define FNPLATFORM_API
#  endif  // FNPLATFORM_APIEXPORT

#elif defined(FNPLATFORM_APIEXPORT)
#  define FNPLATFORM_API __attribute__((visibility("default")))
#else
#  define FNPLATFORM_API
#endif  // _WIN32

#endif  // KATANA_FNPLATFORM_FNPLATFORMAPI_H_
