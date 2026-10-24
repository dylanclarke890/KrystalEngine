#pragma once

#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/Rendering/Style/CounterDirectives.hpp"
#include "Krystal.Booey/Style/NameScope.hpp"
#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Booey/Style/Values/Align/GapGutter.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/AnchorName.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/PositionAnchor.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/PositionArea.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/PositionTryFallbacks.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/PositionVisibility.hpp"
#include "Krystal.Booey/Style/Values/Box/MarginTrim.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Contain/Contain.hpp"
#include "Krystal.Booey/Style/Values/Contain/ContainerName.hpp"
#include "Krystal.Booey/Style/Values/Grid/ItemTolerance.hpp"
#include "Krystal.Booey/Style/Values/Inline/WebKitInitialLetter.hpp"
#include "Krystal.Booey/Style/Values/Masking/Clip.hpp"
#include "Krystal.Booey/Style/Values/Masking/ClipPath.hpp"
#include "Krystal.Booey/Style/Values/Masking/MaskBorder.hpp"
#include "Krystal.Booey/Style/Values/Motion/OffsetAnchor.hpp"
#include "Krystal.Booey/Style/Values/Motion/OffsetDistance.hpp"
#include "Krystal.Booey/Style/Values/Motion/OffsetPath.hpp"
#include "Krystal.Booey/Style/Values/Motion/OffsetPosition.hpp"
#include "Krystal.Booey/Style/Values/Motion/OffsetRotate.hpp"
#include "Krystal.Booey/Style/Values/Overflow/MaximumLines.hpp"
#include "Krystal.Booey/Style/Values/Overflow/ScrollbarGutter.hpp"
#include "Krystal.Booey/Style/Values/Overflow/ScrollBehavior.hpp"
#include "Krystal.Booey/Style/Values/Page/PageSize.hpp"
#include "Krystal.Booey/Style/Values/PointerEvents/TouchAction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Rhythm/BlockStepSize.hpp"
#include "Krystal.Booey/Style/Values/ScrollAnimations/ProgressTimelineAxes.hpp"
#include "Krystal.Booey/Style/Values/ScrollAnimations/ProgressTimelineName.hpp"
#include "Krystal.Booey/Style/Values/ScrollAnimations/ScrollTimelines.hpp"
#include "Krystal.Booey/Style/Values/ScrollAnimations/ViewTimelineInsets.hpp"
#include "Krystal.Booey/Style/Values/ScrollAnimations/ViewTimelines.hpp"
#include "Krystal.Booey/Style/Values/Scrollbars/ScrollbarWidth.hpp"
#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollMargin.hpp"
#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollPadding.hpp"
#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollSnapAlign.hpp"
#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollSnapType.hpp"
#include "Krystal.Booey/Style/Values/Shapes/ShapeImageThreshold.hpp"
#include "Krystal.Booey/Style/Values/Shapes/ShapeMargin.hpp"
#include "Krystal.Booey/Style/Values/Shapes/ShapeOutside.hpp"
#include "Krystal.Booey/Style/Values/Sizing/ContainIntrinsicSize.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextDecorationThickness.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Perspective.hpp"
#include "Krystal.Booey/Style/Values/Transforms/PerspectiveOrigin.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Rotate.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Scale.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Translate.hpp"
#include "Krystal.Booey/Style/Values/Viewport/Zoom.hpp"
#include "Krystal.Booey/Style/Values/ViewTransitions/ViewTransitionClass.hpp"
#include "Krystal.Booey/Style/Values/ViewTransitions/ViewTransitionName.hpp"
#include "Krystal.Booey/Style/Values/WillChange/WillChange.hpp"
#include "Krystal.Core/Types/DataRef.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include <memory>

namespace krys::boo
{
  using namespace css::Literals;

  class PathOperation;
  class StyleBackdropFilterData;
  class StyleCustomPropertyData;
  class StyleFlexibleBoxData;
  class StyleGridData;
  class StyleGridItemData;
  class StyleMaskBorderData;
  class StyleMultiColData;
  class StyleResolver;
  class StyleTransformData;

  namespace style
  {
    class CustomPropertyData;
  }

  // This struct is for rarely used non-inherited CSS3, CSS2 and non-standard properties.
  // By grouping them together, we save space, and only allocate this object when someone
  // actually uses one of these properties.
  class StyleRareNonInheritedData : public RefCounted<StyleRareNonInheritedData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleRareNonInheritedData> Create() noexcept
    {
      return AdoptRef(*new StyleRareNonInheritedData);
    }
    KRYS_NODISCARD Ref<StyleRareNonInheritedData> Copy() const noexcept;
    ~StyleRareNonInheritedData() noexcept;

    bool operator==(const StyleRareNonInheritedData &) const noexcept;

    KRYS_NODISCARD style::Contain UsedContain() const noexcept;

