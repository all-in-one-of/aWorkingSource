// Copyright (c) 2016 The Foundry Visionmongers Ltd.
#ifndef KATANA_FNPLUGINMANAGER_FNPLUGINMANAGERAPI_H_
#define KATANA_FNPLUGINMANAGER_FNPLUGINMANAGERAPI_H_

#if defined(_WIN32)
#  if defined(FNPLUGINMANAGER_APIEXPORT)
#    define FNPLUGINMANAGER_API __declspec(dllexport)
#  elif !defined(FNPLUGINMANAGER_STATIC)
#    define FNPLUGINMANAGER_API __declspec(dllimport)
#  else
#    define FNPLUGINMANAGER_API
#  endif  // FNPLUGINMANAGER_APIEXPORT

#elif defined(FNPLUGINMANAGER_APIEXPORT)
#  define FNPLUGINMANAGER_API __attribute__((visibility("default")))
#else
#  define FNPLUGINMANAGER_API
#endif  // _WIN32

#endif  // KATANA_FNPLUGINMANAGER_FNPLUGINMANAGERAPI_H_
