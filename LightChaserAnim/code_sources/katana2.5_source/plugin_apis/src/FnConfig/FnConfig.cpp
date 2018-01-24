#include <cassert>
#include <iostream>

#include <FnConfig/FnConfig.h>

FNCONFIG_NAMESPACE_ENTER
{

Config::Config() {}
Config::~Config() {}

bool Config::has(const std::string& key)
{
    return getSuite()->has(key.c_str());
}

const std::string Config::get(const std::string& key)
{
    const char * value = getSuite()->get(key.c_str());
    return ((value == NULL) ? std::string() : std::string(value));
}

const FnConfigHostSuite_v1* Config::getSuite()
{
    assert(_configSuite);
    return _configSuite;
}

FnPluginStatus Config::setHost(FnPluginHost* host)
{
    if (host)
    {
        _configSuite = (FnConfigHostSuite_v1*)(host->getSuite("Config", 1));
        if (_configSuite)
        {
            return FnPluginStatusOK;
        }
    }
    return FnPluginStatusError;
}

const FnConfigHostSuite_v1* Config::_configSuite = 0x0;

}
FNCONFIG_NAMESPACE_EXIT

