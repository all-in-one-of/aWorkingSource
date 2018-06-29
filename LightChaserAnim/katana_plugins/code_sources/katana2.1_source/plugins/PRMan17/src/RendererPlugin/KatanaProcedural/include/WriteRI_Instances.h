// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef WRITERI_INSTANCES_H
#define WRITERI_INSTANCES_H

#include <FnScenegraphIterator/FnScenegraphIterator.h>
#include <PRManPluginState.h>
#include <ProducerPacket.h>

namespace PRManProcedural
{
    typedef std::map<std::string, ProducerPacket*> ProducerPacketMap;

    struct InstanceInfo
    {
        std::string instanceType;
        std::string instanceID;
        bool isSource;
    };

    bool WriteRI_CheckInstance(FnKat::FnScenegraphIterator& sgIterator, InstanceInfo& info);

    void WriteRI_InstanceSource(const ProducerPacket& producerPacket, const InstanceInfo& info,
                                PRManPluginState* sharedState);

    void WriteRI_InstanceElement(const ProducerPacket& producerPacket, const InstanceInfo& info,
                                 PRManPluginState* sharedState);

}

#endif // WRITERI_INSTANCES_H
