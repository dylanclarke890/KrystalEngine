#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

#define KRYS_DEFINE_SVG_ATTRIBUTE_NAME(Name, localName)                                                      \
  const dom::QualifiedName &SVGNames::Name() noexcept                                                        \
  {                                                                                                          \
    static NeverDestroyed<const dom::QualifiedName> name {dom::DOMStringAtom::Null(),                        \
                                                          dom::DOMStringAtom::Null(), localName};            \
    return name;                                                                                             \
  }

namespace krys::boo::svg
{
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(ClassAttr, u8"class")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(ClipPathUnitsAttr, u8"clipPathUnits")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(ClipRuleAttr, u8"clip-rule")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(CxAttr, u8"cx")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(CyAttr, u8"cy")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(DAttr, u8"d")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(DxAttr, u8"dx")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(DyAttr, u8"dy")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(FrAttr, u8"fr")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(FxAttr, u8"fx")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(FyAttr, u8"fy")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(GradientTransformAttr, u8"gradientTransform")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(GradientUnitsAttr, u8"gradientUnits")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(HeightAttr, u8"height")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(HrefAttr, u8"href")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(IdAttr, u8"id")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(MarkerHeightAttr, u8"markerHeight")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(MarkerUnitsAttr, u8"markerUnits")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(MarkerWidthAttr, u8"markerWidth")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(MaskContentUnitsAttr, u8"maskContentUnits")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(MaskUnitsAttr, u8"maskUnits")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(LengthAdjustAttr, u8"lengthAdjust")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(MediaAttr, u8"media")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(MethodAttr, u8"method")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(OffsetAttr, u8"offset")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(OrientAttr, u8"orient")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(PathLengthAttr, u8"pathLength")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(PatternContentUnitsAttr, u8"patternContentUnits")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(PatternTransformAttr, u8"patternTransform")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(PatternUnitsAttr, u8"patternUnits")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(PointsAttr, u8"points")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(PreserveAspectRatioAttr, u8"preserveAspectRatio")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(RAttr, u8"r")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(RefXAttr, u8"refX")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(RefYAttr, u8"refY")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(RelAttr, u8"rel")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(RequiredExtensionsAttr, u8"requiredExtensions")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(RotateAttr, u8"rotate")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(RxAttr, u8"rx")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(RyAttr, u8"ry")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(SpacingAttr, u8"spacing")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(SpreadMethodAttr, u8"spreadMethod")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(StartOffsetAttr, u8"startOffset")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(StyleAttr, u8"style")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(SystemLanguageAttr, u8"systemLanguage")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(TargetAttr, u8"target")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(TextLengthAttr, u8"textLength")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(TitleAttr, u8"title")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(TransformAttr, u8"transform")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(TypeAttr, u8"type")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(ViewBoxAttr, u8"viewBox")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(WidthAttr, u8"width")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(X1Attr, u8"x1")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(X2Attr, u8"x2")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(XAttr, u8"x")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(Y1Attr, u8"y1")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(Y2Attr, u8"y2")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(YAttr, u8"y")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(ZoomAndPanAttr, u8"zoomAndPan")

  // Filters
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(AmplitudeAttr, u8"amplitude")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(AzimuthAttr, u8"azimuth")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(BaseFrequencyAttr, u8"baseFrequency")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(BiasAttr, u8"bias")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(CrossoriginAttr, u8"crossorigin")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(DiffuseConstantAttr, u8"diffuseConstant")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(DivisorAttr, u8"divisor")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(EdgeModeAttr, u8"edgeMode")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(ElevationAttr, u8"elevation")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(ExponentAttr, u8"exponent")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(FilterUnitsAttr, u8"filterUnits")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(FloodColorAttr, u8"flood-color")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(FloodOpacityAttr, u8"flood-opacity")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(In2Attr, u8"in2")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(InAttr, u8"in")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(InterceptAttr, u8"intercept")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(K1Attr, u8"k1")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(K2Attr, u8"k2")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(K3Attr, u8"k3")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(K4Attr, u8"k4")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(KernelMatrixAttr, u8"kernelMatrix")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(KernelUnitLengthAttr, u8"kernelUnitLength")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(LightingColorAttr, u8"lighting-color")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(LimitingConeAngleAttr, u8"limitingConeAngle")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(ModeAttr, u8"mode")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(NumOctavesAttr, u8"numOctaves")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(OperatorAttr, u8"operator")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(OrderAttr, u8"order")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(PointsAtXAttr, u8"pointsAtX")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(PointsAtYAttr, u8"pointsAtY")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(PointsAtZAttr, u8"pointsAtZ")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(PreserveAlphaAttr, u8"preserveAlpha")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(PrimitiveUnitsAttr, u8"primitiveUnits")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(RadiusAttr, u8"radius")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(ResultAttr, u8"result")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(ScaleAttr, u8"scale")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(SeedAttr, u8"seed")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(SlopeAttr, u8"slope")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(SpecularConstantAttr, u8"specularConstant")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(SpecularExponentAttr, u8"specularExponent")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(StdDeviationAttr, u8"stdDeviation")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(StitchTilesAttr, u8"stitchTiles")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(SurfaceScaleAttr, u8"surfaceScale")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(TableValuesAttr, u8"tableValues")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(TargetXAttr, u8"targetX")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(TargetYAttr, u8"targetY")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(ValuesAttr, u8"values")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(XChannelSelectorAttr, u8"xChannelSelector")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(YChannelSelectorAttr, u8"yChannelSelector")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(ZAttr, u8"z")

#define KRYS_DEFINE_XLINK_ATTRIBUTE_NAME(Name, localName)                                                    \
  const dom::QualifiedName &XLinkNames::Name() noexcept                                                      \
  {                                                                                                          \
    static NeverDestroyed<const dom::QualifiedName> name {                                                   \
      dom::DOMStringAtom(infra::Namespaces::XLink), dom::DOMStringAtom(infra::NamespacePrefixes::XLink),     \
      dom::DOMStringAtom(localName)};                                                                        \
    return name;                                                                                             \
  }

  KRYS_DEFINE_XLINK_ATTRIBUTE_NAME(HrefAttr, u8"href")
  KRYS_DEFINE_XLINK_ATTRIBUTE_NAME(ShowAttr, u8"show")
  KRYS_DEFINE_XLINK_ATTRIBUTE_NAME(TitleAttr, u8"title")

#undef KRYS_DEFINE_XLINK_ATTRIBUTE_NAME
}

#undef KRYS_DEFINE_SVG_ATTRIBUTE_NAME
