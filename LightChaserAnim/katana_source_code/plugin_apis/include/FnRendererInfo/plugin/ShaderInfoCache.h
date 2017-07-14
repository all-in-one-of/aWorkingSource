// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDERERINFO_SHADERINFOCACHE_H
#define FNRENDERERINFO_SHADERINFOCACHE_H

#include <FnAttribute/FnAttribute.h>

#include <map>

#include <iostream>

namespace Foundry
{
namespace Katana
{
namespace RendererInfo
{
    /**
     * \ingroup RenderAPI
     */

    /**
     * @brief Caches a map of shader names and attributes for quick
     *        access by Renderer Info Plug-ins.
     */
    template<typename TShaderInfoType>
    class ShaderInfoCache
    {
    public:

        /**
         *
         */
        typedef typename std::map<std::string, TShaderInfoType>::const_iterator Iterator;

        /**
         * Queries whether the cache is empty.
         *
         * @return True if empty, false otherwise.
         */
        bool isEmpty() const;

        /**
         * Flush the cache, removing all data from it.
         */
        void flush();

        /**
         * Adds information about a single shader to the cache.
         *
         * @param shaderName A string representing the unique name of the shader.
         * @param shaderInfo A group attribute holding information pertaining to the shader.
         */
        void addShaderInfo(const std::string& shaderName, const TShaderInfoType& shaderInfo);

        /**
         * Allows access to information about a shader.
         * @param  shaderName Unique name of the shader to retreive information for.
         * @return            If information about the named shader was previously cached,
         *                    that info is returned.  Otherwise, an empty Group Attribute.
         */
        const TShaderInfoType& getShaderInfo(const std::string& shaderName) const;

        /**
         * Gets a const iterator to the first Shader's information stored in the cache.
         * @return Const iterator to first Shader's information cache entry.
         */
        Iterator begin() const;

        /**
         * Gets a const iterator to the end of the cache.
         * @return Const iterator to the end of the cache.
         */
        Iterator end() const;

        /**
         * Sets the null value, which is returned if a cache item is empty or unavailable.
         * @param nullValue The null value.
         */
        void setNullValue(const TShaderInfoType& nullValue);

    private:

        typedef std::map<std::string, TShaderInfoType> ShaderInfoMap;

        TShaderInfoType _nullValue;

        ShaderInfoMap _shaderInfo;
    };
    /**
     * @}
     */

template<typename TShaderInfoType>
bool ShaderInfoCache<TShaderInfoType>::isEmpty() const
{
    return _shaderInfo.empty();
}

template<typename TShaderInfoType>
void ShaderInfoCache<TShaderInfoType>::flush()
{
    _shaderInfo.clear();
}

template<typename TShaderInfoType>
void ShaderInfoCache<TShaderInfoType>::addShaderInfo(const std::string& shaderName,
                                                     const TShaderInfoType& shaderInfo)
{
    if (!shaderName.empty())
    {
        _shaderInfo[shaderName] = shaderInfo;
    }
}

template<typename TShaderInfoType>
const TShaderInfoType& ShaderInfoCache<TShaderInfoType>::getShaderInfo(const std::string& shaderName) const
{
    // Get the shader's TShaderInfoType inside _shaderInfo
    typename ShaderInfoMap::const_iterator iter=_shaderInfo.find(shaderName);
    if (iter == _shaderInfo.end())
    {
        return _nullValue;
    }

    return (*iter).second;
}

template<typename TShaderInfoType>
typename ShaderInfoCache<TShaderInfoType>::Iterator ShaderInfoCache<TShaderInfoType>::begin() const
{
    return _shaderInfo.begin();
}

template<typename TShaderInfoType>
typename ShaderInfoCache<TShaderInfoType>::Iterator ShaderInfoCache<TShaderInfoType>::end() const
{
    return _shaderInfo.end();
}

template<typename TShaderInfoType>
void ShaderInfoCache<TShaderInfoType>::setNullValue(const TShaderInfoType& nullValue)
{
    _nullValue = nullValue;
}

}
}
}

namespace FnKat = Foundry::Katana;

#endif
