// Copyright (c) 2016 The Foundry Visionmongers Ltd.
#ifndef KATANA_FNATTRIBUTE_FNATTRIBUTEAPI_H_
#define KATANA_FNATTRIBUTE_FNATTRIBUTEAPI_H_

#if defined(_WIN32)
#  if defined(FNATTRIBUTE_APIEXPORT)
#    define FNATTRIBUTE_API __declspec(dllexport)
#  elif !defined(FNATTRIBUTE_STATIC)
#    define FNATTRIBUTE_API __declspec(dllimport)
#  else
#    define FNATTRIBUTE_API
#  endif  // FNATTRIBUTE_APIEXPORT

#elif defined(FNATTRIBUTE_APIEXPORT)
#  define FNATTRIBUTE_API __attribute__((visibility("default")))
#else
#  define FNATTRIBUTE_API
#endif  // _WIN32

#endif  // KATANA_FNATTRIBUTE_FNATTRIBUTEAPI_H_
