//*****************************************************************************
//
//             Copyright (c) 2009 Sony Pictures Imageworks Inc.
//                            All rights reserved.
//
//  This  material  contains  the confidential and  proprietary information
//  of Sony Pictures Imageworks Inc.   and may not be disclosed,  copied or
//  duplicated  in any form,  electronic or hardcopy,  in whole  or in part,
//  without  the express prior written  consent of Sony Pictures Imageworks
//  Inc. This copyright notice does not imply publication.
//
//*****************************************************************************

#ifndef INCLUDED_FNGEOLIBUTIL_TIMER_H
#define INCLUDED_FNGEOLIBUTIL_TIMER_H

#ifdef _WIN32
#error "FnGeolibUtil::Timer is not supported on Windows."
#endif

#include <FnPlatform/internal/Portability.h>

#include <iostream>
#include <sys/time.h>
#include <string>

#include "ns.h"

/*
  This is a very basic timer class, useful in profiling short code segments.
  You initialize the timer by creating the object, and then stop the timer by destroying it.
  The simplest way to do this is to use scoping:

  <CODE>

  {
     UTIL::Timer myTimer("Code Chunk A");

     <CODE YOU WANT TO TIME>

  } <- Location A

  <CODE>

  When location A is reached, the elapsed time will be printed to cerr.

*/

FNGEOLIBUTIL_NAMESPACE_ENTER
{
    class Counter
    {
    public:
        FNKAT_DEPRECATED Counter(const std::string& name)
            : _name(name), _accum(0), _count(0), _ostr(&std::cerr) {}
        FNKAT_DEPRECATED virtual ~Counter() {print();}

        FNKAT_DEPRECATED const Counter& operator += (double amt) {_accum += amt; ++_count; return *this;}

        FNKAT_DEPRECATED void setOutputStream(std::ostream &ostr) {_ostr = &ostr;}

        FNKAT_DEPRECATED void print() const
        {
            (*_ostr) << _name << " - total: ";
            if(_accum < 1000.0)
            {
                (*_ostr) << _accum << " ms";
            }
            else if(_accum < (1000.0*60))
            {
                (*_ostr) << _accum/1000.0 << " s";
            }
            else
            {
                (*_ostr) << _accum/(1000.0*60) << " min";
            }

            (*_ostr) << ", count: " << _count;

            if (_count > 0)
            {
                (*_ostr) << ", ms/count: " << _accum / (double)_count;
            }
            (*_ostr) << std::endl;
        }

    private:
        std::string _name;
        double _accum;
        unsigned long _count;
        std::ostream *_ostr;
    };


    class Timer
    {
    public:
        FNKAT_DEPRECATED Timer(const std::string& name="Unnamed", Counter *accum = NULL, bool print = true) : _ostr(&std::cerr)
        {
            _name = name;
            _accum = accum;
            _print = print;
            reset();
        }

        FNKAT_DEPRECATED void reset()
        {
            gettimeofday(&_start,0);
        }

        FNKAT_DEPRECATED double getElapsed() const
        {
            timeval end;
            gettimeofday(&end,0);
            return (double)(end.tv_usec - _start.tv_usec + 1000000*(end.tv_sec-_start.tv_sec))/1000.0;
        }

        FNKAT_DEPRECATED void setOutputStream(std::ostream &ostr) {_ostr = &ostr;}

        FNKAT_DEPRECATED ~Timer()
        {
            double ms = getElapsed();
            if (_print && !_accum)
            {
                (*_ostr) << _name << " - ";
                if(ms< 1000.0)
                {
                    (*_ostr) << ms << " ms";
                }
                else if(ms < (1000.0*60))
                {
                    (*_ostr) << ms/1000.0 << " s";
                }
                else
                {
                    (*_ostr) << ms/(1000.0*60) << " min";
                }

                (*_ostr) << std::endl;
            }
            if (_accum) *_accum += ms;
        }

    private:
        Timer (const Timer&);
        Timer& operator= (const Timer&);
        timeval _start;
        std::string _name;
        Counter *_accum;
        bool _print;
        std::ostream *_ostr;
    };

    class MillisecondFormator
    {
        float _timeInMillisconds;
        public :
            FNKAT_DEPRECATED MillisecondFormator(float timeInMillisconds) : _timeInMillisconds(timeInMillisconds){}
            FNKAT_DEPRECATED friend std::ostream& operator <<(std::ostream &os,const MillisecondFormator &formator)
            {
                if (formator._timeInMillisconds > 1000.0f)
                {
                    os << formator._timeInMillisconds/1000.0f << "s";
                } else {
                    os << formator._timeInMillisconds << "ms";
                }
                return os;
            }


    };
}
FNGEOLIBUTIL_NAMESPACE_EXIT

#endif // INCLUDED_FNGEOLIBUTIL_TIMER_H
