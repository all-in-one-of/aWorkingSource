// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDERERINFO_RENDERMETHOD_H
#define FNRENDERERINFO_RENDERMETHOD_H

#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnAttribute.h>

namespace Foundry
{
namespace Katana
{
namespace RendererInfo
{
    /**
     * \ingroup RenderAPI
     */
    /**
     * @brief ...
     */
    class RenderMethod
    {
    public:
        RenderMethod(const std::string& name, const std::string& label);
        virtual ~RenderMethod() {}

        virtual std::string getType() const = 0;
        virtual void buildAttribute(FnAttribute::GroupBuilder& builder) const = 0;

        virtual void setName(const std::string& name);
        virtual std::string getName() const;
        virtual void setLabel(const std::string& label);
        virtual std::string getLabel() const;
        virtual void setCreateCatalogItem(bool createCatalogItem);
        virtual bool isCreateCatalogItem() const;
        virtual void setReportRenderMessages(bool reportRenderMessages);
        virtual bool isReportRenderMessages() const;
        virtual void setRegisterRender(bool registerRender);
        virtual bool isRegisterRender() const;
        virtual void setDebugOutputSupported(bool debugOutputSupported);
        virtual bool isDebugOutputSupported() const;
        virtual void setSceneGraphDebugOutputSupported(bool sceneGraphDebugOutputSupported);
        virtual bool isSceneGraphDebugOutputSupported() const;
        virtual void setDebugOutputFileType(const std::string& debugOutputFileType);
        virtual std::string getDebugOutputFileType() const;
        virtual void setVisible(bool visible);
        virtual bool isVisible() const;

    protected:
        std::string     _name;
        std::string     _label;
        std::string     _debugOutputFileType;
        bool            _createCatalogItem;
        bool            _reportRenderMessages;
        bool            _registerRender;
        bool            _debugOutputSupported;
        bool            _sceneGraphDebugOutputSupported;
        bool            _visible;
    };
    /**
     * @}
     */

    /**
     * \ingroup RenderAPI
     */
    /**
     * @brief ...
     */
    class DiskRenderMethod : public RenderMethod
    {
    public:
        DiskRenderMethod();
        DiskRenderMethod(const std::string& name, const std::string& label);
        virtual ~DiskRenderMethod() {}

        virtual std::string getType() const { return kType; }
        virtual void buildAttribute(FnAttribute::GroupBuilder& builder) const;

        void setAllowWaitingForRenderCompletion(bool allowWaitingForRenderCompletion);
        bool isAllowWaitingForRenderCompletion() const;

        static const char* kType;
        static const char* kDefaultLabel;
        static const char* kDefaultName;
        static const char* kBatchName;

    protected:

        bool _allowWaitingForRenderCompletion;
    };

    /**
     * \ingroup RenderAPI
     */
    /**
     * @brief ...
     */
    class PreviewRenderMethod : public RenderMethod
    {
    public:

        PreviewRenderMethod();
        PreviewRenderMethod(const std::string& name, const std::string& label, bool allowConcurrentRenders = false);
        virtual ~PreviewRenderMethod() {}

        virtual std::string getType() const { return kType; }
        virtual void buildAttribute(FnAttribute::GroupBuilder& builder) const;

        void setAllowConcurrentRenders(bool allowConcurrentRenders);
        bool isAllowConcurrentRenders() const;

        static const char* kType;
        static const char* kDefaultLabel;
        static const char* kDefaultName;

    protected:

        bool _allowConcurrentRenders;

    };
    /**
     * @}
     */

    /**
     * \ingroup RenderAPI
     */
    /**
     * @brief ...
     */
    class LiveRenderMethod : public RenderMethod
    {
    public:
        LiveRenderMethod();
        LiveRenderMethod(const std::string& name, const std::string& label);
        virtual ~LiveRenderMethod() {}

        virtual std::string getType() const { return kType; }
        virtual void buildAttribute(FnAttribute::GroupBuilder& builder) const;

        static const char* kType;
        static const char* kDefaultLabel;
        static const char* kDefaultName;
    };
    /**
     * @}
     */
}
}
}

namespace FnKat = Foundry::Katana;

#endif
