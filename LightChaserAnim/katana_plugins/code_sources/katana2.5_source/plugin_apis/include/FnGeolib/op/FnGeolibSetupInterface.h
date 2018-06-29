#ifndef FnGeolibSetupInterface_H
#define FnGeolibSetupInterface_H

#include <string>
#include <vector>

#include <FnAttribute/FnAttribute.h>

#include <FnGeolib/FnGeolibAPI.h>
#include <FnGeolib/op/ns.h>
#include <FnGeolib/suite/FnGeolibSetupInterfaceSuite.h>

#include <FnPluginSystem/FnPlugin.h>
#include <FnPluginSystem/FnPluginSystem.h>

//  This is the GeolibOp client side wrapper for FnGeolibInterfaceSuite_v1.  A
//  reference to an instance of this is handed to the GeolibOp::setup function.
//  This class is not meant to be derived from, just compiled into the Op plug-
//  in as a convenience.

FNGEOLIBOP_NAMESPACE_ENTER
{

    /**
     * @ingroup FnGeolibOp
     * @brief Provides functions to allow an Op to configure how it's scheduled
     *     and evaluated by the Runtime.
     */
    class FNGEOLIB_API GeolibSetupInterface
    {
    public:
        GeolibSetupInterface(FnGeolibSetupInterfaceHandle interfaceHandle,
            FnGeolibSetupInterfaceSuite_v1 *interfaceSuite);

        /**
         *  @typedef ThreadMode
         *  @brief By default an Op can potentially be evaluated by the Runtime
         *      in multiple threads concurrently. The \c ThreadMode enumeration
         *      provides a means to specify to the Runtime the concurrency
         *      requirements of an Op.
         *
         *  @par ThreadModeConcurrent
         *
         *  The Op can be evaluated concurrently with any other Op in the Op
         *  tree, In addition, the same op type can be evaluated by more than
         *  one thread concurrently. Ops which declare themselves \c
         *  ThreadModeConcurrent may be run at anytime, even during
         *  \c GlobalUnsafe execution.
         *
         *  Considerations for marking an Op as ThreadModeConcurrent:
         *      - Does the Op contain global static data? If yes, is it properly
         *        guarded against race conditions?
         *      - Does the Op make use of third party libraries and are these
         *        libraries thread-safe?
         *      - Does the Op pass or share the same private data instances
         *        amongst its children? If so, is that data structure also
         *        appropriately guarded for concurrent access and is its means
         *        of destruction thread-safe?
         *
         *  @par ThreadModeGlobalUnsafe
         *
         *   The Op will run in a single thread, only one \c GlobalUnsafe Op
         *   will be scheduled to run within the entire process at any one time
         *   even across multiple Runtime instantiations. Use this if the Op
         *   makes calls to unknown code, unknown libraries, or code known to be
         *   thread-unsafe.
         *
         *  @note \c GlobalUnsafe Ops do not prevent the concurrent execution of
         *   those explicitly white-listed as Concurrent.
         */
        typedef enum ThreadMode
        {
            ThreadModeConcurrent = kFnKatGeolibThreadModeConcurrent,
            ThreadModeGlobalUnsafe = kFnKatGeolibThreadModeGlobalUnsafe
        } ThreadMode;

        /**
         * @brief Sets the concurrency constraints the Runtime must adhere to
         *     when evaluating this Op.
         * @param threadmode The thread mode (see ThreadMode) appropriate for
         *     this Op.
         * @see ThreadMode
         */
        void setThreading(ThreadMode threadmode);

    private:

        /// @cond FN_INTERNAL_DEV

        FnGeolibSetupInterfaceHandle _handle;
        FnGeolibSetupInterfaceSuite_v1 *_suite;

        /// @endcond FN_INTERNAL_DEV
    };
}
FNGEOLIBOP_NAMESPACE_EXIT

#endif // FnGeolibSetupInterface_H