    style::ContainIntrinsicSize ContainIntrinsicWidth;
    style::ContainIntrinsicSize ContainIntrinsicHeight;

    style::Zoom Zoom;

    style::MaximumLines MaxLines;

    style::TouchAction TouchAction;

    style::WebkitInitialLetter InitialLetter;

    DataRef<StyleBackdropFilterData> BackdropFilter;
    DataRef<StyleGridData> Grid;
    DataRef<StyleGridItemData> GridItem;
    DataRef<StyleMaskBorderData> MaskBorder;

    style::Clip Clip {css::keywords::Auto {}};

    style::ScrollMarginBox ScrollMargin {0_css_px};
    style::ScrollPaddingBox ScrollPadding {css::keywords::Auto {}};

    CounterDirectiveMap CounterDirectives;

    style::WillChange WillChange;

    style::PageSize PageSize;

    style::ShapeOutside ShapeOutside;
    style::ShapeMargin ShapeMargin;
    style::ShapeImageThreshold ShapeImageThreshold;

    style::Perspective Perspective;
    style::PerspectiveOrigin PerspectiveOrigin;

    style::ClipPath ClipPath;

    DataRef<style::CustomPropertyData> CustomProperties;
    HashSet<css::CSSOMStringAtom> CustomPaintWatchedProperties;

    style::Rotate Rotate;
    style::Scale Scale;
    style::Translate Translate;

    style::ContainerNames ContainerNames;

    style::ViewTransitionClasses ViewTransitionClasses;
    style::ViewTransitionName ViewTransitionName;

    style::GapGutter ColumnGap;
    style::GapGutter RowGap;

    style::ItemTolerance ItemTolerance;

    style::OffsetPath OffsetPath;
    style::OffsetDistance OffsetDistance;
    style::OffsetPosition OffsetPosition;
    style::OffsetAnchor OffsetAnchor;
    style::OffsetRotate OffsetRotate;

    style::Color TextDecorationColor;
    style::TextDecorationThickness TextDecorationThickness;

    style::ScrollTimelines ScrollTimelines;
    style::ProgressTimelineAxes ScrollTimelineAxes;
    style::ProgressTimelineNames ScrollTimelineNames;

    style::ViewTimelines ViewTimelines;
    style::ViewTimelineInsets ViewTimelineInsets;
    style::ProgressTimelineAxes ViewTimelineAxes;
    style::ProgressTimelineNames ViewTimelineNames;

    style::NameScope TimelineScope;

    style::ScrollbarGutter ScrollbarGutter;

    style::ScrollSnapType ScrollSnapType;
    style::ScrollSnapAlign ScrollSnapAlign;

    css::CSSOMStringAtom PseudoElementNameArgument;

    style::AnchorNames AnchorNames;
    style::NameScope AnchorScope;
    style::PositionAnchor PositionAnchor;
    style::PositionArea PositionArea;
    style::PositionTryFallbacks PositionTryFallbacks;
    Maybe<size_t> UsedPositionOptionIndex;

    style::BlockStepSize BlockStepSize;
    unsigned BlockStepAlign : 2;
    unsigned BlockStepInsert : 2;
    unsigned BlockStepRound : 2;

    unsigned OverscrollBehaviorX : 2;
    unsigned OverscrollBehaviorY : 2;

    unsigned TransformStyle3D : 2;
    unsigned TransformStyleForcedToFlat
      : 1; // The used value for transform-style is forced to flat by a grouping property.
    unsigned BackfaceVisibility : 1;

    unsigned ScrollBehavior : 1;
    unsigned TextDecorationStyle : 3;
    unsigned TextGroupAlign : 3;
    unsigned ContentVisibility : 2;
    unsigned EffectiveBlendMode : 5;
    unsigned Isolation : 1;
    unsigned Overlay : 1;
    unsigned InputSecurity : 1;
    unsigned BreakBefore : 4;
    unsigned BreakAfter : 4;
    unsigned BreakInside : 3;
    unsigned ContainerType : 2;
    unsigned TextBoxTrim : 2;
    unsigned OverflowAnchor : 1;
    unsigned PositionTryOrder : 3;
    unsigned PositionVisibility : 3;
    unsigned FieldSizing : 1;
    unsigned NativeAppearanceDisabled : 1;
    unsigned ScrollbarWidth : 2;
    unsigned UsesAnchorFunctions : 1;
    unsigned AnchorFunctionScrollCompensatedAxes : 2;
    unsigned UsesTreeCountingFunctions : 1;
    unsigned IsPopoverInvoker : 1;
    unsigned UseSVGZoomRulesForLength : 1;
    unsigned MarginTrim : 4;
    unsigned Contain : 5;
    unsigned OverflowContinue : 1;
    unsigned ScrollSnapStop : 1;

  private:
    StyleRareNonInheritedData() noexcept;
    StyleRareNonInheritedData(const StyleRareNonInheritedData &) noexcept;
  };
}
