#include <algorithm>
#include <unistd.h>
#include <iostream>
#include <time.h>
#include <cstdio>

#include <FnGeolib/util/PowerNap.h>

FNGEOLIBUTIL_NAMESPACE_ENTER
{
    PowerNap::PowerNap(long startingInterval, long maxInterval)
        : startingInterval_(startingInterval),
          maxInterval_(maxInterval),
          interval_(startingInterval)
    {
        maxInterval_ = std::min(999999999L, maxInterval);
    }

    PowerNap::~PowerNap() {}

    void PowerNap::tick()
    {
        // Double the interval, and clamp between 1 and max
        // Thus, if you set <=0, the next tick will be 1, then 2, etc.
        interval_ *= 2;
        interval_ = std::min( std::max(interval_, 1L), maxInterval_);

        struct timespec tim, tim2;
        tim.tv_sec = 0;
        tim.tv_nsec = interval_;

        if(nanosleep(&tim , &tim2)< 0 )
        {
            // Nano sleep system call failed / interrupted
        }
        else
        {
            // nano sleep successful
        }
    }

    void PowerNap::reset()
    {
        interval_ = startingInterval_;
    }
}
FNGEOLIBUTIL_NAMESPACE_EXIT

