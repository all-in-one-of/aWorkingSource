// Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <stdexcept>
#include <vector>
//#include <arpa/inet.h>  // TODO: Remove once b85 goes away
//#include <sstream>
#include <iostream> // TODO: use better logging

#ifdef _WIN32
#include <FnPlatform/Windows.h>
#else
#include <dlfcn.h>
#endif // _WIN32

namespace {
    std::string getLastError()
    {
        std::string error;
#ifdef _WIN32
        char* messageBuffer = NULL;
        DWORD err = GetLastError();

        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                       FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL,
                       err,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       reinterpret_cast<LPSTR>(&messageBuffer),
                       0, NULL);

        error = messageBuffer ? messageBuffer : "";
        if (messageBuffer != NULL)
            LocalFree(messageBuffer);
#else
        char *lastError = dlerror();
        if (lastError != NULL)
            error = lastError;
#endif // _WIN32
        return error;
    }
}

FNATTRIBUTE_NAMESPACE_ENTER
{
    // --- Bootstrap -----------------------------------------------------------
    bool Bootstrap(const std::string& katanaPath)
    {
#ifdef _WIN32
        HINSTANCE dsoptr = NULL;

        // LOAD_WITH_ALTERED_SEARCH_PATH requires an absolute path that uses
        // backslash path separators.
        char canonicalPath[MAX_PATH];
        DWORD ret = GetFullPathNameA(
            (katanaPath + "\\bin\\FnAttributeHost.dll").c_str(),
            sizeof(canonicalPath), canonicalPath, NULL);
        if (ret < sizeof(canonicalPath))
        {
            dsoptr = LoadLibraryExA(canonicalPath, NULL,
                                    LOAD_WITH_ALTERED_SEARCH_PATH);
        }
#else
        void* dsoptr = dlopen(
            (katanaPath + "/bin/libFnAttributeHost.so").c_str(), RTLD_NOW);
#endif  // _WIN32

        if (!dsoptr)
        {
            std::cerr << "Error loading FnAttributeHost: " << getLastError() << std::endl;
            return false;
        }

        // Get the getAttributeHost() function from FnAttributeHost
#ifdef _WIN32
        void *funcptr = GetProcAddress(dsoptr, "getAttributeHost");
#else
        void *funcptr = dlsym(dsoptr, "getAttributeHost");
#endif // _WIN32

        if (!funcptr)
        {
            std::cerr << "Error loading ATTRUTIL: " << getLastError() << std::endl;
#ifdef _WIN32
            FreeLibrary(dsoptr);
#else
            dlclose(dsoptr);
#endif // _WIN32
            return false;
        }

        typedef FnPluginHost* (*GetAttributeHostFunc)(void);
        GetAttributeHostFunc getAttributeHostFunc = (GetAttributeHostFunc)funcptr;

        // Get the host.
        FnPluginHost *host = getAttributeHostFunc();

        // Inject the host (and consequently the suites) into the C++ plugin
        // wrappers.
        Attribute::setHost(host);
        GroupBuilder::setHost(host);

        return true;
    }

    void Initialize(const FnAttributeHostSuite_v2 *attrHostSuite)
    {
        Attribute::setSuite(attrHostSuite);
        GroupBuilder::setSuite(attrHostSuite);
    }

    // --- FnAttribute ---------------------------------------------------------



    namespace
    {
        int base32_encode(const uint8_t *data, int length, uint8_t *result, int bufSize);
    }

    std::string Hash::str() const
    {
        union HashUnion {
            uint64_t h64[2];
            uint8_t  h8[16];
        } u;
        u.h64[0] = hash1;
        u.h64[1] = hash2;

        // We use a base32 encoding (rather than more sophisticated b64 or b85)
        // so that the resulting character stream is safe to use for both
        // attribute names and scenegraph locations paths.  base32 only
        // uses alphanumerics, which is good.

        const int HashMaxStrLen = 26;
        uint8_t dest[HashMaxStrLen];
        int destsize = base32_encode(u.h8, 16, dest, HashMaxStrLen);
        assert(destsize >= 0);
        return std::string((char * ) dest, static_cast<size_t>(destsize));
    }

    Attribute Hash::attr() const
    {
        union HashUnion {
            uint64_t u64[2];
            int32_t i32[4];
        } u;
        u.u64[0] = hash1;
        u.u64[1] = hash2;
        return IntAttribute(&(u.i32[0]), 4, 1);
    }

    FnPlugStatus Attribute::setHost(FnPluginHost *host)
    {
        if (host)
        {
            _attrSuite = reinterpret_cast<const FnAttributeHostSuite_v2*>(
                host->getSuite("AttributeHost", 2));
            if (_attrSuite)
            {
                return FnPluginStatusOK;
            }
        }
        return FnPluginStatusError;
    }

    void Attribute::setSuite(const FnAttributeHostSuite_v2 *suite)
    {
        if (suite)
        {
            _attrSuite = suite;
        }
    }

    // --- Attr I/O ------------------------------------------------------------

namespace
{
    class CharWriteStream
    {
    public:
        CharWriteStream(std::vector<char> & buf): _buffer(buf) {}

        size_t size() const {return _buffer.size();}

        static int64_t write(void *stream, const void *buf, int64_t size)
        {
            CharWriteStream *that = reinterpret_cast<CharWriteStream *>(stream);
            const size_t oldSize = that->_buffer.size();
            that->_buffer.resize(that->_buffer.size() + size);
            memcpy(that->_buffer.data()+oldSize, buf, size);
            return size;
        }

        const char * getData()
        {
            if (_buffer.empty())
            {
                return NULL;
            }
            return &(_buffer.front());
        }
    private:
        std::vector<char> & _buffer;
    };
}

    std::string Attribute::getXML() const
    {
        std::vector<char> buf;
        CharWriteStream strm(buf);
        if (!writeAttributeStream(&strm,
            CharWriteStream::write, kFnKatStreamTypeXML))
        {
            return "";
        }

        if(buf.size()>0) return std::string(buf.data(), buf.size());
        return "";
    }

    void Attribute::getBinary(std::vector<char> * buf) const
    {
        if(!buf) return;
        buf->clear();

        CharWriteStream strm(*buf);
        if (!writeAttributeStream(&strm,
            CharWriteStream::write, kFnKatStreamTypeBinary))
        {
            buf->clear();
        }
    }

namespace
{
    class CharReadStream
    {
    public:
        CharReadStream(const char * buffer, size_t bufferSize) : _buffer(buffer),
            _bufferSize(bufferSize), _readPos(0)
        {
        }

        void resetReadPos() {_readPos = 0;}
        size_t size() const {return _bufferSize;}

        static int64_t read(void *stream, void *buf, int64_t size)
        {
            CharReadStream *that = reinterpret_cast<CharReadStream *>(stream);

            if (that->_readPos > that->_bufferSize) return 0;

            const size_t copysize = std::min((size_t)size,
                                             that->_bufferSize-that->_readPos);

            if (copysize != 0)
            {
                memcpy(buf, that->_buffer+that->_readPos, copysize);
                that->_readPos += copysize;
            }

            return copysize;
        }

    private:
        const char * _buffer;
        size_t _bufferSize;
        size_t _readPos;
    };
}

    Attribute Attribute::parseXML(const char * str)
    {
        if(!str) return Attribute();
        CharReadStream strm(str, strlen(str));
        return readAttributeStream(&strm, CharReadStream::read,
                                   kFnKatStreamTypeXML);
    }

    Attribute Attribute::parseBinary(const char * buffer, size_t size)
    {
        if(!buffer || size==0) return Attribute();
        CharReadStream strm(buffer, size);
        return readAttributeStream(&strm, CharReadStream::read,
                                   kFnKatStreamTypeBinary);
    }

    bool Attribute::writeAttributeStream(void *stream, FnAttributeWriteStreamFunc func,
                                         FnKatStreamType streamType) const
    {
        FnAttributeHandle errorMessageHandle = 0x0;
        bool returnval = getSuite()->writeAttributeStream(_handle, stream, func,
            &errorMessageHandle, streamType) != 0;
        if(errorMessageHandle)
        {
            Attribute a(errorMessageHandle);
            StringAttribute errorattr(a);
            throw std::runtime_error(errorattr.getValue("unknown", false));
        }
        return returnval;
    }

    Attribute Attribute::readAttributeStream(void *stream, FnAttributeReadStreamFunc func,
                                             FnKatStreamType streamType)
    {
        FnAttributeHandle errorMessageHandle = 0x0;
        Attribute returnval = Attribute(
            getSuite()->readAttributeStream(stream, func, &errorMessageHandle, streamType));
        if(errorMessageHandle)
        {
            Attribute a(errorMessageHandle);
            StringAttribute errorattr(a);
            throw std::runtime_error(errorattr.getValue("unknown", false));
        }
        return returnval;
    }

    const FnAttributeHostSuite_v2 *Attribute::_attrSuite = 0x0;

    // --- FloatAttribute ------------------------------------------------------

    namespace
    {
        // when amt is 0, a is returned
        inline float lerp(float a, float b, float amt)
        {
            return (a + (b-a)*amt);
        }
    }

    void FloatAttribute::fillInterpSample(
        value_type * array, int64_t valueCount,
        float sampleTime,
        const value_type defValue, bool throwOnError) const
    {
        if(!isValid() || !array)
        {
            if(throwOnError)
                throw std::runtime_error("Error filling interpolated float value from FloatAttribute.");
            return;
        }

        float lTime = 0.0f;
        float rTime = 0.0f;
        // TODO: Consider return value?
        getBoundingSampleTimes(&lTime, &rTime, sampleTime);

        array_type attrValue_l = getNearestSample(lTime);
        array_type attrValue_r = getNearestSample(rTime);
        int64_t interp_count = std::max( (int64_t) attrValue_l.size(),
                                         (int64_t) attrValue_r.size());
        if(throwOnError && interp_count<valueCount)
        {
            throw std::runtime_error("Error interpolating FloatAttribute. Attrs do not have sufficient value counts.");
        }

        interp_count = std::max((int64_t)0, std::min(interp_count, valueCount));
        if(interp_count>0)
        {
            if(lTime == rTime)
            {
                for(int64_t i=0; i<interp_count; ++i)
                {
                    array[i] = attrValue_l[i];
                }
            }
            else
            {
                float amt = (sampleTime-lTime) / (rTime-lTime);
                for(int64_t i=0; i<interp_count; ++i)
                {
                    array[i] = lerp(attrValue_l[i], attrValue_r[i], amt);
                }
            }
        }

        for(int64_t i=interp_count; i<valueCount; ++i)
        {
            array[i] = defValue;
        }
    }

    // --- DoubleAttribute -----------------------------------------------------

    namespace
    {
        // when amt is 0, a is returned
        inline double lerp(double a, double b, double amt)
        {
            return (a + (b-a)*amt);
        }
    }

    void DoubleAttribute::fillInterpSample(
        value_type * array, int64_t valueCount,
        float sampleTime,
        const value_type defValue, bool throwOnError) const
    {
        if(!isValid() || !array)
        {
            if(throwOnError)
                throw std::runtime_error("Error filling interpolated double value from DoubleAttribute.");
            return;
        }

        float lTime = 0.0f;
        float rTime = 0.0f;
        // TODO: Consider return value?
        getBoundingSampleTimes(&lTime, &rTime, sampleTime);

        array_type attrValue_l = getNearestSample(lTime);
        array_type attrValue_r = getNearestSample(rTime);
        int64_t interp_count = std::max( (int64_t) attrValue_l.size(),
                                         (int64_t) attrValue_r.size());
        if(throwOnError && interp_count<valueCount)
        {
            throw std::runtime_error("Error interpolating DoubleAttribute. Attrs do not have sufficient value counts.");
        }

        interp_count = std::max((int64_t)0, std::min(interp_count, valueCount));
        if(interp_count>0)
        {
            if(lTime == rTime)
            {
                for(int64_t i=0; i<interp_count; ++i)
                {
                    array[i] = attrValue_l[i];
                }
            }
            else
            {
                double amt = (sampleTime-lTime) / (rTime-lTime);
                for(int64_t i=0; i<interp_count; ++i)
                {
                    array[i] = lerp(attrValue_l[i], attrValue_r[i], amt);
                }
            }
        }

        for(int64_t i=interp_count; i<valueCount; ++i)
        {
            array[i] = defValue;
        }
    }

    // --- StringAttribute -----------------------------------------------------

    StringAttribute::StringAttribute(
        const std::string * values,
        int64_t valueCount,
        int64_t tupleSize) : DataAttribute()
    {
        std::vector<const char *> cstrvec(valueCount);
        for(unsigned int i=0; i<valueCount; ++i)
        {
            cstrvec[i] = values[i].c_str();
        }

        const char ** cstrvals = NULL;
        if(!cstrvec.empty()) cstrvals = const_cast<const char **>(cstrvec.data());

        FnAttributeHandle newhandle = getSuite()->createStringAttr2(
            cstrvals, valueCount, tupleSize);
        stealHandle(newhandle);
    }

    StringAttribute::StringAttribute(
        const std::vector<std::string> & stringvec,
        int64_t tupleSize) : DataAttribute()
    {
        std::vector<const char *> cstrvec(stringvec.size());
        for(unsigned int i=0; i<stringvec.size(); ++i)
        {
            cstrvec[i] = stringvec[i].c_str();
        }

        const char ** values = NULL;
        if(!stringvec.empty()) values = const_cast<const char **>(cstrvec.data());

        FnAttributeHandle newhandle = getSuite()->createStringAttr2(
            values, stringvec.size(), tupleSize);
        stealHandle(newhandle);
    }

    // --- GroupAttribute ------------------------------------------------------

    GroupAttribute::GroupAttribute(const NamedAttrVector_Type &children,
        const bool inheritChildren) : Attribute(0x0)
    {
        std::vector<const char *> names;
        std::vector<FnAttributeHandle> handles;

        names.reserve(children.size());
        handles.reserve(children.size());

        for(size_t i=0, e=children.size(); i!=e; ++i)
        {
            FnAttributeHandle childHandle = children[i].second.getHandle();
            if(childHandle==0x0) continue; // do not add invalid children
            names.push_back(children[i].first.c_str());
            handles.push_back(childHandle);
        }

        FnAttributeHandle handle = getSuite()->createGroupAttr(
            names.data(), handles.data(),
            (int64_t)names.size(), (uint8_t)inheritChildren);
        stealHandle(handle);
    }

    void GroupAttribute::fillChildVector(NamedAttrVector_Type * children) const
    {
        children->clear();
        children->reserve(getNumberOfChildren());

        for(int64_t i = 0, e = getNumberOfChildren(); i!=e; ++i)
        {
            children->push_back(std::make_pair(getChildName(i),
                getChildByIndex(i)));
        }
    }

    // --- FnGropuBuilder ------------------------------------------------------

    FnPlugStatus GroupBuilder::setHost(FnPluginHost *host)
    {
        if (host)
        {
            _attrSuite = reinterpret_cast<const FnAttributeHostSuite_v2*>(
                host->getSuite("AttributeHost", 2));
            if (_attrSuite)
            {
                return FnPluginStatusOK;
            }
        }
        return FnPluginStatusError;
    }

    void GroupBuilder::setSuite(const FnAttributeHostSuite_v2 *suite)
    {
        if (suite)
        {
            _attrSuite = suite;
        }
    }

    const FnAttributeHostSuite_v2 *GroupBuilder::_attrSuite = 0x0;

    std::string DelimiterEncode(const std::string & str)
    {
        const FnAttributeHostSuite_v2* suite = Attribute::getSuite();
        if(!suite) return str;

        if(str.empty()) return str;
        uint64_t num = str.size();
        char * dst = (char *) malloc(num);
        memcpy(dst, str.c_str(), num);
        suite->delimiterEncode(dst, num);
        std::string ret(dst, num);
        free(dst);
        return ret;
    }


    std::string DelimiterDecode(const std::string & str)
    {
        const FnAttributeHostSuite_v2* suite = Attribute::getSuite();
        if(!suite) return str;

        if(str.empty()) return str;
        uint64_t num = str.size();
        char * dst = (char *) malloc(num);
        memcpy(dst, str.c_str(), num);
        suite->delimiterDecode(dst, num);
        std::string ret(dst, num);
        free(dst);
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////


namespace
{

// Base32 implementation
//
// Copyright 2010 Google Inc.
// Author: Markus Gutschke
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

int base32_encode(const uint8_t *data, int length, uint8_t *result,
                  int bufSize)
{
    if (length < 0 || length > (1 << 28))
    {
        return -1;
    }

    int count = 0;
    if (length > 0)
    {
        int buffer = data[0];
        int next = 1;
        int bitsLeft = 8;

        while (count < bufSize && (bitsLeft > 0 || next < length))
        {
            if (bitsLeft < 5)
            {
                if (next < length)
                {
                    buffer <<= 8;
                    buffer |= data[next++] & 0xFF;
                    bitsLeft += 8;
                }
                else
                {
                    int pad = 5 - bitsLeft;
                    buffer <<= pad;
                    bitsLeft += pad;
                }
            }

            int index = 0x1F & (buffer >> (bitsLeft - 5));
            bitsLeft -= 5;

            // Katana: This array has been modified to use lower case instead.
            result[count++] = "abcdefhgijklmnopqrstuvwxyz234567"[index];
        }
    }

    if (count < bufSize)
    {
        result[count] = '\000';
    }
    return count;
}

} // anon-namespace

}
FNATTRIBUTE_NAMESPACE_EXIT
