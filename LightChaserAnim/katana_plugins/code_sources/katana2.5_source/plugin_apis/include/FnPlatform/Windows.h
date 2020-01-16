#ifndef KATANA_PLUGINAPI_FNPLATFORM_WINDOWS_H_
#define KATANA_PLUGINAPI_FNPLATFORM_WINDOWS_H_

#ifndef _WIN32
#error "FnPlatform/Windows.h should only be included on Windows"
#endif

#ifndef _WINDOWS_
// Windows.h hasn't yet been included.

#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x06010000  // NTDDI_WIN7
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601       // _WIN32_WINNT_WIN7
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#endif  // _WINDOWS_

#if !defined(NTDDI_VERSION) || NTDDI_VERSION < 0x06010000
#error "Windows.h has been #included, but NTDDI_VERSION indicates pre-Windows 7"
// If you trigger this case, you may be overriding _Win32_WINNT or
// NTDDI_VERSION to unsupported values.  Katana requires Windows 7 or later.
//
// See: <https://msdn.microsoft.com/en-us/library/windows/desktop/aa383745(v=vs.85).aspx>
#endif

#endif  // KATANA_PLUGINAPI_FNPLATFORM_WINDOWS_H_
