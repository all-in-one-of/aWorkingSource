#include <FnGeolibServices/FnBuiltInOpArgsUtil.h>

#include <FnPluginManager/FnPluginManager.h>

#include <stdexcept>

FNGEOLIBSERVICES_NAMESPACE_ENTER
{

    //***** SceneCreateOpArgsBuilder *****//

    StaticSceneCreateOpArgsBuilder::StaticSceneCreateOpArgsBuilder(
            bool pathsAreAbsolute)
    : m_handle(0x0)

    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();
        if (!suite)
        {
            throw std::runtime_error(
                    "no FnBuiltInOpArgsUtilHostSuite_v1 registered");
        }

        m_handle = suite->staticSceneCreateOpArgsBuilderCtor(pathsAreAbsolute);
    }

    StaticSceneCreateOpArgsBuilder::~StaticSceneCreateOpArgsBuilder()
    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();

        suite->releaseStaticSceneCreateOpArgsBuilder(m_handle);
        m_handle = 0x0;
    }

    FnAttribute::GroupAttribute StaticSceneCreateOpArgsBuilder::build()
    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();

        FnAttributeHandle attrHandle =
                suite->staticSceneCreateOpArgsBuilderBuild(m_handle);

        return FnAttribute::Attribute::CreateAndSteal(attrHandle);
    }

    void StaticSceneCreateOpArgsBuilder::createEmptyLocation(
            const std::string & locationPath, const std::string & locationType)
    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();

        suite->staticSceneCreateOpArgsBuilderCreateEmptyLocation(m_handle,
                locationPath.c_str(), locationType.c_str());

    }


    void StaticSceneCreateOpArgsBuilder::setAttrAtLocation(
            const std::string & locationPath, const std::string & attrName,
                    FnAttribute::Attribute attr)
    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();

        suite->staticSceneCreateOpArgsBuilderSetAttrAtLocation(m_handle,
                locationPath.c_str(), attrName.c_str(), attr.getHandle());
    }

    void StaticSceneCreateOpArgsBuilder::setAttrsAtLeafStateForLocation(
            const std::string & locationPath, bool state)
    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();

        suite->staticSceneCreateOpArgsBuilderSetAttrsAtLeafStateForLocation(
                m_handle, locationPath.c_str(), state);

    }

    void StaticSceneCreateOpArgsBuilder::addSubOpAtLocation(
            const std::string & locationPath, const std::string & opType,
                    FnAttribute::GroupAttribute opArgs)
    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();

        suite->staticSceneCreateOpArgsBuilderAddSubOpAtLocation(m_handle,
                locationPath.c_str(), opType.c_str(), opArgs.getHandle());
    }

    void StaticSceneCreateOpArgsBuilder::skipLocalActionsIfInputExistsAtLocation(
            const std::string & locationPath, bool state)
    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();

        suite->staticSceneCreateOpArgsBuilderSkipLocalActionsIfInputExistsAtLocation(
                m_handle, locationPath.c_str(), state);

    }

    const FnBuiltInOpArgsUtilHostSuite_v1 *
    StaticSceneCreateOpArgsBuilder::_getSuite()
    {
        static const FnBuiltInOpArgsUtilHostSuite_v1 * suite = 0x0;

        if (!suite)
        {
            suite = reinterpret_cast<const FnBuiltInOpArgsUtilHostSuite_v1 *>(
                    FnPluginManager::PluginManager::getHostSuite(
                            "BuiltInOpArgsUtilHost", 1));
        }

        return suite;
    }


    //***** AttributeSetOpArgsBuilder *****//

    AttributeSetOpArgsBuilder::AttributeSetOpArgsBuilder()
    : m_handle(0x0)

    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();
        if (!suite)
        {
            throw std::runtime_error(
                    "no FnBuiltInOpArgsUtilHostSuite_v1 registered");
        }

        m_handle = suite->attributeSetOpArgsBuilderCtor();
    }

    AttributeSetOpArgsBuilder::~AttributeSetOpArgsBuilder()
    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();

        suite->releaseAttributeSetOpArgsBuilder(m_handle);
        m_handle = 0x0;
    }

    FnAttribute::GroupAttribute AttributeSetOpArgsBuilder::build()
    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();

        FnAttributeHandle attrHandle =
                suite->attributeSetOpArgsBuilderBuild(m_handle);

        return FnAttribute::Attribute::CreateAndSteal(attrHandle);
    }

    void AttributeSetOpArgsBuilder::setLocationPaths(
            const FnAttribute::StringAttribute & locationPathsAttr,
            const std::string & batch )
    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();

        suite->attributeSetOpArgsBuilderSetLocationPaths(m_handle,
                locationPathsAttr.getHandle(), batch.c_str());
    }

    void AttributeSetOpArgsBuilder::setLocationPaths(
                const std::vector<std::string> & locationPaths,
                const std::string & batch )
    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();

        FnAttribute::StringAttribute locationPathsAttr(locationPaths);

        suite->attributeSetOpArgsBuilderSetLocationPaths(m_handle,
                locationPathsAttr.getHandle(), batch.c_str());
    }

    void AttributeSetOpArgsBuilder::setCEL(
                    const FnAttribute::StringAttribute & celAttr,
                    const std::string & batch )
    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();

        suite->attributeSetOpArgsBuilderSetCEL(m_handle,
                celAttr.getHandle(), batch.c_str());
    }

    void AttributeSetOpArgsBuilder::setCEL(
            const std::vector<std::string> &  cel,
            const std::string & batch )
    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();

        FnAttribute::StringAttribute celAttr(cel);

        suite->attributeSetOpArgsBuilderSetCEL(m_handle,
                celAttr.getHandle(), batch.c_str());
    }

    void AttributeSetOpArgsBuilder::setAttr(
            const std::string & attrName,
            const FnAttribute::Attribute & attr,
            const std::string & batch,
            const bool inherit )
    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();

        suite->attributeSetOpArgsBuilderSetAttr(m_handle,
                attrName.c_str(), attr.getHandle(), batch.c_str(), inherit);
    }

    void AttributeSetOpArgsBuilder::deleteAttr(
            const std::string & attrName,
            const std::string & batch )
    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();

        suite->attributeSetOpArgsBuilderDeleteAttr(m_handle,
                attrName.c_str(), batch.c_str());
    }

    void AttributeSetOpArgsBuilder::addSubOp( const std::string & opType,
                    const FnAttribute::GroupAttribute & opArgs,
                    const std::string & batch )
    {
        const FnBuiltInOpArgsUtilHostSuite_v1 * suite = _getSuite();

        suite->attributeSetOpArgsBuilderAddSubOp(m_handle,
                opType.c_str(), opArgs.getHandle(), batch.c_str());
    }

    const FnBuiltInOpArgsUtilHostSuite_v1 *
    AttributeSetOpArgsBuilder::_getSuite()
    {
        static const FnBuiltInOpArgsUtilHostSuite_v1 * suite = 0x0;

        if (!suite)
        {
            suite = reinterpret_cast<const FnBuiltInOpArgsUtilHostSuite_v1 *>(
                    FnPluginManager::PluginManager::getHostSuite(
                            "BuiltInOpArgsUtilHost", 1));
        }

        return suite;
    }

}
FNGEOLIBSERVICES_NAMESPACE_EXIT
