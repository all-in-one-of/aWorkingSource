// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef PRODUCERPACKET_H
#define PRODUCERPACKET_H

#include <map>
#include <ptr/shared_ptr.h>
#include <ri.h>

namespace ptr = SPI_ptr_v4;

namespace PRManProcedural 
{
    enum ProducerType
    {
        kStandard = 0,
        kArchive,
        kObjectInstance,
        kKatanaInstance
    };

    // This is the data structure passed down for subdivide calls
    // Typically, only the scenegraphLocation is filled in and
    // the producer is left null.  However, in particular circumstances
    // (such as during a forceExpand circumstance) the location is scenegraphLocation blank
    // and the producer is filled in

    struct ProducerPacket
    {
        // This specifies the scenegraph location this producer packet is a reference to.
        // Storing the scenegraphLocation, rather than the producer, is often more memory
        // efficient (thus often preferable)

        std::string scenegraphLocation;

        // This specifies the producer to the current render location.
        // If the included producer and scenegraph location (string)
        // disagree (such as during instancing), the producer is considered
        // correct.
        
        FnKat::FnScenegraphIterator sgIterator;

        // Allows custom data to be attached to a producer packet.
        // This is used to specify the scene as a native RiArchive or an instance.
        // Archives and instances take precendece over other forms of
        // scene passing, such as the producer or scenegraphLocation

        RtPointer producerData;

        // Specifies the producer type so that the plugin can handle
        // special cases (such as archives or instances) properly.
        // See the ProducerType enum for the currently supported types.

        ProducerType producerType;

        // When set, this directs an immediate recursion through the entire
        // procedural, even if bound attrs are discovered. (When this is not
        // set, bound attrs will result in a delayed expansion based on the
        // bounds)
        
        bool fullRecurse : 1;
        
        // This disables the 'magic' before RiProcedural calls,
        // where the visibility settings are stored in user attrs, set to all on,
        // then restored after the subdivide.  This 'magic' is not always good,
        // cause it can hamper performance, and adds about 2K memory / procedural
        // calls.
        
        bool enableStrictVisibilityTesting : 1;
        
        // When set, this producer packet will never be deleted by PRManProcedural::Free.
        // This is often useful when re-using producer packets. (alternatively,
        // ProducerPacket reference counting could be implemented).
        
        bool neverDelete : 1;
        
        // This is set to true whenever there are no bounds or forceExpand is on
        // It lets us know NOT to try to restore visibility and membership
        bool fromImmediateExpansion : 1;
        
        bool ribDump : 1;
        bool groupHasBounds : 1;

        bool traverseFromFirstChild : 1;

        typedef std::map<std::string, bool> GroupingMap;
        typedef ptr::shared_ptr<GroupingMap> GroupingMapRcPtr;

        GroupingMapRcPtr grouping;
        void copyGroupingFromParent(
                GroupingMapRcPtr parentGrouping, FnKat::FnScenegraphIterator useIterator);
        
        ProducerPacket():
            scenegraphLocation(""),
            sgIterator(FnKat::FnScenegraphIterator()),
            producerData(0),
            producerType(kStandard),
            fullRecurse(false),
            enableStrictVisibilityTesting(false),
            neverDelete(false),
            fromImmediateExpansion(false),
            ribDump(false),
            groupHasBounds(true),
            traverseFromFirstChild(false),
            grouping()
        {}
        
        ~ProducerPacket()
        {
        }
    };
}
#endif
