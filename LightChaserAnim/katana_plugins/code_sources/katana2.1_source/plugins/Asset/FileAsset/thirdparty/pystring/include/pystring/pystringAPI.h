// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef pystringAPI_h__
#define pystringAPI_h__
#ifdef _WIN32
  #if defined(pystringAPIEXPORT)
    #define pystringAPI __declspec(dllexport)
  #else
    #define pystringAPI __declspec(dllimport)
  #endif
#else
  #if defined(pystringAPIEXPORT)
    #define pystringAPI __attribute__ ((visibility("default")))
  #else
    #define pystringAPI
  #endif
#endif
#endif
