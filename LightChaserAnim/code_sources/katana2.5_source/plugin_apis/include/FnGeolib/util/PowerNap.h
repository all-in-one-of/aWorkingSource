#ifndef INCLUDED_FNGEOLIBUTIL_POWERNAP_H
#define INCLUDED_FNGEOLIBUTIL_POWERNAP_H

#ifdef _WIN32
#error "FnGeolibUtil::PowerNap is not supported on Windows."
#endif

#include <unistd.h>

#include <FnGeolib/FnGeolibAPI.h>
#include <FnGeolib/util/ns.h>

#include <FnPlatform/internal/Portability.h>


FNGEOLIBUTIL_NAMESPACE_ENTER
{
    class FNGEOLIB_API PowerNap
    {
    public:
        FNKAT_DEPRECATED PowerNap(long startingInterval_nsec=500,
                 long maxInterval_nsec=(10000000)  // 0.01 sec
                 );
        FNKAT_DEPRECATED ~PowerNap();

        FNKAT_DEPRECATED void tick();
        FNKAT_DEPRECATED void reset();

    private:
        long startingInterval_;
        long maxInterval_;
        long interval_;
    };
}
FNGEOLIBUTIL_NAMESPACE_EXIT

#endif // INCLUDED_FNGEOLIBUTIL_PATHUTILS_H
