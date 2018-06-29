// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include <ai_version.h>
#include <stdlib.h>

namespace
{
    bool g_versionsChecked = false;
    int g_arch = 0, g_major = 0, g_minor = 0, g_fix = 0;
    
    bool isAtLeast(int arch, int major, int minor, int fix)
    {
        if (!g_versionsChecked)
        {
            g_versionsChecked = true;
            char s_arch[256], s_major[256], s_minor[256], s_fix[256];
            AiGetVersion(s_arch, s_major, s_minor, s_fix);
            
            g_arch = atol(s_arch);
            g_major = atol(s_major);
            g_minor = atol(s_minor);
            g_fix = atol(s_fix);
        }
        
        if (g_arch < arch) return false;
        if (g_arch > arch) return true;
        
        if (g_major < major) return false;
        if (g_major > major) return true;
        
        if (g_minor < minor) return false;
        if (g_minor > minor) return true;
        
        if (g_fix < fix) return false;
        if (g_fix > fix) return true;
        
        
        return true;
    }
}

bool shouldHookOSLSkydomesDirectlyToBackground()
{
    return isAtLeast(3, 0, 50, 35);
}


