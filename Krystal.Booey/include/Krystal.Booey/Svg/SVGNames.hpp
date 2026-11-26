#pragma once

#include "Krystal.Booey/Dom/Types/QualifiedName.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief The names of SVG's attributes, in the null namespace.
  /// @note Only the attributes a ported element or mixin uses are here.
  struct SVGNames
  {
    /// @brief The `class` attribute, shared with HTML.
    KRYS_NODISCARD static const QualifiedName &AccumulateAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &AdditiveAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &AlignmentBaselineAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &AttributeNameAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &AttributeTypeAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &BaselineShiftAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &BeginAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ByAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &CalcModeAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ClassAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ClipPathAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ClipPathUnitsAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ClipRuleAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ColorAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ColorInterpolationAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ColorInterpolationFiltersAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &CursorAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &CxAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &CyAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &DAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &DisplayAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &DominantBaselineAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &DurAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &DxAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &DyAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &EndAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FillAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FillOpacityAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FillRuleAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FilterAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FontFamilyAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FontSizeAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FontStretchAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FontStyleAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FontVariantAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FontWeightAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FrAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FromAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FxAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FyAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &GradientTransformAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &GradientUnitsAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &HeightAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &HrefAttr() noexcept;

    /// @brief The `id` attribute, shared with HTML.
    KRYS_NODISCARD static const QualifiedName &IdAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ImageRenderingAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &KeyPointsAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &KeySplinesAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &KeyTimesAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &LetterSpacingAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MarkerEndAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MarkerHeightAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MarkerMidAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MarkerStartAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MarkerUnitsAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MarkerWidthAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MaskAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MaskContentUnitsAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MaskTypeAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MaskUnitsAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &LengthAdjustAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MaxAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MediaAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MethodAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &MinAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &OffsetAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &OpacityAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &OrientAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &OverflowAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &PaintOrderAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &PathAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &PathLengthAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &PatternContentUnitsAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &PatternTransformAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &PatternUnitsAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &PointerEventsAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &PointsAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &PreserveAspectRatioAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &RAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &RefXAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &RefYAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &RelAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &RepeatCountAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &RepeatDurAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &RequiredExtensionsAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &RestartAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &RotateAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &RxAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &RyAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ShapeRenderingAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &SpacingAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &SpreadMethodAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &StartOffsetAttr() noexcept;

    /// @brief The `style` attribute, shared with HTML.
    KRYS_NODISCARD static const QualifiedName &StopColorAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &StopOpacityAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &StrokeAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &StrokeDasharrayAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &StrokeDashoffsetAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &StrokeLinecapAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &StrokeLinejoinAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &StrokeMiterlimitAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &StrokeOpacityAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &StrokeWidthAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &StyleAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &SystemLanguageAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &TargetAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &TextAnchorAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &TextDecorationAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &TextLengthAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &TextRenderingAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &TitleAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ToAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &TransformAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &TypeAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &VectorEffectAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ViewBoxAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &VisibilityAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &WidthAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &WordSpacingAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &X1Attr() noexcept;

    KRYS_NODISCARD static const QualifiedName &X2Attr() noexcept;

    KRYS_NODISCARD static const QualifiedName &XAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &Y1Attr() noexcept;

    KRYS_NODISCARD static const QualifiedName &Y2Attr() noexcept;

    KRYS_NODISCARD static const QualifiedName &YAttr() noexcept;

    // Filters
    KRYS_NODISCARD static const QualifiedName &AmplitudeAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &AzimuthAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &BaseFrequencyAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &BiasAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &CrossoriginAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &DiffuseConstantAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &DivisorAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &EdgeModeAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ElevationAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ExponentAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FilterUnitsAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FloodColorAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &FloodOpacityAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &In2Attr() noexcept;

    KRYS_NODISCARD static const QualifiedName &InAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &InterceptAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &K1Attr() noexcept;

    KRYS_NODISCARD static const QualifiedName &K2Attr() noexcept;

    KRYS_NODISCARD static const QualifiedName &K3Attr() noexcept;

    KRYS_NODISCARD static const QualifiedName &K4Attr() noexcept;

    KRYS_NODISCARD static const QualifiedName &KernelMatrixAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &KernelUnitLengthAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &LightingColorAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &LimitingConeAngleAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ModeAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &NumOctavesAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &OperatorAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &OrderAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &PointsAtXAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &PointsAtYAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &PointsAtZAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &PreserveAlphaAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &PrimitiveUnitsAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &RadiusAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ResultAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ScaleAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &SeedAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &SlopeAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &SpecularConstantAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &SpecularExponentAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &StdDeviationAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &StitchTilesAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &SurfaceScaleAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &TableValuesAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &TargetXAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &TargetYAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ValuesAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &XChannelSelectorAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &YChannelSelectorAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ZAttr() noexcept;
  };
}

namespace krys::boo::svg
{
  /// @brief The names of the XLink attributes SVG uses, in the XLink namespace with the `xlink` prefix.
  struct XLinkNames
  {
    KRYS_NODISCARD static const QualifiedName &HrefAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &ShowAttr() noexcept;

    KRYS_NODISCARD static const QualifiedName &TitleAttr() noexcept;
  };
}
