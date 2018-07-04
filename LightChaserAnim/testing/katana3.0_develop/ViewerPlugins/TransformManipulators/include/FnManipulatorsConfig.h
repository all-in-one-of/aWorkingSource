#ifndef TRANSFORM_MANIPULATORS_FNMANIPULATORSCONFIG_H
#define TRANSFORM_MANIPULATORS_FNMANIPULATORSCONFIG_H

#include <FnViewer/plugin/FnMathTypes.h>

// Used to define alternative (and yet similar) colors.
#define ALTERNATIVE_COLOR_FACTOR 0.75f

namespace FnManipulators
{
    using Foundry::Katana::ViewerAPI::Vec4f;

    static const float GuideFactor = 0.65f;

    static const Vec4f HandleColorX(0.88f, 0.f, 0.11f, 1.0f);
    static const Vec4f HandleColorY(0.f, 0.84f, 0.39f, 1.0f);
    static const Vec4f HandleColorZ(0.29f, 0.56f, 0.89f, 1.0f);

    static const Vec4f HandleColorXY = HandleColorX;
    static const Vec4f HandleColorXZ = HandleColorZ;
    static const Vec4f HandleColorYZ = HandleColorY;

    static const Vec4f HandleColorCentre(1.f, 1.f, 1.f, 0.5f);
    static const Vec4f HandleColorCentreOpaque(1.f, 1.f, 1.f, 0.5f);

    static const Vec4f HandleAlternativeColorX(
        HandleColorX.x * ALTERNATIVE_COLOR_FACTOR,
        HandleColorX.y * ALTERNATIVE_COLOR_FACTOR,
        HandleColorX.z * ALTERNATIVE_COLOR_FACTOR,
        HandleColorX.w);
    static const Vec4f HandleAlternativeColorY(
        HandleColorY.x * ALTERNATIVE_COLOR_FACTOR,
        HandleColorY.y * ALTERNATIVE_COLOR_FACTOR,
        HandleColorY.z * ALTERNATIVE_COLOR_FACTOR,
        HandleColorY.w);
    static const Vec4f HandleAlternativeColorZ(
        HandleColorZ.x * ALTERNATIVE_COLOR_FACTOR,
        HandleColorZ.y * ALTERNATIVE_COLOR_FACTOR,
        HandleColorZ.z * ALTERNATIVE_COLOR_FACTOR,
        HandleColorZ.w);

    static const Vec4f HandleAlternativeColorXY = HandleAlternativeColorX;
    static const Vec4f HandleAlternativeColorXZ = HandleAlternativeColorZ;
    static const Vec4f HandleAlternativeColorYZ = HandleAlternativeColorY;

    static const Vec4f HandleAlternativeColorCentre(
        HandleColorCentre.x * ALTERNATIVE_COLOR_FACTOR,
        HandleColorCentre.y * ALTERNATIVE_COLOR_FACTOR,
        HandleColorCentre.z * ALTERNATIVE_COLOR_FACTOR,
        HandleColorCentre.w);
    static const Vec4f HandleAlternativeColorCentreOpaque(
        HandleColorCentreOpaque.x * ALTERNATIVE_COLOR_FACTOR,
        HandleColorCentreOpaque.y * ALTERNATIVE_COLOR_FACTOR,
        HandleColorCentreOpaque.z * ALTERNATIVE_COLOR_FACTOR,
        HandleColorCentreOpaque.w);

    static const Vec4f HandleColorActive(0.92f, 0.72f, 0.16f, 1.0f);
    static const Vec4f HandleColorHover(0.95f, 0.83f, 0.49f, 1.0f);
    static const Vec4f HandleColorDrag(0.74f, 0.6f, 0.2f, 1.0f);
    static const Vec4f HandleColorNonInteractive(0.5f, 0.5f, 0.5f, 0.8f);

    static const float HandleTrunkLength = 1.55f;
    static const float HandleTrunkRadius = 0.016f;
    static const float HandlePlaneThickness = HandleTrunkRadius * 2.0f;

    static const float TranslateConeLength = 0.4f;
    static const float TranslateConeRadius = 0.118f;
    static const float TranslateSquareLength = 0.35f;
    static const float TranslatePlaneTriangleLength = 0.34f;
    static const float TranslatePlaneDistanceToOrigin = 0.6f;
    static const float TranslateCoiScale = 0.7f;
    static const float TranslateCoiPlaneDistanceToOrigin = 1.1f;

    static const float ScaleCubeLength = 0.236f;
    static const float ScaleUniformCubeLength = 0.3f;
    static const float ScalePlaneSquareLength = 0.3f;
    static const float ScalePlaneDistanceToOrigin = 0.6f;

    static const float RotateAroundCoiRadiusScale = 0.55f;
    static const float RotateViewRadius = 1.70f;
    static const float RotateSliceRadiusScale = 0.94f;
    static const int RotateSliceLineWidth = 1;
    static const Vec4f RotateSliceColor(1, 1, 1, 0.3f);
    static const Vec4f RotateSliceOutlineColor(1, 1, 1, 0.6f);
    static const Vec4f RotateBallColor(0.29f, 0.56f, 0.89f, 0.20f);
    static const Vec4f RotateBallAlternativeColor(
        RotateBallColor.x * ALTERNATIVE_COLOR_FACTOR,
        RotateBallColor.y * ALTERNATIVE_COLOR_FACTOR,
        RotateBallColor.z * ALTERNATIVE_COLOR_FACTOR,
        RotateBallColor.w);

    static const int CoiLinePattern = 0x00FF00FF;
    static const float CoiLinePatternFactor = 0.6f;
    static const unsigned int CoiLineWidth = 2;
    static const Vec4f CoiLineColor(
        HandleColorZ.x * GuideFactor,
        HandleColorZ.y * GuideFactor,
        HandleColorZ.z * GuideFactor,
        1.0f);
    static const unsigned int CoiCrossLineWidth = 1;
    static const float CoiCrossLineLength = 0.1f;
    static const Vec4f CoiCrossColor(1.0f, 1.0f, 1.0f, 1.0f);

}  // namespace FnManipulators

#endif  // TRANSFORM_MANIPULATORS_FNMANIPULATORSCONFIG_H
