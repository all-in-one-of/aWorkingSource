#ifndef FnGeolibServicesBuiltOpArgsUtil_H
#define FnGeolibServicesBuiltOpArgsUtil_H

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnAttribute/FnAttribute.h>

#include <FnGeolibServices/suite/FnBuiltInOpArgsUtilSuite.h>

#include "ns.h"

FNGEOLIBSERVICES_NAMESPACE_ENTER
{
    class StaticSceneCreateOpArgsBuilder
    {
    public:
        StaticSceneCreateOpArgsBuilder(bool pathsAreAbsolute);

        ~StaticSceneCreateOpArgsBuilder();

        FnAttribute::GroupAttribute build();

        void createEmptyLocation(const std::string & locationPath,
                const std::string & locationType = "");

        void setAttrAtLocation(const std::string & locationPath,
                const std::string & attrName, FnAttribute::Attribute attr);

        void setAttrsAtLeafStateForLocation(const std::string & locationPath,
                bool state);

        void addSubOpAtLocation(const std::string & locationPath,
                const std::string & opType, FnAttribute::GroupAttribute opArgs);

        void skipLocalActionsIfInputExistsAtLocation(
                const std::string & locationPath, bool state);


    private:
        // no copy/assign
        StaticSceneCreateOpArgsBuilder(const StaticSceneCreateOpArgsBuilder& rhs);
        StaticSceneCreateOpArgsBuilder& operator=(const StaticSceneCreateOpArgsBuilder& rhs);

        static const FnBuiltInOpArgsUtilHostSuite_v1 * _getSuite();

        StaticSceneCreateOpArgsBuilderHandle m_handle;
    };


    class AttributeSetOpArgsBuilder
    {
    public:
        AttributeSetOpArgsBuilder();

        ~AttributeSetOpArgsBuilder();

        FnAttribute::GroupAttribute build();

        void setLocationPaths(
                const FnAttribute::StringAttribute & locationPathsAttr,
                const std::string & batch="" );

        void setLocationPaths( const std::vector<std::string> & locationPaths,
                               const std::string & batch="" );

        void setCEL( const FnAttribute::StringAttribute & celAttr,
                     const std::string & batch="" );

        void setCEL( const std::vector<std::string> & cel,
                     const std::string & batch="" );

        void setAttr(  const std::string & attrName,
                       const FnAttribute::Attribute & attr,
                       const std::string & batch="",
                       const bool inherit=true );

        void deleteAttr( const std::string & attrName,
                         const std::string & batch="" );

        void addSubOp( const std::string & opType,
                       const FnAttribute::GroupAttribute & opArgs,
                       const std::string & batch="" );

    private:
        // no copy/assign
        AttributeSetOpArgsBuilder(const AttributeSetOpArgsBuilder& rhs);
        AttributeSetOpArgsBuilder& operator=(const AttributeSetOpArgsBuilder& rhs);

        static const FnBuiltInOpArgsUtilHostSuite_v1 * _getSuite();

        AttributeSetOpArgsBuilderHandle m_handle;
    };
}
FNGEOLIBSERVICES_NAMESPACE_EXIT




#endif
