#pragma once

#include "Krystal.Booey/Core/TextSpacing.hpp"
#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/Graphics/SystemFontDatabase.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/BuilderStateInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"
#include "Krystal.Booey/Style/FontSizeFunctions.hpp"
#include "Krystal.Booey/Style/ForVisitedLink.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Inline/TextEdgeCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/CoordinatedValueListValue.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include <cmath>

namespace krys::boo::style
{
#define KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(property)                                                      \
  static void ApplyInherit##property(BuilderState &) noexcept;                                               \
  static void ApplyInitial##property(BuilderState &) noexcept;                                               \
  static void ApplyValue##property(BuilderState &, css::Value &) noexcept

  template <typename T>
  KRYS_NODISCARD inline T ForwardInheritedValue(T &&value) noexcept
  {
    return std::forward<T>(value);
  }
  template <auto R, typename V>
  KRYS_NODISCARD inline Length<R, V> ForwardInheritedValue(const Length<R, V> &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline AccentColor ForwardInheritedValue(const AccentColor &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline AnchorNames ForwardInheritedValue(const AnchorNames &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline AppleColorFilter ForwardInheritedValue(const AppleColorFilter &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline AspectRatio ForwardInheritedValue(const AspectRatio &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline BackgroundSize ForwardInheritedValue(const BackgroundSize &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline BlockEllipsis ForwardInheritedValue(const BlockEllipsis &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline BlockStepSize ForwardInheritedValue(const BlockStepSize &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline BorderImageSource ForwardInheritedValue(const BorderImageSource &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline BorderImageSlice ForwardInheritedValue(const BorderImageSlice &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline BorderImageWidth ForwardInheritedValue(const BorderImageWidth &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline BorderImageOutset ForwardInheritedValue(const BorderImageOutset &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline BorderImageRepeat ForwardInheritedValue(const BorderImageRepeat &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline BorderRadiusValue ForwardInheritedValue(const BorderRadiusValue &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline BoxShadows ForwardInheritedValue(const BoxShadows &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ContainIntrinsicSize ForwardInheritedValue(const ContainIntrinsicSize &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ContainerNames ForwardInheritedValue(const ContainerNames &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline Content ForwardInheritedValue(const Content &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline boo::Color ForwardInheritedValue(const boo::Color &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline Color ForwardInheritedValue(const Color &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline EasingFunction ForwardInheritedValue(const EasingFunction &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline GapGutter ForwardInheritedValue(const GapGutter &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline FontFamilies ForwardInheritedValue(const FontFamilies &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline FilterOperations ForwardInheritedValue(const FilterOperations &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ScrollMarginEdge ForwardInheritedValue(const ScrollMarginEdge &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ScrollPaddingEdge ForwardInheritedValue(const ScrollPaddingEdge &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline MaskBorderSource ForwardInheritedValue(const MaskBorderSource &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline MaskBorderSlice ForwardInheritedValue(const MaskBorderSlice &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline MaskBorderWidth ForwardInheritedValue(const MaskBorderWidth &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline MaskBorderOutset ForwardInheritedValue(const MaskBorderOutset &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline MaskBorderRepeat ForwardInheritedValue(const MaskBorderRepeat &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline MarginEdge ForwardInheritedValue(const MarginEdge &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline PaddingEdge ForwardInheritedValue(const PaddingEdge &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ImageOrNone ForwardInheritedValue(const ImageOrNone &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline InsetEdge ForwardInheritedValue(const InsetEdge &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline Perspective ForwardInheritedValue(const Perspective &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline Quotes ForwardInheritedValue(const Quotes &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline Rotate ForwardInheritedValue(const Rotate &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline Scale ForwardInheritedValue(const Scale &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline Translate ForwardInheritedValue(const Translate &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline PreferredSize ForwardInheritedValue(const PreferredSize &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline MinimumSize ForwardInheritedValue(const MinimumSize &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline MaximumSize ForwardInheritedValue(const MaximumSize &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline Filter ForwardInheritedValue(const Filter &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline FlexBasis ForwardInheritedValue(const FlexBasis &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline DynamicRangeLimit ForwardInheritedValue(const DynamicRangeLimit &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline Clip ForwardInheritedValue(const Clip &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ClipPath ForwardInheritedValue(const ClipPath &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline CornerShapeValue ForwardInheritedValue(const CornerShapeValue &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline GridPosition ForwardInheritedValue(const GridPosition &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline GridTemplateAreas ForwardInheritedValue(const GridTemplateAreas &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline GridTemplateList ForwardInheritedValue(const GridTemplateList &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline GridTrackSizes ForwardInheritedValue(const GridTrackSizes &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline HyphenateCharacter ForwardInheritedValue(const HyphenateCharacter &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ItemTolerance ForwardInheritedValue(const ItemTolerance &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline LetterSpacing ForwardInheritedValue(const LetterSpacing &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline LineHeight ForwardInheritedValue(const LineHeight &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ListStyleType ForwardInheritedValue(const ListStyleType &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline NameScope ForwardInheritedValue(const NameScope &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline OffsetAnchor ForwardInheritedValue(const OffsetAnchor &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline OffsetDistance ForwardInheritedValue(const OffsetDistance &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline OffsetPath ForwardInheritedValue(const OffsetPath &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline OffsetPosition ForwardInheritedValue(const OffsetPosition &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline OffsetRotate ForwardInheritedValue(const OffsetRotate &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline Position ForwardInheritedValue(const Position &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline PositionAnchor ForwardInheritedValue(const PositionAnchor &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline PositionTryFallbacks ForwardInheritedValue(const PositionTryFallbacks &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline PositionX ForwardInheritedValue(const PositionX &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline PositionY ForwardInheritedValue(const PositionY &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline RepeatStyle ForwardInheritedValue(const RepeatStyle &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline SVGBaselineShift ForwardInheritedValue(const SVGBaselineShift &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline SVGCenterCoordinateComponent
    ForwardInheritedValue(const SVGCenterCoordinateComponent &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline SVGCoordinateComponent
    ForwardInheritedValue(const SVGCoordinateComponent &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline SVGMarkerResource ForwardInheritedValue(const SVGMarkerResource &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline SVGPathData ForwardInheritedValue(const SVGPathData &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline SVGPaint ForwardInheritedValue(const SVGPaint &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline SVGRadius ForwardInheritedValue(const SVGRadius &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline SVGRadiusComponent ForwardInheritedValue(const SVGRadiusComponent &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline SVGStrokeDasharray ForwardInheritedValue(const SVGStrokeDasharray &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline SVGStrokeDashoffset ForwardInheritedValue(const SVGStrokeDashoffset &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ScrollSnapAlign ForwardInheritedValue(const ScrollSnapAlign &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ScrollSnapType ForwardInheritedValue(const ScrollSnapType &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ScrollbarColor ForwardInheritedValue(const ScrollbarColor &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ScrollbarGutter ForwardInheritedValue(const ScrollbarGutter &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ShapeMargin ForwardInheritedValue(const ShapeMargin &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ShapeOutside ForwardInheritedValue(const ShapeOutside &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline SingleAnimationName ForwardInheritedValue(const SingleAnimationName &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline SingleAnimationRangeStart
    ForwardInheritedValue(const SingleAnimationRangeStart &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline SingleAnimationRangeEnd
    ForwardInheritedValue(const SingleAnimationRangeEnd &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline SingleAnimationRange ForwardInheritedValue(const SingleAnimationRange &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline SingleAnimationTimeline
    ForwardInheritedValue(const SingleAnimationTimeline &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline SingleTransitionProperty
    ForwardInheritedValue(const SingleTransitionProperty &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline StrokeWidth ForwardInheritedValue(const StrokeWidth &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline TabSize ForwardInheritedValue(const TabSize &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline TextDecorationLine ForwardInheritedValue(const TextDecorationLine &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline TextDecorationThickness
    ForwardInheritedValue(const TextDecorationThickness &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline TextEmphasisStyle ForwardInheritedValue(const TextEmphasisStyle &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline TextIndent ForwardInheritedValue(const TextIndent &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline TextShadows ForwardInheritedValue(const TextShadows &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline TextUnderlineOffset ForwardInheritedValue(const TextUnderlineOffset &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline URL ForwardInheritedValue(const URL &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline FixedVector<PositionTryFallback>
    ForwardInheritedValue(const FixedVector<PositionTryFallback> &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ProgressTimelineAxes ForwardInheritedValue(const ProgressTimelineAxes &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ProgressTimelineNames
    ForwardInheritedValue(const ProgressTimelineNames &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ScrollTimelines ForwardInheritedValue(const ScrollTimelines &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline Transform ForwardInheritedValue(const Transform &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline VerticalAlign ForwardInheritedValue(const VerticalAlign &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ViewTimelineInsets ForwardInheritedValue(const ViewTimelineInsets &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ViewTimelines ForwardInheritedValue(const ViewTimelines &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ViewTransitionClasses
    ForwardInheritedValue(const ViewTransitionClasses &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline ViewTransitionName ForwardInheritedValue(const ViewTransitionName &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline WebkitInitialLetter ForwardInheritedValue(const WebkitInitialLetter &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline WillChange ForwardInheritedValue(const WillChange &value) noexcept
  {
    auto copy = value;
    return copy;
  }
  KRYS_NODISCARD inline WordSpacing ForwardInheritedValue(const WordSpacing &value) noexcept
  {
    auto copy = value;
    return copy;
  }

  // Note that we assume the CSS parser only allows valid css::Value types.
  class BuilderCustom
  {
  public:
    // Custom handling of inherit, initial and value setting.
    // FIXME: <https://webkit.org/b/212506> Teach makeprop.pl to generate setters for hasExplicitlySet* flags
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(BorderBottomLeftRadius);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(BorderBottomRightRadius);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(BorderTopLeftRadius);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(BorderTopRightRadius);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(CaretColor);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(Color);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(CounterIncrement);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(CounterReset);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(CounterSet);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(Fill);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(FontFamily);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(FontSize);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(LetterSpacing);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(PaddingBottom);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(PaddingLeft);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(PaddingRight);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(PaddingTop);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(Stroke);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(WordSpacing);
    KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS(Zoom);

    // Custom handling of initial setting only.
    static void ApplyInitialBorderTopWidth(BuilderState &) noexcept;
    static void ApplyInitialBorderRightWidth(BuilderState &) noexcept;
    static void ApplyInitialBorderBottomWidth(BuilderState &) noexcept;
    static void ApplyInitialBorderLeftWidth(BuilderState &) noexcept;
    static void ApplyInitialOutlineWidth(BuilderState &) noexcept;
    static void ApplyInitialColumnRuleWidth(BuilderState &) noexcept;

    // Custom handling of value setting only.
    static void ApplyValueDirection(BuilderState &, css::Value &) noexcept;
    static void ApplyValueTextOrientation(BuilderState &, css::Value &) noexcept;
    static void ApplyValueWritingMode(BuilderState &, css::Value &) noexcept;

    static void ApplyValueColorScheme(BuilderState &, css::Value &) noexcept;

    static void ApplyValueStrokeWidth(BuilderState &, css::Value &) noexcept;
    static void ApplyValueStrokeColor(BuilderState &, css::Value &) noexcept;

  private:
    static void ResetUsedZoom(BuilderState &) noexcept;

    enum CounterBehavior
    {
      Increment,
      Reset,
      Set
    };
    template <CounterBehavior>
    static void ApplyInheritCounter(BuilderState &) noexcept;
    template <CounterBehavior>
    static void ApplyValueCounter(BuilderState &, css::Value &) noexcept;

    KRYS_NODISCARD static float LargerFontSize(float size) noexcept;
    KRYS_NODISCARD static float SmallerFontSize(float size) noexcept;
    KRYS_NODISCARD static float DetermineRubyTextSizeMultiplier(BuilderState &) noexcept;
    KRYS_NODISCARD static float DetermineMathDepthScale(BuilderState &) noexcept;
  };

  // MARK: - CoordinatedValueList Utilities

  template <auto propertyID, auto listMutableGetter, typename ListType>
  void ApplyInitialCoordinatedValueListProperty(BuilderState &builderState) noexcept
  {
    using PropertyAccessor = CoordinatedValueListPropertyAccessor<propertyID>;

    auto &list = (builderState.Style().*listMutableGetter)();
    krys_debug_assert(list.ComputedLength() > 0);

    PropertyAccessor {list[0]}.Set(PropertyAccessor::Initial());

    for (size_t i = 0; i < list.ComputedLength(); ++i)
    {
      PropertyAccessor {list[i]}.Clear();
    }
  }

  template <auto propertyID, auto listMutableGetter, auto listGetter, typename ListType>
  void ApplyInheritCoordinatedValueListProperty(BuilderState &builderState) noexcept
  {
    using PropertyAccessor = CoordinatedValueListPropertyAccessor<propertyID>;
    using ConstPropertyAccessor = CoordinatedValueListPropertyConstAccessor<propertyID>;

    auto &list = (builderState.Style().*listMutableGetter)();
    auto &parentList = (builderState.ParentStyle().*listGetter)();

    size_t i = 0;
    size_t parentSize = parentList.IsInitial() ? 0 : parentList.ComputedLength();

    for (; i < parentSize && ConstPropertyAccessor {parentList[i]}.IsSet(); ++i)
    {
      if (list.ComputedLength() <= i)
      {
        list.Append(typename ListType::value_type {});
      }
      PropertyAccessor {list[i]}.Set(ForwardInheritedValue(ConstPropertyAccessor {parentList[i]}.Get()));
    }

    for (; i < list.ComputedLength(); ++i)
    {
      PropertyAccessor {list[i]}.Clear();
    }
  }

  template <auto propertyID, auto listMutableGetter, typename ItemType, typename ListType>
  void ApplyValueCoordinatedValueListProperty(BuilderState &builderState, css::Value &value) noexcept
  {
    using PropertyAccessor = CoordinatedValueListPropertyAccessor<propertyID>;

    auto &list = (builderState.Style().*listMutableGetter)();

    auto set = [&](auto i, auto &item)
    {
      if (item.ValueId() == css::ValueId::Initial)
      {
        PropertyAccessor {list[i]}.Set(PropertyAccessor::Initial());
      }
      else
      {
        PropertyAccessor {list[i]}.Set(ConvertCSSValueToStyle<ItemType>(builderState, item));
      }
    };

    size_t i = 0;
    if (auto *valueList = DynamicDowncast<css::ValueList>(value))
    {
      for (const auto &item : *valueList)
      {
        if (i >= list.ComputedLength())
        {
          list.Append(typename ListType::value_type {});
        }

        set(i, item);
        ++i;
      }
    }
    else
    {
      krys_debug_assert(list.ComputedLength() > 0);
      set(0, value);
      i = 1;
    }

    for (; i < list.ComputedLength(); ++i)
    {
      PropertyAccessor {list[i]}.Clear();
    }
  }

  // MARK: - Custom conversions

  inline void BuilderCustom::ApplyValueDirection(BuilderState &builderState, css::Value &value) noexcept
  {
    builderState.Style().SetDirection(FromCSSValue<TextDirection>(value));
    builderState.Style().SetHasExplicitlySetDirection(true);
  }

  inline void BuilderCustom::ResetUsedZoom(BuilderState &builderState) noexcept
  {
    // Reset the zoom in effect. This allows the setZoom method to accurately compute a new zoom in effect.
    builderState.SetUsedZoom(builderState.ParentStyle().UsedZoom());
  }

  inline void BuilderCustom::ApplyInitialZoom(BuilderState &builderState) noexcept
  {
    ResetUsedZoom(builderState);
    builderState.SetZoom(style::ComputedStyle::InitialZoom());
  }

  inline void BuilderCustom::ApplyInheritZoom(BuilderState &builderState) noexcept
  {
    ResetUsedZoom(builderState);
    builderState.SetZoom(ForwardInheritedValue(builderState.ParentStyle().GetZoom()));
  }

  inline void BuilderCustom::ApplyValueZoom(BuilderState &builderState, css::Value &value) noexcept
  {
    auto primitiveValue = RequiredDowncast<css::PrimitiveValue>(builderState, value);
    if (!primitiveValue)
    {
      return;
    }

    if (primitiveValue->ValueId() == css::ValueId::Normal)
    {
      ResetUsedZoom(builderState);
      builderState.SetZoom(style::ComputedStyle::InitialZoom());
    }
    else
    {
      ResetUsedZoom(builderState);

      auto zoom = ConvertCSSValueToStyle<Zoom>(builderState, *primitiveValue);
      // FIXME: The spec says that zoom values of 0 should be treated as 1, not ignored entirely.
      // https://drafts.csswg.org/css-viewport/#valdef-zoom-number
      if (!IsValueZero(zoom))
      {
        builderState.SetZoom(zoom);
      }
    }
  }

  inline void MaybeUpdateFontForLetterSpacingOrWordSpacing(BuilderState &builderState,
                                                           css::Value &value) noexcept
  {
    // This is unfortunate. It's related to https://github.com/w3c/csswg-drafts/issues/5498.
    //
    // From StyleBuilder's point of view, there's a dependency cycle:
    // letter-spacing accepts an arbitrary <length>, which must be resolved against a font, which must
    // be selected after all the properties that affect font selection are processed, but letter-spacing
    // itself affects font selection because it can disable font features. StyleBuilder has some (valid)
    // krys_debug_assert()s which would fire because of this cycle.
    //
    // There isn't *actually* a dependency cycle, though, as none of the font-relative units are
    // actually sensitive to font features (luckily). The problem is that our StyleBuilder is only
    // smart enough to consider fonts as one indivisible thing, rather than having the deeper
    // understanding that different parts of fonts may or may not depend on each other.
    //
    // So, we update the font early here, so that if there is a font-relative unit inside the css::Value,
    // its font is updated and ready to go. In the worst case there might be a second call to
    // updateFont() later, but that isn't bad for perf because 1. It only happens twice if there is
    // actually a font-relative unit passed to letter-spacing or word-spacing, and 2. updateFont() internally
    // has logic to only do work if the font is actually dirty.

    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      if (primitiveValue->IsFontRelativeLength() || primitiveValue->IsCalculated())
      {
        builderState.UpdateFont();
      }
    }
  }

  inline void BuilderCustom::ApplyInheritWordSpacing(BuilderState &builderState) noexcept
  {
    builderState.Style().SetWordSpacing(
      ForwardInheritedValue(builderState.ParentStyle().ComputedWordSpacing()));
    builderState.SetFontDirty();
  }

  inline void BuilderCustom::ApplyInitialWordSpacing(BuilderState &builderState) noexcept
  {
    builderState.Style().SetWordSpacing(style::ComputedStyle::InitialWordSpacing());
    builderState.SetFontDirty();
  }

  inline void BuilderCustom::ApplyValueWordSpacing(BuilderState &builderState, css::Value &value) noexcept
  {
    MaybeUpdateFontForLetterSpacingOrWordSpacing(builderState, value);
    builderState.Style().SetWordSpacing(ConvertCSSValueToStyle<WordSpacing>(builderState, value));
    builderState.SetFontDirty();
  }

  inline void BuilderCustom::ApplyInheritLetterSpacing(BuilderState &builderState) noexcept
  {
    builderState.Style().SetLetterSpacing(
      ForwardInheritedValue(builderState.ParentStyle().ComputedLetterSpacing()));
    builderState.SetFontDirty();
  }

  inline void BuilderCustom::ApplyInitialLetterSpacing(BuilderState &builderState) noexcept
  {
    builderState.Style().SetLetterSpacing(style::ComputedStyle::InitialLetterSpacing());
    builderState.SetFontDirty();
  }

  inline void BuilderCustom::ApplyValueLetterSpacing(BuilderState &builderState, css::Value &value) noexcept
  {
    MaybeUpdateFontForLetterSpacingOrWordSpacing(builderState, value);
    builderState.Style().SetLetterSpacing(ConvertCSSValueToStyle<LetterSpacing>(builderState, value));
    builderState.SetFontDirty();
  }

  inline void BuilderCustom::ApplyInitialCaretColor(BuilderState &builderState) noexcept
  {
    if (builderState.ApplyPropertyToRegularStyle())
    {
      builderState.Style().SetHasAutoCaretColor();
    }
    if (builderState.ApplyPropertyToVisitedLinkStyle())
    {
      builderState.Style().SetHasVisitedLinkAutoCaretColor();
    }
  }

  inline void BuilderCustom::ApplyInheritCaretColor(BuilderState &builderState) noexcept
  {
    auto &color = builderState.ParentStyle().CaretColor();
    if (builderState.ApplyPropertyToRegularStyle())
    {
      if (builderState.ParentStyle().HasAutoCaretColor())
      {
        builderState.Style().SetHasAutoCaretColor();
      }
      else
      {
        builderState.Style().SetCaretColor(ForwardInheritedValue(color));
      }
    }
    if (builderState.ApplyPropertyToVisitedLinkStyle())
    {
      if (builderState.ParentStyle().HasVisitedLinkAutoCaretColor())
      {
        builderState.Style().SetHasVisitedLinkAutoCaretColor();
      }
      else
      {
        builderState.Style().SetVisitedLinkCaretColor(ForwardInheritedValue(color));
      }
    }
  }

  inline void BuilderCustom::ApplyValueCaretColor(BuilderState &builderState, css::Value &value) noexcept
  {
    if (builderState.ApplyPropertyToRegularStyle())
    {
      if (value.ValueId() == css::ValueId::Auto)
      {
        builderState.Style().SetHasAutoCaretColor();
      }
      else
      {
        builderState.Style().SetCaretColor(
          ConvertCSSValueToStyle<Color>(builderState, value, ForVisitedLink::No));
      }
    }
    if (builderState.ApplyPropertyToVisitedLinkStyle())
    {
      if (value.ValueId() == css::ValueId::Auto)
      {
        builderState.Style().SetHasVisitedLinkAutoCaretColor();
      }
      else
      {
        builderState.Style().SetVisitedLinkCaretColor(
          ConvertCSSValueToStyle<Color>(builderState, value, ForVisitedLink::Yes));
      }
    }
  }

  inline void BuilderCustom::ApplyValueWritingMode(BuilderState &builderState, css::Value &value) noexcept
  {
    builderState.SetWritingMode(FromCSSValue<StyleWritingMode>(value));
    builderState.Style().SetHasExplicitlySetWritingMode(true);
  }

  inline void BuilderCustom::ApplyValueTextOrientation(BuilderState &builderState, css::Value &value) noexcept
  {
    builderState.SetTextOrientation(FromCSSValue<TextOrientation>(value));
  }

  inline void BuilderCustom::ApplyValueColorScheme(BuilderState &builderState, css::Value &value) noexcept
  {
    builderState.Style().SetColorScheme(ConvertCSSValueToStyle<ColorScheme>(builderState, value));
    builderState.Style().SetHasExplicitlySetColorScheme(true);
  }

  inline void BuilderCustom::ApplyInitialFontFamily(BuilderState &builderState) noexcept
  {
    auto &fontDescription = builderState.GetFontDescription();
    auto initialDesc = FontCascadeDescription();

    // We need to adjust the size to account for the generic family change from monospace to non-monospace.
    if (fontDescription.UseFixedDefaultSize())
    {
      if (css::ValueId sizeIdentifier = fontDescription.KeywordSizeAsIdentifier();
          sizeIdentifier != css::ValueId::Invalid)
      {
        builderState.SetFontDescriptionFontSize(
          style::FontSizeForKeyword(static_cast<uint32>(sizeIdentifier), false, builderState.GetDocument()));
      }
    }

    if (!initialDesc.FirstFamily().View().empty())
    {
      builderState.SetFontDescriptionFamilies(FontFamilies {
        List<css::CSSOMStringAtom> {initialDesc.Families()}, fontDescription.IsSpecifiedFont()});
    }
  }

  inline void BuilderCustom::ApplyInheritFontFamily(BuilderState &builderState) noexcept
  {
    builderState.SetFontDescriptionFamilies(ForwardInheritedValue(builderState.ParentStyle().FontFamily()));
  }

  inline void BuilderCustom::ApplyValueFontFamily(BuilderState &builderState, css::Value &value) noexcept
  {
    auto &fontDescription = builderState.GetFontDescription();

    // Before mapping in a new font-family property, we should reset the generic family.
    bool oldFamilyUsedFixedDefaultSize = fontDescription.UseFixedDefaultSize();

    builderState.SetFontDescriptionFamilies(ConvertCSSValueToStyle<FontFamilies>(builderState, value));

    if (fontDescription.UseFixedDefaultSize() != oldFamilyUsedFixedDefaultSize)
    {
      if (css::ValueId sizeIdentifier = fontDescription.KeywordSizeAsIdentifier();
          sizeIdentifier != css::ValueId::Invalid)
      {
        builderState.SetFontDescriptionFontSize(style::FontSizeForKeyword(
          static_cast<uint32>(sizeIdentifier), !oldFamilyUsedFixedDefaultSize, builderState.GetDocument()));
      }
    }
  }

  inline void BuilderCustom::ApplyInitialBorderTopWidth(BuilderState &builderState) noexcept
  {
    builderState.Style().SetBorderTopWidth(
      style::LineWidth {style::ComputedStyle::InitialBorderTopWidth().Value.UnresolvedValue()
                        * builderState.Style().UsedZoom()});
  }

  inline void BuilderCustom::ApplyInitialBorderRightWidth(BuilderState &builderState) noexcept
  {
    builderState.Style().SetBorderRightWidth(
      style::LineWidth {style::ComputedStyle::InitialBorderRightWidth().Value.UnresolvedValue()
                        * builderState.Style().UsedZoom()});
  }

  inline void BuilderCustom::ApplyInitialBorderBottomWidth(BuilderState &builderState) noexcept
  {
    builderState.Style().SetBorderBottomWidth(
      style::LineWidth {style::ComputedStyle::InitialBorderBottomWidth().Value.UnresolvedValue()
                        * builderState.Style().UsedZoom()});
  }

  inline void BuilderCustom::ApplyInitialBorderLeftWidth(BuilderState &builderState) noexcept
  {
    builderState.Style().SetBorderLeftWidth(
      style::LineWidth {style::ComputedStyle::InitialBorderLeftWidth().Value.UnresolvedValue()
                        * builderState.Style().UsedZoom()});
  }

  inline void BuilderCustom::ApplyInitialOutlineWidth(BuilderState &builderState) noexcept
  {
    builderState.Style().SetOutlineWidth(style::LineWidth {
      style::ComputedStyle::InitialOutlineWidth().Value.UnresolvedValue() * builderState.Style().UsedZoom()});
  }

  inline void BuilderCustom::ApplyInitialColumnRuleWidth(BuilderState &builderState) noexcept
  {
    builderState.Style().SetColumnRuleWidth(
      style::LineWidth {style::ComputedStyle::InitialColumnRuleWidth().Value.UnresolvedValue()
                        * builderState.Style().UsedZoom()});
  }

  inline void BuilderCustom::ApplyInitialBorderBottomLeftRadius(BuilderState &builderState) noexcept
  {
    builderState.Style().SetBorderBottomLeftRadius(style::ComputedStyle::InitialBorderBottomLeftRadius());
    builderState.Style().SetHasExplicitlySetBorderBottomLeftRadius(false);
  }

  inline void BuilderCustom::ApplyInheritBorderBottomLeftRadius(BuilderState &builderState) noexcept
  {
    builderState.Style().SetBorderBottomLeftRadius(
      ForwardInheritedValue(builderState.ParentStyle().BorderBottomLeftRadius()));
    builderState.Style().SetHasExplicitlySetBorderBottomLeftRadius(
      builderState.ParentStyle().HasExplicitlySetBorderBottomLeftRadius());
  }

  inline void BuilderCustom::ApplyValueBorderBottomLeftRadius(BuilderState &builderState,
                                                              css::Value &value) noexcept
  {
    builderState.Style().SetBorderBottomLeftRadius(
      ConvertCSSValueToStyle<BorderRadiusValue>(builderState, value));
    builderState.Style().SetHasExplicitlySetBorderBottomLeftRadius(true);
  }

  inline void BuilderCustom::ApplyInitialBorderBottomRightRadius(BuilderState &builderState) noexcept
  {
    builderState.Style().SetBorderBottomRightRadius(style::ComputedStyle::InitialBorderBottomRightRadius());
    builderState.Style().SetHasExplicitlySetBorderBottomRightRadius(false);
  }

  inline void BuilderCustom::ApplyInheritBorderBottomRightRadius(BuilderState &builderState) noexcept
  {
    builderState.Style().SetBorderBottomRightRadius(
      ForwardInheritedValue(builderState.ParentStyle().BorderBottomRightRadius()));
    builderState.Style().SetHasExplicitlySetBorderBottomRightRadius(
      builderState.ParentStyle().HasExplicitlySetBorderBottomRightRadius());
  }

  inline void BuilderCustom::ApplyValueBorderBottomRightRadius(BuilderState &builderState,
                                                               css::Value &value) noexcept
  {
    builderState.Style().SetBorderBottomRightRadius(
      ConvertCSSValueToStyle<BorderRadiusValue>(builderState, value));
    builderState.Style().SetHasExplicitlySetBorderBottomRightRadius(true);
  }

  inline void BuilderCustom::ApplyInitialBorderTopLeftRadius(BuilderState &builderState) noexcept
  {
    builderState.Style().SetBorderTopLeftRadius(style::ComputedStyle::InitialBorderTopLeftRadius());
    builderState.Style().SetHasExplicitlySetBorderTopLeftRadius(false);
  }

  inline void BuilderCustom::ApplyInheritBorderTopLeftRadius(BuilderState &builderState) noexcept
  {
    builderState.Style().SetBorderTopLeftRadius(
      ForwardInheritedValue(builderState.ParentStyle().BorderTopLeftRadius()));
    builderState.Style().SetHasExplicitlySetBorderTopLeftRadius(
      builderState.ParentStyle().HasExplicitlySetBorderTopLeftRadius());
  }

  inline void BuilderCustom::ApplyValueBorderTopLeftRadius(BuilderState &builderState,
                                                           css::Value &value) noexcept
  {
    builderState.Style().SetBorderTopLeftRadius(
      ConvertCSSValueToStyle<BorderRadiusValue>(builderState, value));
    builderState.Style().SetHasExplicitlySetBorderTopLeftRadius(true);
  }

  inline void BuilderCustom::ApplyInitialBorderTopRightRadius(BuilderState &builderState) noexcept
  {
    builderState.Style().SetBorderTopRightRadius(style::ComputedStyle::InitialBorderTopRightRadius());
    builderState.Style().SetHasExplicitlySetBorderTopRightRadius(false);
  }

  inline void BuilderCustom::ApplyInheritBorderTopRightRadius(BuilderState &builderState) noexcept
  {
    builderState.Style().SetBorderTopRightRadius(
      ForwardInheritedValue(builderState.ParentStyle().BorderTopRightRadius()));
    builderState.Style().SetHasExplicitlySetBorderTopRightRadius(
      builderState.ParentStyle().HasExplicitlySetBorderTopRightRadius());
  }

  inline void BuilderCustom::ApplyValueBorderTopRightRadius(BuilderState &builderState,
                                                            css::Value &value) noexcept
  {
    builderState.Style().SetBorderTopRightRadius(
      ConvertCSSValueToStyle<BorderRadiusValue>(builderState, value));
    builderState.Style().SetHasExplicitlySetBorderTopRightRadius(true);
  }

  template <BuilderCustom::CounterBehavior counterBehavior>
  inline void BuilderCustom::ApplyInheritCounter(BuilderState &builderState) noexcept
  {
    auto &map = builderState.Style().AccessCounterDirectives().Map;
    for (const auto &[key, value] : builderState.ParentStyle().GetCounterDirectives().Map)
    {
      auto &directives = map.try_emplace(key).first->second;
      if constexpr (counterBehavior == Reset)
      {
        directives.ResetValue = value.ResetValue;
      }
      else if constexpr (counterBehavior == Increment)
      {
        directives.IncrementValue = value.IncrementValue;
      }
      else
      {
        directives.SetValue = value.SetValue;
      }
    }
  }

  template <BuilderCustom::CounterBehavior counterBehavior>
  inline void BuilderCustom::ApplyValueCounter(BuilderState &builderState, css::Value &value) noexcept
  {
    bool setCounterIncrementToNone = counterBehavior == Increment && value.ValueId() == css::ValueId::None;

    if (!Is<css::ValueList>(value) && !setCounterIncrementToNone)
    {
      return;
    }

    auto &map = builderState.Style().AccessCounterDirectives().Map;
    for (auto &[key, directives] : map)
    {
      if constexpr (counterBehavior == Reset)
      {
        directives.ResetValue = std::nullopt;
      }
      else if constexpr (counterBehavior == Increment)
      {
        directives.IncrementValue = std::nullopt;
      }
      else
      {
        directives.SetValue = std::nullopt;
      }
    }

    if (setCounterIncrementToNone)
    {
      return;
    }

    auto &conversionData = builderState.CssToLengthConversionData();

    auto list = RequiredListDowncast<css::ValueList, css::ValuePair>(builderState, value);
    if (!list)
    {
      return;
    }

    for (auto &pairValue : *list)
    {
      auto pair = RequiredPairDowncast<css::PrimitiveValue>(builderState, pairValue);
      if (!pair)
      {
        return;
      }
      css::CSSOMStringAtom identifier {pair->first->StringValue()};
      int32 counterValue = pair->second->ResolveAsNumber<int32>(conversionData);
      auto &directives = map.try_emplace(identifier).first->second;
      if constexpr (counterBehavior == Reset)
      {
        directives.ResetValue = counterValue;
      }
      else if constexpr (counterBehavior == Increment)
      {
        directives.IncrementValue = SaturatedSum(directives.IncrementValue.value_or(0), counterValue);
      }
      else
      {
        directives.SetValue = counterValue;
      }
    }
  }

  inline void BuilderCustom::ApplyInitialCounterIncrement(BuilderState &) noexcept
  {
  }

  inline void BuilderCustom::ApplyInheritCounterIncrement(BuilderState &builderState) noexcept
  {
    ApplyInheritCounter<Increment>(builderState);
  }

  inline void BuilderCustom::ApplyValueCounterIncrement(BuilderState &builderState,
                                                        css::Value &value) noexcept
  {
    ApplyValueCounter<Increment>(builderState, value);
  }

  inline void BuilderCustom::ApplyInitialCounterReset(BuilderState &) noexcept
  {
  }

  inline void BuilderCustom::ApplyInheritCounterReset(BuilderState &builderState) noexcept
  {
    ApplyInheritCounter<Reset>(builderState);
  }

  inline void BuilderCustom::ApplyValueCounterReset(BuilderState &builderState, css::Value &value) noexcept
  {
    ApplyValueCounter<Reset>(builderState, value);
  }

  inline void BuilderCustom::ApplyInitialCounterSet(BuilderState &) noexcept
  {
  }

  inline void BuilderCustom::ApplyInheritCounterSet(BuilderState &builderState) noexcept
  {
    ApplyInheritCounter<Set>(builderState);
  }

  inline void BuilderCustom::ApplyValueCounterSet(BuilderState &builderState, css::Value &value) noexcept
  {
    ApplyValueCounter<Set>(builderState, value);
  }

  inline void BuilderCustom::ApplyInitialFill(BuilderState &builderState) noexcept
  {
    auto &style = builderState.Style();
    if (builderState.ApplyPropertyToRegularStyle())
    {
      style.SetFill(style::ComputedStyle::InitialFill());
    }
    if (builderState.ApplyPropertyToVisitedLinkStyle())
    {
      style.SetVisitedLinkFill(style::ComputedStyle::InitialFill());
    }
  }

  inline void BuilderCustom::ApplyInheritFill(BuilderState &builderState) noexcept
  {
    auto &style = builderState.Style();
    auto &parentStyle = builderState.ParentStyle();

    if (builderState.ApplyPropertyToRegularStyle())
    {
      style.SetFill(ForwardInheritedValue(parentStyle.Fill()));
    }
    if (builderState.ApplyPropertyToVisitedLinkStyle())
    {
      style.SetVisitedLinkFill(ForwardInheritedValue(parentStyle.Fill()));
    }
  }

  inline void BuilderCustom::ApplyValueFill(BuilderState &builderState, css::Value &value) noexcept
  {
    auto &style = builderState.Style();
    if (builderState.ApplyPropertyToRegularStyle())
    {
      style.SetFill(ConvertCSSValueToStyle<SVGPaint>(builderState, value, ForVisitedLink::No));
    }
    if (builderState.ApplyPropertyToVisitedLinkStyle())
    {
      style.SetVisitedLinkFill(ConvertCSSValueToStyle<SVGPaint>(builderState, value, ForVisitedLink::Yes));
    }
  }

  inline void BuilderCustom::ApplyInitialStroke(BuilderState &builderState) noexcept
  {
    auto &style = builderState.Style();
    if (builderState.ApplyPropertyToRegularStyle())
    {
      style.SetStroke(style::ComputedStyle::InitialStroke());
    }
    if (builderState.ApplyPropertyToVisitedLinkStyle())
    {
      style.SetVisitedLinkStroke(style::ComputedStyle::InitialStroke());
    }
  }

  inline void BuilderCustom::ApplyInheritStroke(BuilderState &builderState) noexcept
  {
    auto &style = builderState.Style();
    auto &parentStyle = builderState.ParentStyle();

    if (builderState.ApplyPropertyToRegularStyle())
    {
      style.SetStroke(ForwardInheritedValue(parentStyle.Stroke()));
    }
    if (builderState.ApplyPropertyToVisitedLinkStyle())
    {
      style.SetVisitedLinkStroke(ForwardInheritedValue(parentStyle.Stroke()));
    }
  }

  inline void BuilderCustom::ApplyValueStroke(BuilderState &builderState, css::Value &value) noexcept
  {
    auto &style = builderState.Style();
    if (builderState.ApplyPropertyToRegularStyle())
    {
      style.SetStroke(ConvertCSSValueToStyle<SVGPaint>(builderState, value, ForVisitedLink::No));
    }
    if (builderState.ApplyPropertyToVisitedLinkStyle())
    {
      style.SetVisitedLinkStroke(ConvertCSSValueToStyle<SVGPaint>(builderState, value, ForVisitedLink::Yes));
    }
  }

  inline void BuilderCustom::ApplyInitialFontSize(BuilderState &builderState) noexcept
  {
    auto fontDescription = builderState.GetFontDescription();
    float size = style::FontSizeForKeyword(static_cast<uint32>(css::ValueId::Medium),
                                           fontDescription.UseFixedDefaultSize(), builderState.GetDocument());

    if (size < 0)
    {
      return;
    }

    fontDescription.SetKeywordSizeFromIdentifier(css::ValueId::Medium);
    builderState.SetFontSize(fontDescription, size);
    builderState.SetFontDescription(krys::move(fontDescription));
  }

  inline void BuilderCustom::ApplyInheritFontSize(BuilderState &builderState) noexcept
  {
    const auto &parentFontDescription = builderState.ParentStyle().GetFontDescription();
    float size = parentFontDescription.SpecifiedSize();

    if (size < 0)
    {
      return;
    }

    builderState.SetFontDescriptionKeywordSize(parentFontDescription.KeywordSize());
    builderState.SetFontDescriptionFontSize(size);
  }

  // When the CSS keyword "larger" is used, this function will attempt to match within the keyword
  // table, and failing that, will simply multiply by 1.2.
  inline float BuilderCustom::LargerFontSize(float size) noexcept
  {
    // FIXME: Figure out where we fall in the size ranges (xx-small to xxx-large) and scale up to
    // the next size level.
    return size * 1.2f;
  }

  // Like the previous function, but for the keyword "smaller".
  inline float BuilderCustom::SmallerFontSize(float size) noexcept
  {
    // FIXME: Figure out where we fall in the size ranges (xx-small to xxx-large) and scale down to
    // the next size level.
    return size / 1.2f;
  }

  inline float BuilderCustom::DetermineRubyTextSizeMultiplier(BuilderState &builderState) noexcept
  {
    if (!builderState.Style().IsInterCharacterRubyPosition())
    {
      return 0.5f;
    }

    auto rubyPosition = builderState.Style().GetRubyPosition();
    if (rubyPosition == RubyPosition::InterCharacter)
    {
      // If the writing mode of the enclosing ruby container is vertical, 'inter-character' value has the same
      // effect as over.
      return !builderState.ParentStyle().GetWritingMode().IsVerticalTypographic() ? 0.3f : 0.5f;
    }

    // Legacy inter-character behavior.
    // FIXME: This hack is to ensure tone marks are the same size as
    // the bopomofo. This code will go away if we make a special renderer
    // for the tone marks eventually.
    for (auto *ancestor = builderState.GetElement() ? builderState.GetElement()->ParentElement() : nullptr;
         ancestor; ancestor = ancestor->ParentElement())
    {
      if (Is<html::HTMLElement>(*ancestor) && ancestor->TagQualifiedName().TagName() == html::TagName::rt)
      {
        return 1.0f;
      }
    }
    return 0.25f;
  }

  // https://w3c.github.io/mathml-core/#the-math-script-level-property
  inline float BuilderCustom::DetermineMathDepthScale(BuilderState &builderState) noexcept
  {
    // Step 1.
    auto inherited = builderState.ParentStyle().GetMathDepth();
    auto computed = builderState.Style().GetMathDepth();
    float scale = 1.0f;
    float scaleDown = 0.71f;

    // Step 2.
    if (inherited == computed)
    {
      return scale;
    }
    bool invertScaleFactor = false;
    if (computed < inherited)
    {
      std::swap(computed, inherited);
      invertScaleFactor = true;
    }

    // Step 3.
    int exponent = computed.Value - inherited.Value;

    // Step 4.

    // Step 5.
    scale *= static_cast<float>(std::pow(scaleDown, exponent));

    // Step 6.
    return invertScaleFactor ? 1.f / scale : scale;
  }

  inline void BuilderCustom::ApplyValueFontSize(BuilderState &builderState, css::Value &value) noexcept
  {
    auto &fontDescription = builderState.GetFontDescription();
    builderState.SetFontDescriptionKeywordSizeFromIdentifier(css::ValueId::Invalid);

    float parentSize = builderState.ParentStyle().GetFontDescription().SpecifiedSize();
    bool parentIsAbsoluteSize = builderState.ParentStyle().GetFontDescription().IsAbsoluteSize();

    auto primitiveValue = RequiredDowncast<css::PrimitiveValue>(builderState, value);
    if (!primitiveValue)
    {
      return;
    }

    float size = 0;
    if (css::ValueId ident = primitiveValue->ValueId(); ident != css::ValueId::Invalid)
    {
      builderState.SetFontDescriptionIsAbsoluteSize(
        (parentIsAbsoluteSize
         && (ident == css::ValueId::Larger || ident == css::ValueId::Smaller
             || ident == css::ValueId::WebkitRubyText || ident == css::ValueId::Math))
        || css::PropertyParserHelpers::IsSystemFontShorthand(ident));

      if (css::PropertyParserHelpers::IsSystemFontShorthand(ident))
      {
        size = SystemFontDatabase::Singleton().SystemFontShorthandSize(
          css::PropertyParserHelpers::LowerFontShorthand(ident));
      }

      switch (ident)
      {
        case css::ValueId::XxSmall:
        case css::ValueId::XSmall:
        case css::ValueId::Small:
        case css::ValueId::Medium:
        case css::ValueId::Large:
        case css::ValueId::XLarge:
        case css::ValueId::XxLarge:
        case css::ValueId::XxxLarge:
          size = style::FontSizeForKeyword(static_cast<uint32>(ident), fontDescription.UseFixedDefaultSize(),
                                           builderState.GetDocument());
          builderState.SetFontDescriptionKeywordSizeFromIdentifier(ident);
          break;
        case css::ValueId::Larger:  size = LargerFontSize(parentSize); break;
        case css::ValueId::Smaller: size = SmallerFontSize(parentSize); break;
        case css::ValueId::Math:    size = DetermineMathDepthScale(builderState) * parentSize; break;
        case css::ValueId::WebkitRubyText:
          size = DetermineRubyTextSizeMultiplier(builderState) * parentSize;
          break;
        default: break;
      }
    }
    else
    {
      builderState.SetFontDescriptionIsAbsoluteSize(parentIsAbsoluteSize
                                                    || !primitiveValue->IsParentFontRelativeLength());
      auto conversionData = builderState.CssToLengthConversionData().CopyForFontSize();
      if (primitiveValue->IsLength())
      {
        size = primitiveValue->ResolveAsLength<float>(conversionData);
      }
      else if (primitiveValue->IsPercentage())
      {
        size = (primitiveValue->ResolveAsPercentage<float>(conversionData) * parentSize) / 100.0f;
      }
      else if (primitiveValue->IsCalculatedPercentageWithLength())
      {
        size = static_cast<float>(primitiveValue->CSSCalcValue()
                                    ->CreateCalculationValue(conversionData, css::SymbolTable {})
                                    ->Evaluate(parentSize, style::ZoomNeeded {}));
      }
      else
      {
        return;
      }
    }

    if (size < 0)
    {
      return;
    }

    builderState.SetFontDescriptionFontSize(std::min(MaximumAllowedFontSize, size));
  }

  inline void BuilderCustom::ApplyValueStrokeWidth(BuilderState &builderState, css::Value &value) noexcept
  {
    builderState.Style().SetStrokeWidth(ConvertCSSValueToStyle<StrokeWidth>(builderState, value));
    builderState.Style().SetHasExplicitlySetStrokeWidth(true);
  }

  inline void BuilderCustom::ApplyValueStrokeColor(BuilderState &builderState, css::Value &value) noexcept
  {
    if (builderState.ApplyPropertyToRegularStyle())
    {
      builderState.Style().SetStrokeColor(
        ConvertCSSValueToStyle<Color>(builderState, value, ForVisitedLink::No));
    }
    if (builderState.ApplyPropertyToVisitedLinkStyle())
    {
      builderState.Style().SetVisitedLinkStrokeColor(
        ConvertCSSValueToStyle<Color>(builderState, value, ForVisitedLink::Yes));
    }
    builderState.Style().SetHasExplicitlySetStrokeColor(true);
  }

  // For the color property, "currentcolor" is actually the inherited computed color.
  inline void BuilderCustom::ApplyValueColor(BuilderState &builderState, css::Value &value) noexcept
  {
    if (builderState.ApplyPropertyToRegularStyle())
    {
      auto color = ConvertCSSValueToStyle<Color>(builderState, value, ForVisitedLink::No);
      builderState.Style().SetColor(color.ResolveColor(builderState.ParentStyle().GetColor()));
    }
    if (builderState.ApplyPropertyToVisitedLinkStyle())
    {
      auto color = ConvertCSSValueToStyle<Color>(builderState, value, ForVisitedLink::Yes);
      builderState.Style().SetVisitedLinkColor(
        color.ResolveColor(builderState.ParentStyle().VisitedLinkColor()));
    }

    builderState.Style().SetDisallowsFastPathInheritance();
    builderState.Style().SetHasExplicitlySetColor(builderState.IsAuthorOrigin());
  }

  inline void BuilderCustom::ApplyInitialColor(BuilderState &builderState) noexcept
  {
    if (builderState.ApplyPropertyToRegularStyle())
    {
      builderState.Style().SetColor(style::ComputedStyle::InitialColor());
    }
    if (builderState.ApplyPropertyToVisitedLinkStyle())
    {
      builderState.Style().SetVisitedLinkColor(style::ComputedStyle::InitialColor());
    }

    builderState.Style().SetDisallowsFastPathInheritance();
    builderState.Style().SetHasExplicitlySetColor(builderState.IsAuthorOrigin());
  }

  inline void BuilderCustom::ApplyInheritColor(BuilderState &builderState) noexcept
  {
    if (builderState.ApplyPropertyToRegularStyle())
    {
      builderState.Style().SetColor(ForwardInheritedValue(builderState.ParentStyle().GetColor()));
    }
    if (builderState.ApplyPropertyToVisitedLinkStyle())
    {
      builderState.Style().SetVisitedLinkColor(ForwardInheritedValue(builderState.ParentStyle().GetColor()));
    }

    builderState.Style().SetDisallowsFastPathInheritance();
    builderState.Style().SetHasExplicitlySetColor(builderState.IsAuthorOrigin());
  }

  inline void BuilderCustom::ApplyInitialPaddingBottom(BuilderState &builderState) noexcept
  {
    builderState.Style().SetPaddingBottom(style::ComputedStyle::InitialPaddingBottom());
    builderState.Style().SetHasExplicitlySetPaddingBottom(builderState.IsAuthorOrigin());
  }

  inline void BuilderCustom::ApplyInheritPaddingBottom(BuilderState &builderState) noexcept
  {
    builderState.Style().SetPaddingBottom(ForwardInheritedValue(builderState.ParentStyle().PaddingBottom()));
    builderState.Style().SetHasExplicitlySetPaddingBottom(builderState.IsAuthorOrigin());
  }

  inline void BuilderCustom::ApplyValuePaddingBottom(BuilderState &builderState, css::Value &value) noexcept
  {
    builderState.Style().SetPaddingBottom(ConvertCSSValueToStyle<PaddingEdge>(builderState, value));
    builderState.Style().SetHasExplicitlySetPaddingBottom(builderState.IsAuthorOrigin());
  }

  inline void BuilderCustom::ApplyInitialPaddingLeft(BuilderState &builderState) noexcept
  {
    builderState.Style().SetPaddingLeft(style::ComputedStyle::InitialPaddingLeft());
    builderState.Style().SetHasExplicitlySetPaddingLeft(builderState.IsAuthorOrigin());
  }

  inline void BuilderCustom::ApplyInheritPaddingLeft(BuilderState &builderState) noexcept
  {
    builderState.Style().SetPaddingLeft(ForwardInheritedValue(builderState.ParentStyle().PaddingLeft()));
    builderState.Style().SetHasExplicitlySetPaddingLeft(builderState.IsAuthorOrigin());
  }

  inline void BuilderCustom::ApplyValuePaddingLeft(BuilderState &builderState, css::Value &value) noexcept
  {
    builderState.Style().SetPaddingLeft(ConvertCSSValueToStyle<PaddingEdge>(builderState, value));
    builderState.Style().SetHasExplicitlySetPaddingLeft(builderState.IsAuthorOrigin());
  }

  inline void BuilderCustom::ApplyInitialPaddingRight(BuilderState &builderState) noexcept
  {
    builderState.Style().SetPaddingRight(style::ComputedStyle::InitialPaddingRight());
    builderState.Style().SetHasExplicitlySetPaddingRight(builderState.IsAuthorOrigin());
  }

  inline void BuilderCustom::ApplyInheritPaddingRight(BuilderState &builderState) noexcept
  {
    builderState.Style().SetPaddingRight(ForwardInheritedValue(builderState.ParentStyle().PaddingRight()));
    builderState.Style().SetHasExplicitlySetPaddingRight(builderState.IsAuthorOrigin());
  }

  inline void BuilderCustom::ApplyValuePaddingRight(BuilderState &builderState, css::Value &value) noexcept
  {
    builderState.Style().SetPaddingRight(ConvertCSSValueToStyle<PaddingEdge>(builderState, value));
    builderState.Style().SetHasExplicitlySetPaddingRight(builderState.IsAuthorOrigin());
  }

  inline void BuilderCustom::ApplyInitialPaddingTop(BuilderState &builderState) noexcept
  {
    builderState.Style().SetPaddingTop(style::ComputedStyle::InitialPaddingTop());
    builderState.Style().SetHasExplicitlySetPaddingTop(builderState.IsAuthorOrigin());
  }

  inline void BuilderCustom::ApplyInheritPaddingTop(BuilderState &builderState) noexcept
  {
    builderState.Style().SetPaddingTop(ForwardInheritedValue(builderState.ParentStyle().PaddingTop()));
    builderState.Style().SetHasExplicitlySetPaddingTop(builderState.IsAuthorOrigin());
  }

  inline void BuilderCustom::ApplyValuePaddingTop(BuilderState &builderState, css::Value &value) noexcept
  {
    builderState.Style().SetPaddingTop(ConvertCSSValueToStyle<PaddingEdge>(builderState, value));
    builderState.Style().SetHasExplicitlySetPaddingTop(builderState.IsAuthorOrigin());
  }

#undef KRYS_DECLARE_PROPERTY_CUSTOM_HANDLERS
}
