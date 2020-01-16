// Copyright (c) 2016 The Foundry Visionmongers Ltd.
#ifndef KATANA_FNLOGGING_FNLOGGINGAPI_H_
#define KATANA_FNLOGGING_FNLOGGINGAPI_H_

#if defined(_WIN32)
#  if defined(FNLOGGING_APIEXPORT)
#    define FNLOGGING_API __declspec(dllexport)
#  elif !defined(FNLOGGING_STATIC)
#    define FNLOGGING_API __declspec(dllimport)
#  else
#    define FNLOGGING_API
#  endif  // FNLOGGING_APIEXPORT

#elif defined(FNLOGGING_APIEXPORT)
#  define FNLOGGING_API __attribute__((visibility("default")))
#else
#  define FNLOGGING_API
#endif  // _WIN32

#endif  // KATANA_FNLOGGING_FNLOGGINGAPI_H_
