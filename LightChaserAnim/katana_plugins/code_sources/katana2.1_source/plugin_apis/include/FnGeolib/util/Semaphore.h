// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef INCLUDED_FNGEOLIBUTIL_SEMAPHORE_H
#define INCLUDED_FNGEOLIBUTIL_SEMAPHORE_H

#include <cassert>
#include <stdexcept>
#include <semaphore.h>

#include "ns.h"
#include "Platform.h"

// Define this to get a bit more debug info dumped to std::cerr
// #define DEBUG_SEMAPHORE

// TODO: If we want to follow the model provided by boost interprocess, then we should
// define a set of exception classes that can be thrown when unexpected POSIX
// error flags are set. E.g. EAGAIN is fine for a try_wait, but EINVAL is not.

FNGEOLIBUTIL_NAMESPACE_ENTER
{

#ifdef WINDOWS
#error "FnGeolibUtil Semaphore not yet implemented in Windows"

#else
    // assume linux/unix/posix

    /**
     * @brief Class that represents a posix semaphore.
     *
     * It provides basic features wrapping posix semaphore calls around a class.
     */
    class Semaphore
    {
    public:

        Semaphore(unsigned int initialCount)
        {
            if (sem_init(&_semaphore, 1, initialCount) != 0)
            {
                std::cerr << "Semaphore::Semaphore error initialising semaphore"
                          << std::endl;
                throw std::runtime_error("Failed to initialise semaphore");
            }
#ifdef DEBUG_SEMAPHORE
            std::cerr << "Semaphore::Semaphore construction count is " << get_count()
                      << std::endl;
#endif
        }

        ~Semaphore()
        {
            if (sem_destroy(&_semaphore) != 0)
            {
                std::cerr << "Semaphore::~Semaphore() failed to destroy semaphore.";
                assert(false);
            }
        }

        int get_count() const
        {
            int count = 0;
            if (sem_getvalue(&_semaphore, &count) != 0)
            {
                std::cerr << "Semaphore::get_count() failed " << std::endl;
            }
            return count;
        }

        void post()
        {
#ifdef DEBUG_SEMAPHORE
            std::cerr << "Semaphore::post() old sem count was " << get_count() << std::endl;
#endif
            if (sem_post(&_semaphore) != 0)
            {
                std::cerr << "Semaphore::post() failed." << std::endl;
            }
#ifdef DEBUG_SEMAPHORE
            std::cerr << "Semaphore::post() new sem count is " << get_count() << std::endl;
#endif
        }

        bool wait()
        {
            // Like posix semaphore, you have to check the
            // return value, and thereafter the errno.
            // It's perfectly valid for sem_wait to be
            // woken up by an interrupt (EINTR).
#ifdef DEBUG_SEMAPHORE
            std::cerr << "Semaphore::wait() current sem count is " << get_count() << std::endl;
#endif
            if (sem_wait(&_semaphore) != 0)
            {
                std::cerr << "Semaphore::wait() failed." << std::endl;
                return false;
            }
            return true;
        }

        bool try_wait()
        {
#ifdef DEBUG_SEMAPHORE
            std::cerr << "Semaphore::try_wait() current sem count is " << get_count() << std::endl;
#endif
            const int result = sem_trywait(&_semaphore);
            return result == 0;
        }

    private:
        Semaphore();
        Semaphore(const Semaphore& semaphore);
        Semaphore &operator= (const Semaphore & semaphore);

        mutable sem_t _semaphore;
    };

#endif // WINDOWS
}
FNGEOLIBUTIL_NAMESPACE_EXIT

#endif // INCLUDED_FNGEOLIBUTIL_SEMAPHORE_H
