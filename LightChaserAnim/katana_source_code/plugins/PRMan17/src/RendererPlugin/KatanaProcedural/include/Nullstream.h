// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef INCLUDED_NULLSTREAM_H
#define INCLUDED_NULLSTREAM_H

#if 0 //__GNUC__ == 3

#include <assert.h>
#include <iostream>

/**
 * Take the input bytes, and drop them on the floor.
 * I'm assuming the underlying streambuf
 * takes care of thread synchronization/reentrance 
 * for us.  
 */
template <class Elem, class Tr = std::char_traits<Elem> >
class  null_streambuf : public std::basic_streambuf<Elem,Tr> 
{
public:
  typedef typename std::basic_streambuf<Elem,Tr>::char_type char_type;
  typedef typename std::basic_streambuf<Elem,Tr>::traits_type traits_type;
  typedef typename std::basic_streambuf<Elem,Tr>::int_type int_type;
  typedef typename std::basic_streambuf<Elem,Tr>::pos_type pos_type;
  typedef typename std::basic_streambuf<Elem,Tr>::off_type off_type;

  null_streambuf() : oi_bufsize(1024)
  {
      setp(op_buf, op_buf+oi_bufsize);
      setg(0,0,0);
  }

  ~null_streambuf()
  {}

private:
    char        op_buf[1024];
    const int   oi_bufsize;

protected:
    /** Handle overflow of the put buffer  */
    virtual int overflow(int i_code=EOF)
    {
        // allocate a buffer if one isn't available
        if ( !pbase() )
        {
            assert( 0 );
            return EOF;
            setg( 0,0,0 );
        }
        sync();
        // reset the put area, leave one character at the end for a '\0'
        setp ( pbase(), pbase() + oi_bufsize - 1 );
        return 0;
    }

    /** Synchornize the destination (nowhere) with the stream buffer */
    virtual int sync() { return 0; }
};


/**
 * nullstream is a stream that goes nowhere.
 *    This is a useful place to send a cdebug stream
 *    for a piece of production software.
 */
template <class Elem, class Tr = std::char_traits<Elem> >
class basic_nullstream : public std::basic_ostream<Elem,Tr> 
{
public:
    basic_nullstream():
    std::basic_ostream<Elem,Tr>(0),
    ox_streambuf()
    {
        std::ios::init( &ox_streambuf );
        this->exceptions( static_cast<std::ios_base::iostate>(0) ); // do not throw exceptions!
    }
    virtual ~basic_nullstream() {}

private:
    null_streambuf<Elem,Tr> ox_streambuf;
};
typedef basic_nullstream<char,std::char_traits<char> > nullstream;

#else

#include <assert.h>
#include <iostream>

/**
 * The streambuf that manages the incoming strings
 * and funnels them to syslog.  This is a very basic
 * derivation.  I'm assuming the underlying streambuf
 * takes care of thread synchronization/reentrance 
 * for us.  
 */
class nullbuf : public std::streambuf
{ /* a stream buffer for files */
private:
    char        op_buf[1024];
    const int   oi_bufsize;

protected:
    /** Handle overflow of the put buffer  */
    virtual int overflow(int i_code=EOF)
    {
        // allocate a buffer if one isn't available
        if ( !pbase() )
        {
            assert( 0 );
            return EOF;
            setg( 0,0,0 );
        }
        sync();
        // reset the put area, leave one character at the end for a '\0'
        setp ( pbase(), pbase() + oi_bufsize - 1 );
        return 0;
    }

    /** Synchornize the destination (nowhere) with the stream buffer */
    virtual int sync() { return 0; }

public:
    nullbuf (): oi_bufsize ( 1024 )
    {
        setp ( op_buf, op_buf+oi_bufsize );
        setg ( 0,0,0 );
    }

    ~nullbuf() {}
};

/**
 * nullstream is a stream that goes nowhere.
 *    This is a useful place to send a cdebug stream
 *    for a piece of production software.
 */
class nullstream : public std::ostream
{
private:
    nullbuf   ox_streambuf;
public:
    nullstream (): std::ostream (0),ox_streambuf()
    {
        std::ios::init ( &ox_streambuf );
    }
    virtual ~nullstream() {}
};

#endif

#endif //INCLUDED_NULLSTREAM_H

