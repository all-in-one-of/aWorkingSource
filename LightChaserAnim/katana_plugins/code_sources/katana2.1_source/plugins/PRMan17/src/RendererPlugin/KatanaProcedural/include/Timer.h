// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef INCLUDED_RIPYTHON_TIMER_H
#define INCLUDED_RIPYTHON_TIMER_H

#include <iostream>
#include <sys/time.h>
#include <string>

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

namespace UTIL
{
    // ***********************************************************************
    // Deprecation Warning
    // These classes should no longer be used and will soon be deprecated.
    //
    // Consider using Renderman Rix functionality rather than these.
    class Counter
    {
    public:
        Counter(const std::string& name, std::ostream &ostr = std::cerr)
            : name_(name), accum_(0), count_(0), ostr_(&ostr), counterMutex_() {}
        virtual ~Counter() {print();}

        const Counter& operator += (double amt)
        {
            boost::recursive_mutex::scoped_lock lock(counterMutex_);
            accum_ += amt;
            ++count_;
            return *this;
        }

        void setOutputStream(std::ostream &ostr) {ostr_ = &ostr;}


    private:
        std::string name_;
        double accum_;
        unsigned long count_;
        std::ostream *ostr_;
        boost::recursive_mutex counterMutex_;

        // print() is only called from the destructor which makes it inherintly
        // thread safe and therefore we're not protecting it with a mutex.
        void print()
        {
            (*ostr_) << name_ << " - total: ";
            if(accum_ < 1000.0)
            {
                (*ostr_) << accum_ << " ms";
            }
            else if(accum_ < (1000.0*60))
            {
                (*ostr_) << accum_/1000.0 << " s";
            }
            else
            {
                (*ostr_) << accum_/(1000.0*60) << " min";
            }
            
            (*ostr_) << ", count: " << count_;
            
            if (count_ > 0)
            {
                (*ostr_) << ", ms/count: " << accum_ / count_;
            }
            (*ostr_) << std::endl;
        }
    };

    
    class Timer
    {
    public:
        Timer(std::string name="Unnamed", Counter *accum = NULL, bool print = true, std::ostream &ostr = std::cerr) : ostr_(&ostr)
        {
            name_ = name;
            accum_ = accum;
            print_ = print;
            reset();
        }

        void setOutputStream(std::ostream &ostr)
        {
            ostr_ = &ostr;
        }

        ~Timer()
        {
            double ms = get_elapsed();
            if (print_ && !accum_)
            {
                (*ostr_) << name_ << " - ";
                if(ms< 1000.0)
                {
                    (*ostr_) << ms << " ms";
                }
                else if(ms < (1000.0*60))
                {
                    (*ostr_) << ms/1000.0 << " s";
                }
                else
                {
                    (*ostr_) << ms/(1000.0*60) << " min";
                }
                
                (*ostr_) << std::endl;
            }
            if (accum_) *accum_ += ms;
        }

        void reset()
        {
            gettimeofday(&start_,0);
        }

    private:
        Timer (const Timer&); 
        Timer& operator= (const Timer&);
        timeval start_;
        std::string name_;
        Counter *accum_;
        bool print_;
        std::ostream *ostr_;

        float get_elapsed()
        {
            timeval end;
            gettimeofday(&end,0);
            return (end.tv_usec - start_.tv_usec + 1000000*end.tv_sec- 1000000*start_.tv_sec)/1000.0;
        }

    };

}

#endif
