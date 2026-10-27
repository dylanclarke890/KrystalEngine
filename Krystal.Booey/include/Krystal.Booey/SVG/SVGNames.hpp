#pragma once

#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::svg
{
  /// @brief The names of SVG's attributes, in the null namespace.
  /// @note Only the attributes a ported element or mixin uses are here.
  struct SVGNames
  {
    /// @brief The `class` attribute, shared with HTML.
    KRYS_NODISCARD static const dom::QualifiedName &ClassAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &ClipPathUnitsAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &ClipRuleAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &CxAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &CyAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &DAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &DxAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &DyAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &FrAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &FxAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &FyAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &GradientTransformAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &GradientUnitsAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &HeightAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &HrefAttr() noexcept;
    /// @brief The `id` attribute, shared with HTML.
    KRYS_NODISCARD static const dom::QualifiedName &IdAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &MarkerHeightAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &MarkerUnitsAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &MarkerWidthAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &MaskContentUnitsAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &MaskUnitsAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &LengthAdjustAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &MediaAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &MethodAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &OffsetAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &OrientAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &PathLengthAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &PatternContentUnitsAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &PatternTransformAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &PatternUnitsAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &PointsAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &PreserveAspectRatioAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &RAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &RefXAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &RefYAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &RelAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &RequiredExtensionsAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &RotateAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &RxAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &RyAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &SpacingAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &SpreadMethodAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &StartOffsetAttr() noexcept;
    /// @brief The `style` attribute, shared with HTML.
    KRYS_NODISCARD static const dom::QualifiedName &StyleAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &SystemLanguageAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &TargetAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &TextLengthAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &TitleAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &TransformAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &TypeAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &ViewBoxAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &WidthAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &X1Attr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &X2Attr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &XAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &Y1Attr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &Y2Attr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &YAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &ZoomAndPanAttr() noexcept;

    // Filters
    KRYS_NODISCARD static const dom::QualifiedName &AmplitudeAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &AzimuthAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &BaseFrequencyAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &BiasAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &CrossoriginAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &DiffuseConstantAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &DivisorAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &EdgeModeAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &ElevationAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &ExponentAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &FilterUnitsAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &FloodColorAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &FloodOpacityAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &In2Attr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &InAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &InterceptAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &K1Attr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &K2Attr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &K3Attr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &K4Attr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &KernelMatrixAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &KernelUnitLengthAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &LightingColorAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &LimitingConeAngleAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &ModeAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &NumOctavesAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &OperatorAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &OrderAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &PointsAtXAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &PointsAtYAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &PointsAtZAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &PreserveAlphaAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &PrimitiveUnitsAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &RadiusAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &ResultAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &ScaleAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &SeedAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &SlopeAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &SpecularConstantAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &SpecularExponentAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &StdDeviationAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &StitchTilesAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &SurfaceScaleAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &TableValuesAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &TargetXAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &TargetYAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &ValuesAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &XChannelSelectorAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &YChannelSelectorAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &ZAttr() noexcept;
  };

  /// @brief The names of the XLink attributes SVG uses, in the XLink namespace with the `xlink` prefix.
  struct XLinkNames
  {
    KRYS_NODISCARD static const dom::QualifiedName &HrefAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &ShowAttr() noexcept;
    KRYS_NODISCARD static const dom::QualifiedName &TitleAttr() noexcept;
  };
}
