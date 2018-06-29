// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDER_COPYANDCONVERTRENDERACTION_H
#define FNRENDER_COPYANDCONVERTRENDERACTION_H

#include "FnRender/plugin/CopyRenderAction.h"
#include "FnAttribute/FnGroupBuilder.h"
#include "FnAttribute/FnAttribute.h"
#include "FnRender/FnRenderAPI.h"

namespace Foundry
{
namespace Katana
{
namespace Render
{
    /**
     * \ingroup RenderAPI
     */

    /**
     * @brief A render action which renders to a temporary location and
     *        then performs a conversion based on the input parameters
     *        which include convert settings attributes.
     *        The converted file is then copied to the target location.
     *
     * \note
     * The temporary file is deleted unless the
     * KATANA_KEEP_TEMP_RENDER_FILES environment variable is set.
     */
    class FNRENDER_API CopyAndConvertRenderAction : public CopyRenderAction
    {
    public:
        /**
         * @param renderTargetLocation The file location which is read and
         *                             displayed in the monitor.
         * @param tempRenderLocation   The file location which is rendered
         *                             to and then copied to the target
         *                             location.
         * @param clampOutput          Post-render, clamp negative rgb values
         *                             to 0, and clamp alpha values to 0-1.
         * @param colorConvert         Post-render, convert rendered image
         *                             data from linear to output colorspace
         *                             specified in the filename.
         * @param computeStats         Specifies whether and how to compute image
         *                             statistics as a post process, appending as
         *                             exr metadata. The default value is 'None'.
         * @param convertSettings      exr conversion settings which include
         *                             compression, bit depth, type, and
         *                             an optimisation flag.
         */
        CopyAndConvertRenderAction(const std::string& renderTargetLocation,
                                   const std::string& tempRenderLocation,
                                   bool clampOutput,
                                   bool colorConvert,
                                   const std::string & computeStats,
                                   const RenderSettings::AttributeSettings& convertSettings);

        virtual ~CopyAndConvertRenderAction() {}

        virtual void buildAttribute(FnAttribute::GroupBuilder& builder) const;

        void setAdditionalExrAttrs(const RenderSettings::AttributeSettings& additionalExrAttrs);

        void setOffsetForOverscan(bool value);

    protected:
        bool                              _clampOutput;
        bool                              _colorConvert;
        std::string                       _computeStats;
        RenderSettings::AttributeSettings _convertSettings;
        RenderSettings::AttributeSettings _additionalExrAttrs;
        bool                              _offsetForOverscan;
    };

    /**
     * @}
     */
}
}
}

namespace FnKat = Foundry::Katana;

#endif
