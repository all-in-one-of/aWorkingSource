#ifndef INCLUDED_FNGEOLIBUTIL_POWERNAP_H
#define INCLUDED_FNGEOLIBUTIL_POWERNAP_H

#include "ns.h"

#include <unistd.h>

FNGEOLIBUTIL_NAMESPACE_ENTER
{
    class PowerNap
    {
    public:
        PowerNap(long startingInterval_nsec=500,
                 long maxInterval_nsec=(10000000)  // 0.01 sec
                 );
        ~PowerNap();

        void tick();
        void reset();

    private:
        long startingInterval_;
        long maxInterval_;
        long interval_;
    };
}
FNGEOLIBUTIL_NAMESPACE_EXIT

#endif // INCLUDED_FNGEOLIBUTIL_PATHUTILS_H
