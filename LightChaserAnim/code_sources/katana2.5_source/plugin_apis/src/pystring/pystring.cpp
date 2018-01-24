///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008-2010, Sony Pictures Imageworks Inc
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// Neither the name of the organization Sony Pictures Imageworks nor the
// names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER
// OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///////////////////////////////////////////////////////////////////////////////

#include "pystring/pystring.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include "FnPlatform/Windows.h"
#include <Shlobj.h>  // SHGetKnownFolderPath
#include <Sddl.h>    // ConvertSidToStringSid
#else
#include <pwd.h>
#include <unistd.h>
#endif // _WIN32

GEOLIB3_PYSTRING_NAMESPACE_ENTER
{


// This definition codes from configure.in in the python src.
// Strictly speaking this limits us to str sizes of 2**31.
// Should we wish to handle this limit, we could use an architecture
// specific #defines and read from ssize_t (unistd.h) if the header exists.
// But in the meantime, the use of int assures maximum arch compatibility.
// This must also equal the size used in the end = MAX_32BIT_INT default arg.

typedef int Py_ssize_t;

/* helper macro to fixup start/end slice values */
#define ADJUST_INDICES(start, end, len) \
    if (end > len)                      \
        end = len;                      \
    else if (end < 0)                   \
    {                                   \
        end += len;                     \
        if (end < 0)                    \
            end = 0;                    \
    }                                   \
    if (start < 0)                      \
    {                                   \
        start += len;                   \
        if (start < 0)                  \
            start = 0;                  \
    }

namespace
{

//////////////////////////////////////////////////////////////////////////////////////////////
/// why doesn't the std::reverse work?
///
void reverse_strings( std::vector< std::string > & result)
{
    for (std::vector< std::string >::size_type i = 0; i < result.size() / 2; i++ )
    {
        std::swap(result[i], result[result.size() - 1 - i]);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
void split_whitespace( const std::string & str, std::vector< std::string > & result, int maxsplit )
{
    std::string::size_type i, j, len = str.size();
    for (i = j = 0; i < len; )
    {

        while ( i < len && ::isspace( str[i] ) ) i++;
        j = i;

        while ( i < len && ! ::isspace( str[i]) ) i++;



        if (j < i)
        {
            if ( maxsplit-- <= 0 ) break;

            result.push_back( str.substr( j, i - j ));

            while ( i < len && ::isspace( str[i])) i++;
            j = i;
        }
    }
    if (j < len)
    {
        result.push_back( str.substr( j, len - j ));
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////
///
///
void rsplit_whitespace( const std::string & str, std::vector< std::string > & result, int maxsplit )
{
    std::string::size_type len = str.size();
    std::string::size_type i, j;
    for (i = j = len; i > 0; )
    {

        while ( i > 0 && ::isspace( str[i - 1] ) ) i--;
        j = i;

        while ( i > 0 && ! ::isspace( str[i - 1]) ) i--;



        if (j > i)
        {
            if ( maxsplit-- <= 0 ) break;

            result.push_back( str.substr( i, j - i ));

            while ( i > 0 && ::isspace( str[i - 1])) i--;
            j = i;
        }
    }
    if (j > 0)
    {
        result.push_back( str.substr( 0, j ));
    }
    //std::reverse( result, result.begin(), result.end() );
    reverse_strings( result );
}

// Returns true if the given character is a valid character for
// environment variable names, false otherwise. This function is used
// in pystring::os::path::expandvars(). Valid characters include
// letters, digits and the characters '_' and '-'.
inline bool is_var_char(char c)
{
    return (::isalnum(c) || c == '-' || c == '_');
}

// Searches for the given |rightDelimiter| in |sourceString| starting
// at |startPosition|. If the  delimiter is found, interprets the
// substring, defined by the start position and the position of the
// delimiter, as an environment variable name, resolves the variable
// and then appends the environment variable's value to |resultString|.
// If |rightDelimiter| is not found, appends |leftDelimiter| and the
// substring of |sourceString| starting at |startPosition|, to
// |resultString|.
// If |rightDelimiter| is found but the substring it is not a valid
// environment variable name, appends |leftDelimiter|, the substring
// and |rightDelimiter| to |resultString|.
// Returns the index of the next character to be processed in
// |sourceString|.
size_t replace_substring_variable(
    const os::path::EnvironmentInterface& environment,
    const std::string& sourceString,
    const std::string& leftDelimiter,
    char rightDelimiter,
    size_t startPosition,
    std::string& resultString)
{
    size_t index = startPosition;
    const size_t stringSize = sourceString.size();
    const size_t numCharsLeft = stringSize - startPosition;
    const size_t foundPos = sourceString.find(rightDelimiter,
                                              startPosition);
    if (foundPos == std::string::npos)
    {
        resultString += leftDelimiter;
        resultString += sourceString.substr(startPosition);
        index += numCharsLeft - 1;
    }
    else
    {
        const std::string varName =
                sourceString.substr(startPosition,
                                    foundPos - startPosition);
        index = foundPos;
        std::string varValue;
        if (!environment.getenv(varName, varValue))
        {
            resultString += leftDelimiter;
            resultString += varName;
            resultString += rightDelimiter;
        }
        else
        {
            resultString += varValue;
        }
    }
    return index;
}

}  // namespace


//////////////////////////////////////////////////////////////////////////////////////////////
///
///
void split( const std::string & str, std::vector< std::string > & result, const std::string & sep, int maxsplit )
{
    result.clear();

    if ( maxsplit < 0 ) maxsplit = MAX_32BIT_INT;//result.max_size();


    if ( sep.size() == 0 )
    {
        split_whitespace( str, result, maxsplit );
        return;
    }

    std::string::size_type i,j, len = str.size(), n = sep.size();

    i = j = 0;

    while ( i+n <= len )
    {
        if ( str[i] == sep[0] && str.substr( i, n ) == sep )
        {
            if ( maxsplit-- <= 0 ) break;

            result.push_back( str.substr( j, i - j ) );
            i = j = i + n;
        }
        else
        {
            i++;
        }
    }

    result.push_back( str.substr( j, len-j ) );
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
void rsplit( const std::string & str, std::vector< std::string > & result, const std::string & sep, int maxsplit )
{
    if ( maxsplit < 0 )
    {
        split( str, result, sep, maxsplit );
        return;
    }

    result.clear();

    if ( sep.size() == 0 )
    {
        rsplit_whitespace( str, result, maxsplit );
        return;
    }

    Py_ssize_t i,j, len = (Py_ssize_t) str.size(), n = (Py_ssize_t) sep.size();

    i = j = len;

    while ( i >= n )
    {
        if ( str[i - 1] == sep[n - 1] && str.substr( i - n, n ) == sep )
        {
            if ( maxsplit-- <= 0 ) break;

            result.push_back( str.substr( i, j - i ) );
            i = j = i - n;
        }
        else
        {
            i--;
        }
    }

    result.push_back( str.substr( 0, j ) );
    reverse_strings( result );
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
#define LEFTSTRIP 0
#define RIGHTSTRIP 1
#define BOTHSTRIP 2

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string do_strip( const std::string & str, int striptype, const std::string & chars  )
{
    Py_ssize_t len = (Py_ssize_t) str.size(), i, j, charslen = (Py_ssize_t) chars.size();

    if ( charslen == 0 )
    {
        i = 0;
        if ( striptype != RIGHTSTRIP )
        {
            while ( i < len && ::isspace( str[i] ) )
            {
                i++;
            }
        }

        j = len;
        if ( striptype != LEFTSTRIP )
        {
            do
            {
                j--;
            }
            while (j >= i && ::isspace(str[j]));

            j++;
        }


    }
    else
    {
        const char * sep = chars.c_str();

        i = 0;
        if ( striptype != RIGHTSTRIP )
        {
            while ( i < len && memchr(sep, str[i], charslen) )
            {
                i++;
            }
        }

        j = len;
        if (striptype != LEFTSTRIP)
        {
            do
            {
                j--;
            }
            while (j >= i &&  memchr(sep, str[j], charslen)  );
            j++;
        }


    }

    if ( i == 0 && j == len )
    {
        return str;
    }
    else
    {
        return str.substr( i, j - i );
    }

}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
void partition( const std::string & str, const std::string & sep, std::vector< std::string > & result )
{
    result.resize(3);
    int index = find( str, sep );
    if ( index < 0 )
    {
        result[0] = str;
        result[1] = "";
        result[2] = "";
    }
    else
    {
        result[0] = str.substr( 0, index );
        result[1] = sep;
        result[2] = str.substr( index + sep.size(), str.size() );
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
void rpartition( const std::string & str, const std::string & sep, std::vector< std::string > & result )
{
    result.resize(3);
    int index = rfind( str, sep );
    if ( index < 0 )
    {
        result[0] = "";
        result[1] = "";
        result[2] = str;
    }
    else
    {
        result[0] = str.substr( 0, index );
        result[1] = sep;
        result[2] = str.substr( index + sep.size(), str.size() );
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string strip( const std::string & str, const std::string & chars )
{
    return do_strip( str, BOTHSTRIP, chars );
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string lstrip( const std::string & str, const std::string & chars )
{
    return do_strip( str, LEFTSTRIP, chars );
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string rstrip( const std::string & str, const std::string & chars )
{
    return do_strip( str, RIGHTSTRIP, chars );
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string join( const std::string & str, const std::vector< std::string > & seq )
{
    std::vector< std::string >::size_type seqlen = seq.size(), i;

    if ( seqlen == 0 ) return "";
    if ( seqlen == 1 ) return seq[0];

    std::string result( seq[0] );

    for ( i = 1; i < seqlen; ++i )
    {
        result += str + seq[i];

    }


    return result;
}


//////////////////////////////////////////////////////////////////////////////////////////////
///
///

namespace
{
    /* Matches the end (direction >= 0) or start (direction < 0) of self
     * against substr, using the start and end arguments. Returns
     * -1 on error, 0 if not found and 1 if found.
     */

    int _string_tailmatch(const std::string & self, const std::string & substr,
                          Py_ssize_t start, Py_ssize_t end,
                          int direction)
    {
        Py_ssize_t len = (Py_ssize_t) self.size();
        Py_ssize_t slen = (Py_ssize_t) substr.size();

        const char* sub = substr.c_str();
        const char* str = self.c_str();

        ADJUST_INDICES(start, end, len);

        if (direction < 0) {
            // startswith
            if (start+slen > len)
                return 0;
        } else {
            // endswith
            if (end-start < slen || start > len)
                return 0;
            if (end-slen > start)
                start = end - slen;
        }
        if (end-start >= slen)
            return (!std::memcmp(str+start, sub, slen));

        return 0;
    }
}

bool endswith( const std::string & str, const std::string & suffix, int start, int end )
{
    int result = _string_tailmatch(str, suffix,
                                   (Py_ssize_t) start, (Py_ssize_t) end, +1);
    //if (result == -1) // TODO: Error condition

    return result != 0;
}


bool startswith( const std::string & str, const std::string & prefix, int start, int end )
{
    int result = _string_tailmatch(str, prefix,
                                   (Py_ssize_t) start, (Py_ssize_t) end, -1);
    //if (result == -1) // TODO: Error condition

    return result != 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///

bool isalnum( const std::string & str )
{
    std::string::size_type len = str.size(), i;
    if ( len == 0 ) return false;


    if( len == 1 )
    {
        return ::isalnum( str[0] ) != 0;
    }

    for ( i = 0; i < len; ++i )
    {
        if ( !::isalnum( str[i] ) ) return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
bool isalpha( const std::string & str )
{
    std::string::size_type len = str.size(), i;
    if ( len == 0 ) return false;
    if( len == 1 ) return ::isalpha( (int) str[0] ) != 0;

    for ( i = 0; i < len; ++i )
    {
       if ( !::isalpha( (int) str[i] ) ) return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
bool isdigit( const std::string & str )
{
    std::string::size_type len = str.size(), i;
    if ( len == 0 ) return false;
    if( len == 1 ) return ::isdigit( str[0] ) != 0;

    for ( i = 0; i < len; ++i )
    {
       if ( ! ::isdigit( str[i] ) ) return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
bool islower( const std::string & str )
{
    bool hasCasedCharacter = false;

    for (std::string::size_type i = 0; i < str.size(); ++i)
    {
        if ( !::isalpha( str[i] ) ) continue;

        hasCasedCharacter = true;
        if ( !::islower( str[i] ) ) return false;
    }

    return hasCasedCharacter;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
bool isspace( const std::string & str )
{
    std::string::size_type len = str.size(), i;
    if ( len == 0 ) return false;
    if( len == 1 ) return ::isspace( str[0] ) != 0;

    for ( i = 0; i < len; ++i )
    {
       if ( !::isspace( str[i] ) ) return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
bool istitle( const std::string & str )
{
    std::string::size_type len = str.size(), i;

    if ( len == 0 ) return false;
    if ( len == 1 ) return ::isupper( str[0] ) != 0;

    bool cased = false, previous_is_cased = false;

    for ( i = 0; i < len; ++i )
    {
        if ( ::isupper( str[i] ) )
        {
            if ( previous_is_cased )
            {
                return false;
            }

            previous_is_cased = true;
            cased = true;
        }
        else if ( ::islower( str[i] ) )
        {
            if (!previous_is_cased)
            {
                return false;
            }

            previous_is_cased = true;
            cased = true;

        }
        else
        {
            previous_is_cased = false;
        }
    }

    return cased;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
bool isupper( const std::string & str )
{
    bool hasCasedCharacter = false;

    for (std::string::size_type i = 0; i < str.size(); ++i)
    {
        if ( !::isalpha( str[i] ) ) continue;

        hasCasedCharacter = true;
        if ( !::isupper( str[i] ) ) return false;
    }

    return hasCasedCharacter;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string capitalize( const std::string & str )
{
    std::string s( str );
    std::string::size_type len = s.size(), i;

    if ( len > 0)
    {
        if (::islower(s[0])) s[0] = (char) ::toupper( s[0] );
    }

    for ( i = 1; i < len; ++i )
    {
        if (::isupper(s[i])) s[i] = (char) ::tolower( s[i] );
    }

    return s;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string lower( const std::string & str )
{
    std::string s( str );
    std::string::size_type len = s.size(), i;

    for ( i = 0; i < len; ++i )
    {
        if ( ::isupper( s[i] ) ) s[i] = (char) ::tolower( s[i] );
    }

    return s;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string upper( const std::string & str )
{
    std::string s( str ) ;
    std::string::size_type len = s.size(), i;

    for ( i = 0; i < len; ++i )
    {
        if ( ::islower( s[i] ) ) s[i] = (char) ::toupper( s[i] );
    }

    return s;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string swapcase( const std::string & str )
{
    std::string s( str );
    std::string::size_type len = s.size(), i;

    for ( i = 0; i < len; ++i )
    {
        if ( ::islower( s[i] ) ) s[i] = (char) ::toupper( s[i] );
        else if (::isupper( s[i] ) ) s[i] = (char) ::tolower( s[i] );
    }

    return s;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string title( const std::string & str )
{
    std::string s( str );
    std::string::size_type len = s.size(), i;
    bool previous_is_cased = false;

    for ( i = 0; i < len; ++i )
    {
        int c = s[i];
        if ( ::islower(c) )
        {
            if ( !previous_is_cased )
            {
                s[i] = (char) ::toupper(c);
            }
            previous_is_cased = true;
        }
        else if ( ::isupper(c) )
        {
            if ( previous_is_cased )
            {
                s[i] = (char) ::tolower(c);
            }
            previous_is_cased = true;
        }
        else
        {
            previous_is_cased = false;
        }
    }

    return s;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string translate( const std::string & str, const std::string & table, const std::string & deletechars )
{
    std::string s;
    std::string::size_type len = str.size(), dellen = deletechars.size();

    if ( table.size() != 256 )
    {
        // TODO : raise exception instead
        return str;
    }

    //if nothing is deleted, use faster code
    if ( dellen == 0 )
    {
        s = str;
        for ( std::string::size_type i = 0; i < len; ++i )
        {
            s[i] = table[ s[i] ];
        }
        return s;
    }


    int trans_table[256];
    for ( int i = 0; i < 256; i++)
    {
        trans_table[i] = table[i];
    }

    for ( std::string::size_type i = 0; i < dellen; i++)
    {
        trans_table[(int) deletechars[i] ] = -1;
    }

    for ( std::string::size_type i = 0; i < len; ++i )
    {
        if ( trans_table[ (int) str[i] ] != -1 )
        {
            s += table[ str[i] ];
        }
    }

    return s;

}


//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string zfill( const std::string & str, int width )
{
    int len = (int)str.size();

    if ( len >= width )
    {
        return str;
    }

    std::string s( str );

    int fill = width - len;

    s = std::string( fill, '0' ) + s;


    if ( s[fill] == '+' || s[fill] == '-' )
    {
        s[0] = s[fill];
        s[fill] = '0';
    }

    return s;

}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string ljust( const std::string & str, int width )
{
    std::string::size_type len = str.size();
    if ( (( int ) len ) >= width ) return str;
    return str + std::string( width - len, ' ' );
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string rjust( const std::string & str, int width )
{
    std::string::size_type len = str.size();
    if ( (( int ) len ) >= width ) return str;
    return std::string( width - len, ' ' ) + str;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string center( const std::string & str, int width )
{
    int len = (int) str.size();
    int marg, left;

    if ( len >= width ) return str;

    marg = width - len;
    left = marg / 2 + (marg & width & 1);

    return std::string( left, ' ' ) + str + std::string( marg - left, ' ' );

}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string slice( const std::string & str, int start, int end )
{
    ADJUST_INDICES(start, end, (int) str.size());
    if ( start >= end ) return "";
    return str.substr( start, end - start );
}


//////////////////////////////////////////////////////////////////////////////////////////////
///
///
int find( const std::string & str, const std::string & sub, int start, int end  )
{
    ADJUST_INDICES(start, end, (int) str.size());

    std::string::size_type result = str.find( sub, start );

    // If we cannot find the string, or if the end-point of our found substring is past
    // the allowed end limit, return that it can't be found.
    if( result == std::string::npos ||
       (result + sub.size() > (std::string::size_type)end) )
    {
        return -1;
    }

    return (int) result;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
int index( const std::string & str, const std::string & sub, int start, int end  )
{
    return find( str, sub, start, end );
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
int rfind( const std::string & str, const std::string & sub, int start, int end )
{
    ADJUST_INDICES(start, end, (int) str.size());

    // std::string#rfind's second argument specifies the last location at
    // which the substring can start. i.e. the substring can continue past
    // |end|.  Accordingly, we adjust |end| to match the Python semantics.
    if( end > (int) sub.size() )
        end -= (int) sub.size();
    else
        end = 0;

    std::string::size_type result = str.rfind( sub, end );

    if( result == std::string::npos ||
        result < (std::string::size_type)start )
        return -1;

    return (int)result;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
int rindex( const std::string & str, const std::string & sub, int start, int end )
{
    return rfind( str, sub, start, end );
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string expandtabs( const std::string & str, int tabsize )
{
    std::string s( str );

    std::string::size_type len = str.size(), i = 0;
    int offset = 0;

    int j = 0;

    for ( i = 0; i < len; ++i )
    {
        if ( str[i] == '\t' )
        {

            if ( tabsize > 0 )
            {
                int fillsize = tabsize - (j % tabsize);
                j += fillsize;
                s.replace( i + offset, 1, std::string( fillsize, ' ' ));
                offset += fillsize - 1;
            }
            else
            {
                s.replace( i + offset, 1, "" );
                offset -= 1;
            }

        }
        else
        {
            j++;

            if (str[i] == '\n' || str[i] == '\r')
            {
                j = 0;
            }
        }
    }

    return s;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
int count( const std::string & str, const std::string & substr, int start, int end )
{
    int nummatches = 0;
    int cursor = start;
    int needlelen = std::max((int)substr.size(), 1);

    while ( 1 )
    {
        cursor = find( str, substr, cursor, end );

        if ( cursor < 0 ) break;

        cursor += needlelen;
        nummatches += 1;
    }

    return nummatches;


}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///

std::string replace( const std::string & str, const std::string & oldstr, const std::string & newstr, int count )
{
    int sofar = 0;
    int cursor = 0;
    std::string s( str );

    std::string::size_type oldlen = oldstr.size(), newlen = newstr.size();

    cursor = find( s, oldstr, cursor );

    while ( cursor != -1 && cursor <= (int)s.size() )
    {
        if ( count > -1 && sofar >= count )
        {
            break;
        }

        s.replace( cursor, oldlen, newstr );
        cursor += (int) newlen;

        if ( oldlen != 0)
        {
            cursor = find( s, oldstr, cursor );
        }
        else
        {
            ++cursor;
        }

        ++sofar;
    }

    return s;

}


//////////////////////////////////////////////////////////////////////////////////////////////
///
///
void splitlines(  const std::string & str, std::vector< std::string > & result, bool keepends )
{
    result.clear();
    std::string::size_type len = str.size(), i, j, eol;

     for (i = j = 0; i < len; )
     {
        while (i < len && str[i] != '\n' && str[i] != '\r') i++;

        eol = i;
        if (i < len)
        {
            if (str[i] == '\r' && i + 1 < len && str[i+1] == '\n')
            {
                i += 2;
            }
            else
            {
                i++;
            }
            if (keepends)
            eol = i;

        }

        result.push_back( str.substr( j, eol - j ) );
        j = i;

    }

    if (j < len)
    {
        result.push_back( str.substr( j, len - j ) );
    }

}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::string mul( const std::string & str, int n )
{
    // Early exits
    if (n <= 0) return "";
    if (n == 1) return str;

    std::ostringstream os;
    for(int i=0; i<n; ++i)
    {
        os << str;
    }
    return os.str();
}



namespace os
{
namespace path
{

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
/// These functions are C++ ports of the python2.7 versions of os.path,
/// and come from genericpath.py, ntpath.py, posixpath.py

/// Split a pathname into drive and path specifiers.
/// Returns drivespec, pathspec. Either part may be empty.
void splitdrive_nt(std::string & drivespec, std::string & pathspec,
                   const std::string & p)
{
    if(pystring::slice(p, 1, 2) == ":")
    {
        std::string path = p; // In case drivespec == p
        drivespec = pystring::slice(path, 0, 2);
        pathspec = pystring::slice(path, 2);
    }
    else
    {
        drivespec = "";
        pathspec = p;
    }
}

// On Posix, drive is always empty
void splitdrive_posix(std::string & drivespec, std::string & pathspec,
                      const std::string & path)
{
    drivespec = "";
    pathspec = path;
}

void splitdrive(std::string & drivespec, std::string & pathspec,
                const std::string & path)
{
#if defined(_WIN32)
    return splitdrive_nt(drivespec, pathspec, path);
#else
    return splitdrive_posix(drivespec, pathspec, path);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///

// Test whether a path is absolute
// In windows, if the character to the right of the colon
// is a forward or backslash it's absolute.
bool isabs_nt(const std::string & path)
{
    std::string drivespec, pathspec;
    splitdrive_nt(drivespec, pathspec, path);
    if(pathspec.empty()) return false;
    return ((pathspec[0] == '/') || (pathspec[0] == '\\'));
}

bool isabs_posix(const std::string & s)
{
    return pystring::startswith(s, "/");
}

bool isabs(const std::string & path)
{
#if defined(_WIN32)
    return isabs_nt(path);
#else
    return isabs_posix(path);
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////////
///
///

std::string abspath_nt(const std::string & path, const std::string & cwd)
{
    std::string p = path;
    if(!isabs_nt(p)) p = join_nt(cwd, p);
    return normpath_nt(p);
}

std::string abspath_posix(const std::string & path, const std::string & cwd)
{
    std::string p = path;
    if(!isabs_posix(p)) p = join_posix(cwd, p);
    return normpath_posix(p);
}

std::string abspath(const std::string & path, const std::string & cwd)
{
#if defined(_WIN32)
    return abspath_nt(path, cwd);
#else
    return abspath_posix(path, cwd);
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////////
///
///

std::string join_nt(const std::vector< std::string > & paths)
{
    if(paths.empty()) return "";
    if(paths.size() == 1) return paths[0];

    std::string path = paths[0];

    for(unsigned int i=1; i<paths.size(); ++i)
    {
        std::string b = paths[i];

        bool b_nts = false;
        if(path.empty())
        {
            b_nts = true;
        }
        else if(isabs_nt(b))
        {
            // This probably wipes out path so far.  However, it's more
            // complicated if path begins with a drive letter:
            //     1. join('c:', '/a') == 'c:/a'
            //     2. join('c:/', '/a') == 'c:/a'
            // But
            //     3. join('c:/a', '/b') == '/b'
            //     4. join('c:', 'd:/') = 'd:/'
            //     5. join('c:/', 'd:/') = 'd:/'

            if( (pystring::slice(path, 1, 2) != ":") ||
                (pystring::slice(b, 1, 2) == ":") )
            {
                // Path doesnt start with a drive letter
                b_nts = true;
            }
            // Else path has a drive letter, and b doesn't but is absolute.
            else if((path.size()>3) ||
                    ((path.size()==3) && !pystring::endswith(path, "/") && !pystring::endswith(path, "\\")))
            {
                b_nts = true;
            }
        }

        if(b_nts)
        {
            path = b;
        }
        else
        {
            // Join, and ensure there's a separator.
            // assert len(path) > 0
            if( pystring::endswith(path, "/") || pystring::endswith(path, "\\"))
            {
                if(pystring::startswith(b,"/") || pystring::startswith(b,"\\"))
                {
                    path += pystring::slice(b, 1);
                }
                else
                {
                    path += b;
                }
            }
            else if(pystring::endswith(path, ":"))
            {
                path += b;
            }
            else if(!b.empty())
            {
                if(pystring::startswith(b,"/") || pystring::startswith(b,"\\"))
                {
                    path += b;
                }
                else
                {
                    path += "\\" + b;
                }
            }
            else
            {
                // path is not empty and does not end with a backslash,
                // but b is empty; since, e.g., split('a/') produces
                // ('a', ''), it's best if join() adds a backslash in
                // this case.
                path += "\\";
            }
        }
    }

    return path;
}

// Join two or more pathname components, inserting "\\" as needed.
std::string join_nt(const std::string & a, const std::string & b)
{
    std::vector< std::string > paths(2);
    paths[0] = a;
    paths[1] = b;
    return join_nt(paths);
}

// Join pathnames.
// If any component is an absolute path, all previous path components
// will be discarded.
// Ignore the previous parts if a part is absolute.
// Insert a '/' unless the first part is empty or already ends in '/'.

std::string join_posix(const std::vector< std::string > & paths)
{
    if(paths.empty()) return "";
    if(paths.size() == 1) return paths[0];

    std::string path = paths[0];

    for(unsigned int i=1; i<paths.size(); ++i)
    {
        std::string b = paths[i];
        if(pystring::startswith(b, "/"))
        {
            path = b;
        }
        else if(path.empty() || pystring::endswith(path, "/"))
        {
            path += b;
        }
        else
        {
            path += "/" + b;
        }
    }

    return path;
}

std::string join_posix(const std::string & a, const std::string & b)
{
    std::vector< std::string > paths(2);
    paths[0] = a;
    paths[1] = b;
    return join_posix(paths);
}

std::string join(const std::string & path1, const std::string & path2)
{
#if defined(_WIN32)
    return join_nt(path1, path2);
#else
    return join_posix(path1, path2);
#endif
}


std::string join(const std::vector< std::string > & paths)
{
#if defined(_WIN32)
    return join_nt(paths);
#else
    return join_posix(paths);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///


// Split a pathname.
// Return (head, tail) where tail is everything after the final slash.
// Either part may be empty

void split_nt(std::string & head, std::string & tail, const std::string & path)
{
    std::string d, p;
    splitdrive_nt(d, p, path);

    // set i to index beyond p's last slash
    int i = (int)p.size();

    while(i>0 && (p[i-1] != '\\') && (p[i-1] != '/'))
    {
        i = i - 1;
    }

    head = pystring::slice(p,0,i);
    tail = pystring::slice(p,i); // now tail has no slashes

    // remove trailing slashes from head, unless it's all slashes
    std::string head2 = pystring::rstrip(head, "/\\");
    if(!head2.empty()) head = head2;

    head = d + head;
}


// Split a path in head (everything up to the last '/') and tail (the
// rest).  If the path ends in '/', tail will be empty.  If there is no
// '/' in the path, head  will be empty.
// Trailing '/'es are stripped from head unless it is the root.

void split_posix(std::string & head, std::string & tail, const std::string & p)
{
    int i = pystring::rfind(p, "/") + 1;

    head = pystring::slice(p,0,i);
    tail = pystring::slice(p,i);

    if(!head.empty() && (head != pystring::mul("/", (int) head.size())))
    {
        head = pystring::rstrip(head, "/");
    }
}

void split(std::string & head, std::string & tail, const std::string & path)
{
#if defined(_WIN32)
    return split_nt(head, tail, path);
#else
    return split_posix(head, tail, path);
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////////
///
///

std::string basename_nt(const std::string & path)
{
    std::string head, tail;
    split_nt(head, tail, path);
    return tail;
}

std::string basename_posix(const std::string & path)
{
    std::string head, tail;
    split_posix(head, tail, path);
    return tail;
}

std::string basename(const std::string & path)
{
#if defined(_WIN32)
    return basename_nt(path);
#else
    return basename_posix(path);
#endif
}

std::string dirname_nt(const std::string & path)
{
    std::string head, tail;
    split_nt(head, tail, path);
    return head;
}

std::string dirname_posix(const std::string & path)
{
    std::string head, tail;
    split_posix(head, tail, path);
    return head;
}

std::string dirname(const std::string & path)
{
#if defined(_WIN32)
    return dirname_nt(path);
#else
    return dirname_posix(path);
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////////
///
///

// Normalize a path, e.g. A//B, A/./B and A/foo/../B all become A\B.
std::string normpath_nt(const std::string & p)
{
    std::string path = p;
    path = pystring::replace(path, "/","\\");

    std::string prefix;
    splitdrive_nt(prefix, path, path);

    // We need to be careful here. If the prefix is empty, and the path starts
    // with a backslash, it could either be an absolute path on the current
    // drive (\dir1\dir2\file) or a UNC filename (\\server\mount\dir1\file). It
    // is therefore imperative NOT to collapse multiple backslashes blindly in
    // that case.
    // The code below preserves multiple backslashes when there is no drive
    // letter. This means that the invalid filename \\\a\b is preserved
    // unchanged, where a\\\b is normalised to a\b. It's not clear that there
    // is any better behaviour for such edge cases.

    if(prefix.empty())
    {
        // No drive letter - preserve initial backslashes
        while(pystring::slice(path,0,1) == "\\")
        {
            prefix = prefix + "\\";
            path = pystring::slice(path,1);
        }
    }
    else
    {
        // We have a drive letter - collapse initial backslashes
        if(pystring::startswith(path, "\\"))
        {
            prefix = prefix + "\\";
            path = pystring::lstrip(path, "\\");
        }
    }

    std::vector<std::string> comps;
    pystring::split(path, comps, "\\");

    int i = 0;

    while(i<(int)comps.size())
    {
        if(comps[i].empty() || comps[i] == ".")
        {
            comps.erase(comps.begin()+i);
        }
        else if(comps[i] == "..")
        {
            if(i>0 && comps[i-1] != "..")
            {
                comps.erase(comps.begin()+i-1, comps.begin()+i+1);
                i -= 1;
            }
            else if(i == 0 && pystring::endswith(prefix, "\\"))
            {
                comps.erase(comps.begin()+i);
            }
            else
            {
                i += 1;
            }
        }
        else
        {
            i += 1;
        }
    }

    // If the path is now empty, substitute '.'
    if(prefix.empty() && comps.empty())
    {
        comps.push_back(".");
    }

    return prefix + pystring::join("\\", comps);
}

// Normalize a path, e.g. A//B, A/./B and A/foo/../B all become A/B.
// It should be understood that this may change the meaning of the path
// if it contains symbolic links!
// Normalize path, eliminating double slashes, etc.

std::string normpath_posix(const std::string & p)
{
    if(p.empty()) return ".";

    std::string path = p;

    int initial_slashes = pystring::startswith(path,"/") ? 1 : 0;

    // POSIX allows one or two initial slashes, but treats three or more
    // as single slash.

    if (initial_slashes && pystring::startswith(path,"//")
        && !pystring::startswith(path,"///"))
        initial_slashes = 2;

    std::vector<std::string> comps, new_comps;
    pystring::split(path, comps, "/");

    for(unsigned int i=0; i<comps.size(); ++i)
    {
        std::string comp = comps[i];
        if(comp.empty() || comp == ".")
            continue;

        if( (comp != "..") || ((initial_slashes == 0) && new_comps.empty()) ||
            (!new_comps.empty() && new_comps[new_comps.size()-1] == ".."))
        {
            new_comps.push_back(comp);
        }
        else if (!new_comps.empty())
        {
            new_comps.pop_back();
        }
    }

    path = pystring::join("/", new_comps);

    if (initial_slashes > 0)
        path = pystring::mul("/",initial_slashes) + path;

    if(path.empty()) return ".";
    return path;
}

std::string normpath(const std::string & path)
{
#if defined(_WIN32)
    return normpath_nt(path);
#else
    return normpath_posix(path);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///

// Split the extension from a pathname.
// Extension is everything from the last dot to the end, ignoring
// leading dots.  Returns "(root, ext)"; ext may be empty.
// It is always true that root + ext == p

void splitext_generic(std::string & root, std::string & ext,
                      const std::string & p,
                      const std::string & sepChar,
                      const std::string & altSepChar,
                      const std::string & extSepChar)
{
    int sepIndex = pystring::rfind(p, sepChar);
    if(!altSepChar.empty())
    {
        int altsepIndex = pystring::rfind(p, altSepChar);
        sepIndex = std::max(sepIndex, altsepIndex);
    }

    int dotIndex = pystring::rfind(p, extSepChar);
    if(dotIndex > sepIndex)
    {
        // Skip all leading dots
        int filenameIndex = sepIndex + 1;

        while(filenameIndex < dotIndex)
        {
            if(pystring::slice(p,filenameIndex) != extSepChar)
            {
                root = pystring::slice(p, 0, dotIndex);
                ext = pystring::slice(p, dotIndex);
                return;
            }

            filenameIndex += 1;
        }
    }

    root = p;
    ext = "";
}

void splitext_nt(std::string & root, std::string & ext, const std::string & path)
{
    return splitext_generic(root, ext, path,
                            "\\", "/", ".");
}

void splitext_posix(std::string & root, std::string & ext, const std::string & path)
{
    return splitext_generic(root, ext, path,
                            "/", "", ".");
}

void splitext(std::string & root, std::string & ext, const std::string & path)
{
#if defined(_WIN32)
    return splitext_nt(root, ext, path);
#else
    return splitext_posix(root, ext, path);
#endif
}

#if defined(_WIN32)
static std::string ToUTF8(const std::wstring& u16String)
{
    if (u16String.empty())
        return std::string();
    int numBytes = WideCharToMultiByte(CP_UTF8, 0, u16String.data(),
                                       static_cast<int>(u16String.size()),
                                       NULL, 0, NULL, NULL);
    std::string u8String(numBytes, '\0');
    if (!WideCharToMultiByte(CP_UTF8, 0, u16String.data(),
                             static_cast<int>(u16String.size()),
                             &*u8String.begin(), numBytes, NULL, NULL))
    {
        return std::string();
    }
    return u8String;
}

static std::wstring ToUTF16(const std::string& u8String)
{
    if (u8String.empty())
        return std::wstring();
    int numCodeUnits = MultiByteToWideChar(CP_UTF8, 0, u8String.data(),
                                           static_cast<int>(u8String.size()),
                                           NULL, 0);
    std::wstring u16String(numCodeUnits, L'\0');
    if (!MultiByteToWideChar(CP_UTF8, 0, u8String.data(),
                             static_cast<int>(u8String.size()),
                             &*u16String.begin(), numCodeUnits))
    {
        return std::wstring();
    }
    return u16String;
}

static std::string getUserHomeFromRegistry(const std::string& userName)
{
    // Compute the path to the registry key containing the user's profile
    // directory.
    std::wstring regKey(
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\");
    std::wstring regValueName(L"ProfileImagePath");
    {
        std::wstring userNameUTF16 = ToUTF16(userName);
        DWORD sidSize = 0;
        DWORD domainLength = 0;
        SID_NAME_USE sidNameUse;
        if (!LookupAccountNameW(NULL, userNameUTF16.c_str(), NULL, &sidSize,
                                NULL, &domainLength, &sidNameUse) &&
            GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            return std::string();
        }
        // Get SID buffer and domain. Domain is not used by us, but
        // LookupAccountNameW requires that we obtain it.
        std::vector<char> sidByteArray(sidSize);
        std::vector<wchar_t> domain(domainLength);
        PSID pSid = reinterpret_cast<PSID>(sidByteArray.data());
        if (!LookupAccountNameW(NULL, userNameUTF16.c_str(), pSid, &sidSize,
                                domain.data(), &domainLength, &sidNameUse))
        {
            return std::string();
        }
        wchar_t* sidCString = NULL;
        if (!ConvertSidToStringSidW(pSid, &sidCString))
            return std::string();
        regKey += sidCString;
        LocalFree(sidCString);
    }

    DWORD userPathBytes = 0;
    std::wstring userPathUTF16;
    LONG error;
    error = RegGetValueW(
        HKEY_LOCAL_MACHINE, regKey.c_str(), regValueName.c_str(),
        RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ, NULL, NULL, &userPathBytes);
    if (error != ERROR_MORE_DATA)
        return std::string();

    userPathUTF16.resize(userPathBytes / sizeof(wchar_t) - 1);
    error =
        RegGetValueW(HKEY_LOCAL_MACHINE, regKey.c_str(), regValueName.c_str(),
                     RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ, NULL,
                     &*userPathUTF16.begin(), &userPathBytes);
    if (error != ERROR_SUCCESS)
        return std::string();

    return ToUTF8(userPathUTF16);
}
#endif  // _WIN32

static std::string getUserHome(const std::string& userName = std::string())
{
#if !defined(_WIN32)
    if (userName.empty())
    {
        if (const char* userHome = ::getenv("HOME"))
            return userHome;
    }
    std::vector<char> pwdStorage(
        std::max<size_t>(sysconf(_SC_GETPW_R_SIZE_MAX), 1024));
    struct passwd pwdBuf;
    struct passwd *pwd = NULL;
    if (userName.empty())
    {
        getpwuid_r(getuid(), &pwdBuf, pwdStorage.data(), pwdStorage.size(),
                   &pwd);
        if (!pwd)
            return std::string();

        return pwd->pw_dir;
    }
    else
    {
        getpwnam_r(userName.c_str(), &pwdBuf, pwdStorage.data(),
                   pwdStorage.size(), &pwd);
        if (!pwd)
            return std::string();

        return pwd->pw_dir;
    }
#else
    if (userName.empty())
    {
        wchar_t* homePath = NULL;
        if (!SUCCEEDED(
                SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, &homePath)))
        {
            return std::string();
        }
        std::string homePathUTF8 = ToUTF8(homePath);
        SHFree(homePath);
        return homePathUTF8;
    }
    else
    {
        // Home folder of a specific user. We do a more thorough job than
        // the CPython implementation in ntpath.h:
        // - read from registry
        // - if that fails, check if there is directory named after the
        //   user in the profiles directory.
        std::string userPath = getUserHomeFromRegistry(userName);
        // if (!userPath.empty())
        // return userPath;

        // Fallback. Gets base location for all user profile directories.
        // Assumes that it is <dir>+<username>. Not a good assumption.
        wchar_t* profilesPath;
        if (!SUCCEEDED(SHGetKnownFolderPath(FOLDERID_UserProfiles, 0, NULL,
                                            &profilesPath)))
        {
            return std::string();
        }

        std::wstring userPathUTF16(profilesPath);
        SHFree(profilesPath);

        userPathUTF16.push_back(L'\\');
        userPathUTF16.append(ToUTF16(userName));

        struct _stat sb;
        if (_wstat(userPathUTF16.c_str(), &sb) || !(sb.st_mode & _S_IFDIR))
        {
            return std::string();
        }

        return ToUTF8(userPathUTF16);
    }
#endif  // _WIN32
}

std::string expanduser(const std::string& path)
{
    if (!pystring::startswith(path, "~"))
        return path;

#if !defined(_WIN32)
    int i = pystring::find(path, "/", 1);
    if (i < 0)
    {
        i = static_cast<int>(path.size());
    }
#else
    int i = 1, n = static_cast<int>(path.size());
    while (i < n && path[i] != '/' && path[i] != '\\')
    {
        ++i;
    }
#endif  // _WIN32
    std::string userHome;
    if (i == 1)
    {
        userHome = getUserHome();
    }
    else
    {
        userHome = getUserHome(pystring::slice(path, 1, i));
    }

    if (userHome.empty())
    {
        return path;
    }

    if (pystring::endswith(userHome, os::sep))
    {
        i += 1;
    }

    return userHome + pystring::slice(path, i);
}

// Expands environment variables contained in the given |path|.
//
// This function implements variables expansion following the convention
// in place for Python's os.path.expandvars():
//  - elements within single quotes are not expanded
//  - '$$' is translated into '$'
//
// The following patterns are accepted:
//  - ${varname}
//  - $varname
// Variable names can be made out of letters, digits and the characters '_'
// and '-' (though is not verified in the ${varname} cases).
std::string expandvars(const std::string& path,
                       EnvironmentInterface* environmentPtr)
{
    class SystemEnvironment : public EnvironmentInterface
    {
        bool getenv(const std::string& key, std::string& result) const
        {
#if !defined(_WIN32)
            const char* resultCStr = ::getenv(key.c_str());
            result = resultCStr ? resultCStr : "";
            return resultCStr != NULL;
#else
            size_t returnValue = 0;
            char resultCStr[4096];
            ::getenv_s(&returnValue, resultCStr, key.c_str());
            result = returnValue ? resultCStr : "";
            return returnValue != 0;
#endif  // _WIN32
        }
    } systemEnvironment;
    const EnvironmentInterface& environment =
        environmentPtr ? *environmentPtr : systemEnvironment;

    std::string result;
    size_t index = 0;
    const size_t pathSize = path.size();
    while (index < pathSize)
    {
        const char& currChar = path.at(index);

        if (currChar == '\'')
        {
            const size_t numCharsLeft = pathSize - index;
            const size_t foundPos = path.find(currChar, index + 1);
            if (foundPos == std::string::npos)
            {
                result += currChar;
                result += path.substr(index + 1);
                index += numCharsLeft - 1;
            }
            else
            {
                result += currChar;
                result += path.substr(index + 1, foundPos - index);
                index = foundPos;
            }
        }
        else if (currChar == '$')
        {
            // Handle escaped '$'
            if (path.at(index + 1) == '$')
            {
                result += currChar;
                index += 1;
            }
            else if (path.at(index + 1) == '{')
            {
                index = replace_substring_variable(environment, path, "${", '}',
                                                   index + 2, result);
            }
            else
            {
                std::string varName;
                ++index;
                char c = path.at(index);
                while (c && is_var_char(c))
                {
                    varName += c;
                    index += 1;
                    if (index >= pathSize) break;
                    c = path.at(index);
                }
                std::string varValue;
                if (environment.getenv(varName, varValue))
                {
                    result += varValue;
                }
                else
                {
                    result += '$';
                    result += varName;
                }
                if (c)
                {
                    index -= 1;
                }
            }
        }
        else
        {
            result += currChar;
        }
        index += 1;
    }
    return result;
}

} // namespace path
} // namespace os


}
GEOLIB3_PYSTRING_NAMESPACE_EXIT
