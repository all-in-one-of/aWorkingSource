#ifndef _PLUGINMANAGERAPI_H_
#define _PLUGINMANAGERAPI_H_

#if defined(FnPluginManagerAPIEXPORT)
#define FnPluginManagerAPI __attribute__((visibility("default")))
#else
#define FnPluginManagerAPI
#endif

#endif  // _PLUGINMANAGERAPI_H_
