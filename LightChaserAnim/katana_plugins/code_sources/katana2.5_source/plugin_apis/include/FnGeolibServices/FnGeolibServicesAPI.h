// Copyright (c) 2016 The Foundry Visionmongers Ltd.
#ifndef KATANA_FNGEOLIBSERVICES_FNGEOLIBSERVICESAPI_H_
#define KATANA_FNGEOLIBSERVICES_FNGEOLIBSERVICESAPI_H_

#if defined(_WIN32)
#  if defined(FNGEOLIBSERVICES_APIEXPORT)
#    define FNGEOLIBSERVICES_API __declspec(dllexport)
#  elif !defined(FNGEOLIBSERVICES_STATIC)
#    define FNGEOLIBSERVICES_API __declspec(dllimport)
#  else
#    define FNGEOLIBSERVICES_API
#  endif  // FNGEOLIBSERVICES_APIEXPORT

#elif defined(FNGEOLIBSERVICES_APIEXPORT)
#  define FNGEOLIBSERVICES_API __attribute__((visibility("default")))
#else
#  define FNGEOLIBSERVICES_API
#endif  // _WIN32

#endif  // KATANA_FNGEOLIBSERVICES_FNGEOLIBSERVICESAPI_H_
