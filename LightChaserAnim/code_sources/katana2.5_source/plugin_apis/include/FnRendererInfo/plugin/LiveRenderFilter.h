// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDERERINFO_LIVERENDERFILTER_H
#define FNRENDERERINFO_LIVERENDERFILTER_H

#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnAttribute.h>
#include <FnRendererInfo/FnRendererInfoAPI.h>

namespace Foundry
{
namespace Katana
{
namespace RendererInfo
{
    /**
     * \ingroup RenderAPI
     */

    enum SceneGraphTraversalMode
    {
        kSceneGraphTraversalMode_LocationOrigin,
        kSceneGraphTraversalMode_RecursiveFromSelection,
        kSceneGraphTraversalMode_SelectedLocations,
        kSceneGraphTraversalMode_SelectedLocationsRecursive
    };

    class FNRENDERERINFO_API LiveRenderFilter
    {
    public:
        LiveRenderFilter();
        LiveRenderFilter(const std::string& name,
                         const std::string& updateType,
                         const std::string& sgLocationType,
                         const std::string& sgLocationOrigin,
                         SceneGraphTraversalMode sgTraversalMode);

        virtual ~LiveRenderFilter() {};

        void buildAttribute(FnAttribute::GroupBuilder& builder, const std::string& prefix) const;
        void addAttribute(const std::string& attributeName);

    protected:
        std::string _name;
        std::string _updateType;
        std::string _sceneGraphLocationType;
        std::string _sceneGraphLocationOrigin;
        SceneGraphTraversalMode _sceneGraphTraversalMode;
        std::vector<std::string> _attributes;
    };

/**
 * @}
 */
}
}
}

namespace FnKat = Foundry::Katana;

#endif
