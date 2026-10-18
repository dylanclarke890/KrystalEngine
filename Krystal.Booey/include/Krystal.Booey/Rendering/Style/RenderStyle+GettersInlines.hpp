#pragma once

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"

#define RENDER_STYLE_PROPERTIES_GETTERS_INLINES_INCLUDE_TRAP 1
#include "Krystal.Booey/Rendering/Style/RenderStyleProperties+GettersInlines.hpp"
#undef RENDER_STYLE_PROPERTIES_GETTERS_INLINES_INCLUDE_TRAP

namespace krys::boo
{
  // MARK: - Comparisons

  inline bool RenderStyle::operator==(const RenderStyle &other) const noexcept
  {
    return _computedStyle == other._computedStyle;
  }

  inline bool RenderStyle::InheritedEqual(const RenderStyle &other) const noexcept
  {
    return _computedStyle.InheritedEqual(other._computedStyle);
  }

  inline bool RenderStyle::NonInheritedEqual(const RenderStyle &other) const noexcept
  {
    return _computedStyle.NonInheritedEqual(other._computedStyle);
  }

  inline bool RenderStyle::FastPathInheritedEqual(const RenderStyle &other) const noexcept
  {
    return _computedStyle.FastPathInheritedEqual(other._computedStyle);
  }

  inline bool RenderStyle::NonFastPathInheritedEqual(const RenderStyle &other) const noexcept
  {
    return _computedStyle.NonFastPathInheritedEqual(other._computedStyle);
  }

  inline bool
    RenderStyle::DescendantAffectingNonInheritedPropertiesEqual(const RenderStyle &other) const noexcept
  {
    return _computedStyle.DescendantAffectingNonInheritedPropertiesEqual(other._computedStyle);
  }

  inline bool RenderStyle::BorderAndBackgroundEqual(const RenderStyle &other) const noexcept
  {
    return _computedStyle.BorderAndBackgroundEqual(other._computedStyle);
  }

  inline bool RenderStyle::ContainerTypeAndNamesEqual(const RenderStyle &other) const noexcept
  {
    return _computedStyle.ContainerTypeAndNamesEqual(other._computedStyle);
  }

  inline bool RenderStyle::ColumnSpanEqual(const RenderStyle &other) const noexcept
  {
    return _computedStyle.ColumnSpanEqual(other._computedStyle);
  }

  inline bool RenderStyle::ScrollPaddingEqual(const RenderStyle &other) const noexcept
  {
    return _computedStyle.ScrollPaddingEqual(other._computedStyle);
  }

  inline bool RenderStyle::FontCascadeEqual(const RenderStyle &other) const noexcept
  {
    return _computedStyle.FontCascadeEqual(other._computedStyle);
  }

  inline bool RenderStyle::ScrollSnapDataEquivalent(const RenderStyle &other) const noexcept
  {
    return _computedStyle.ScrollSnapDataEquivalent(other._computedStyle);
  }

  // MARK: Derived values

  inline bool RenderStyle::CollapseWhiteSpace() const noexcept
  {
    return CollapseWhiteSpace(GetWhiteSpaceCollapse());
  }

  inline bool RenderStyle::PreserveNewline() const noexcept
  {
    return PreserveNewline(GetWhiteSpaceCollapse());
  }

  inline bool RenderStyle::Preserves3D() const noexcept
  {
    return UsedTransformStyle3D() == TransformStyle3D::Preserve3D;
  }

  inline bool RenderStyle::AffectsTransform() const noexcept
  {
    return HasTransform() || HasOffsetPath() || HasRotate() || HasScale() || HasTranslate();
  }

  // ignore non-standard ::-webkit-scrollbar when standard properties are in use
  inline bool RenderStyle::UsesStandardScrollbarStyle() const noexcept
  {
    return GetScrollbarWidth() != style::ScrollbarWidth::Auto || !GetScrollbarColor().IsAuto();
  }

  inline bool RenderStyle::ShouldPlaceVerticalScrollbarOnLeft() const noexcept
  {
    return !GetWritingMode().IsAnyLeftToRight();
  }

  inline bool RenderStyle::SpecifiesColumns() const noexcept
  {
    return !GetColumnCount().IsAuto() || !GetColumnWidth().IsAuto();
  }

  inline bool RenderStyle::AutoWrap() const noexcept
  {
    return GetTextWrapMode() != TextWrapMode::NoWrap;
  }

  inline bool RenderStyle::BorderBottomIsTransparent() const noexcept
  {
    return Border().Bottom().IsTransparent();
  }

  inline bool RenderStyle::BorderLeftIsTransparent() const noexcept
  {
    return Border().Left().IsTransparent();
  }

  inline bool RenderStyle::BorderRightIsTransparent() const noexcept
  {
    return Border().Right().IsTransparent();
  }

  inline bool RenderStyle::BorderTopIsTransparent() const noexcept
  {
    return Border().Top().IsTransparent();
  }

  inline bool RenderStyle::ColumnRuleIsTransparent() const noexcept
  {
    return ColumnRule().IsTransparent();
  }

  inline bool RenderStyle::HasExplicitlySetBorderRadius() const noexcept
  {
    return HasExplicitlySetBorderBottomLeftRadius() || HasExplicitlySetBorderBottomRightRadius()
           || HasExplicitlySetBorderTopLeftRadius() || HasExplicitlySetBorderTopRightRadius();
  }

  inline float RenderStyle::ComputedLineHeight() const noexcept
  {
    return _computedStyle.ComputedLineHeight();
  }

  inline float RenderStyle::ComputeLineHeight(const style::LineHeight &lineHeight) const noexcept
  {
    return _computedStyle.ComputeLineHeight(lineHeight);
  }

  // MARK: Derived used values

  inline UserModify RenderStyle::UsedUserModify() const noexcept
  {
    return EffectiveInert() ? UserModify::ReadOnly : GetUserModify();
  }

  inline PointerEvents RenderStyle::UsedPointerEvents() const noexcept
  {
    return EffectiveInert() ? PointerEvents::None : GetPointerEvents();
  }

  inline TransformStyle3D RenderStyle::UsedTransformStyle3D() const noexcept
  {
    return TransformStyleForcedToFlat() ? TransformStyle3D::Flat : GetTransformStyle3D();
  }

  inline float RenderStyle::UsedPerspective() const noexcept
  {
    return GetPerspective().UsedPerspective();
  }

  inline Visibility RenderStyle::UsedVisibility() const noexcept
  {
    if (IsForceHidden()) KRYS_UNLIKELY
    {
      return Visibility::Hidden;
    }
    return GetVisibility();
  }

  inline bool RenderStyle::BreakOnlyAfterWhiteSpace() const noexcept
  {
    return GetWhiteSpaceCollapse() == WhiteSpaceCollapse::Preserve
           || GetWhiteSpaceCollapse() == WhiteSpaceCollapse::PreserveBreaks
           || GetWhiteSpaceCollapse() == WhiteSpaceCollapse::BreakSpaces
           || GetLineBreak() == LineBreak::AfterWhiteSpace;
  }

  inline bool RenderStyle::BreakWords() const noexcept
  {
    return GetWordBreak() == WordBreak::BreakWord || GetOverflowWrap() == OverflowWrap::BreakWord
           || GetOverflowWrap() == OverflowWrap::Anywhere;
  }

  constexpr bool RenderStyle::CollapseWhiteSpace(WhiteSpaceCollapse mode) noexcept
  {
    return mode == WhiteSpaceCollapse::Collapse || mode == WhiteSpaceCollapse::PreserveBreaks;
  }

  inline bool RenderStyle::IsCollapsibleWhiteSpace(char16_t character) const noexcept
  {
    switch (character)
    {
      case ' ':
      case '\t': return CollapseWhiteSpace();
      case '\n': return !PreserveNewline();
      default:   return false;
    }
  }

  constexpr bool RenderStyle::IsDisplayBlockType(DisplayType display) noexcept
  {
    return display == DisplayType::Block || display == DisplayType::Box || display == DisplayType::Flex
           || display == DisplayType::FlowRoot || display == DisplayType::Grid
           || display == DisplayType::GridLanes || display == DisplayType::ListItem
           || display == DisplayType::Table || display == DisplayType::RubyBlock;
  }

  constexpr bool RenderStyle::IsDisplayInlineType(DisplayType display) noexcept
  {
    return display == DisplayType::Inline || display == DisplayType::InlineBlock
           || display == DisplayType::InlineBox || display == DisplayType::InlineFlex
           || display == DisplayType::InlineGrid || display == DisplayType::InlineGridLanes
           || display == DisplayType::InlineTable || display == DisplayType::Ruby
           || display == DisplayType::RubyBase || display == DisplayType::RubyAnnotation;
  }

  constexpr bool RenderStyle::IsDisplayRegionType() const noexcept
  {
    return Display() == DisplayType::Block || Display() == DisplayType::InlineBlock
           || Display() == DisplayType::TableCell || Display() == DisplayType::TableCaption
           || Display() == DisplayType::ListItem;
  }

  constexpr bool RenderStyle::IsDisplayTableOrTablePart(DisplayType display) noexcept
  {
    return display == DisplayType::Table || display == DisplayType::InlineTable
           || display == DisplayType::TableCell || display == DisplayType::TableCaption
           || display == DisplayType::TableRowGroup || display == DisplayType::TableHeaderGroup
           || display == DisplayType::TableFooterGroup || display == DisplayType::TableRow
           || display == DisplayType::TableColumnGroup || display == DisplayType::TableColumn;
  }

  constexpr bool RenderStyle::IsInternalTableBox(DisplayType display) noexcept
  {
    // https://drafts.csswg.org/css-display-3/#layout-specific-display
    return display == DisplayType::TableCell || display == DisplayType::TableRowGroup
           || display == DisplayType::TableHeaderGroup || display == DisplayType::TableFooterGroup
           || display == DisplayType::TableRow || display == DisplayType::TableColumnGroup
           || display == DisplayType::TableColumn;
  }

  constexpr bool RenderStyle::IsRubyContainerOrInternalRubyBox(DisplayType display) noexcept
  {
    return display == DisplayType::Ruby || display == DisplayType::RubyAnnotation
           || display == DisplayType::RubyBase;
  }

  constexpr bool RenderStyle::DoesDisplayGenerateBlockContainer() const noexcept
  {
    auto display = this->Display();
    return (display == DisplayType::Block || display == DisplayType::InlineBlock
            || display == DisplayType::FlowRoot || display == DisplayType::ListItem
            || display == DisplayType::TableCell || display == DisplayType::TableCaption);
  }

  constexpr bool RenderStyle::PreserveNewline(WhiteSpaceCollapse mode) noexcept
  {
    return mode == WhiteSpaceCollapse::Preserve || mode == WhiteSpaceCollapse::PreserveBreaks
           || mode == WhiteSpaceCollapse::BreakSpaces;
  }

  KRYS_NODISCARD inline float AdjustFloatForAbsoluteZoom(float value, const RenderStyle &style) noexcept
  {
    return value / style.UsedZoom();
  }

  KRYS_NODISCARD inline int AdjustForAbsoluteZoom(int value, const RenderStyle &style) noexcept
  {
    double zoomFactor = style.UsedZoom();
    if (zoomFactor == 1)
    {
      return value;
    }
    // Needed because resolveAsLength<int> truncates (rather than rounds) when scaling up.
    if (zoomFactor > 1)
    {
      if (value < 0)
      {
        value--;
      }
      else
      {
        value++;
      }
    }

    return css::RoundForImpreciseConversion<int>(value / zoomFactor);
  }

  KRYS_NODISCARD inline LayoutSize AdjustLayoutSizeForAbsoluteZoom(LayoutSize size,
                                                                   const RenderStyle &style) noexcept
  {
    auto zoom = style.UsedZoom();
    return {size.Width() / zoom, size.Height() / zoom};
  }

  KRYS_NODISCARD inline LayoutUnit AdjustLayoutUnitForAbsoluteZoom(LayoutUnit value,
                                                                   const RenderStyle &style) noexcept
  {
    return LayoutUnit(value / style.UsedZoom());
  }

  KRYS_NODISCARD inline float ApplyZoom(float value, const RenderStyle &style) noexcept
  {
    return value * style.UsedZoom();
  }

  KRYS_NODISCARD constexpr BorderStyle CollapsedBorderStyle(BorderStyle style) noexcept
  {
    if (style == BorderStyle::Outset)
    {
      return BorderStyle::Groove;
    }
    if (style == BorderStyle::Inset)
    {
      return BorderStyle::Ridge;
    }
    return style;
  }

  inline bool RenderStyle::IsInterCharacterRubyPosition() const noexcept
  {
    auto rubyPosition = GetRubyPosition();
    return rubyPosition == RubyPosition::InterCharacter || rubyPosition == RubyPosition::LegacyInterCharacter;
  }

  KRYS_NODISCARD inline bool GeneratesBox(const RenderStyle &style) noexcept
  {
    return style.Display() != DisplayType::None && style.Display() != DisplayType::Contents;
  }

  KRYS_NODISCARD inline bool IsNonVisibleOverflow(Overflow overflow) noexcept
  {
    return overflow == Overflow::Hidden || overflow == Overflow::Scroll || overflow == Overflow::Clip;
  }

  KRYS_NODISCARD inline bool PseudoElementRendererIsNeeded(const RenderStyle *style) noexcept
  {
    return style && style->Display() != DisplayType::None && style->GetContent().IsData();
  }

  // MARK: has*() functions

  inline bool RenderStyle::HasAnimations() const noexcept
  {
    return !GetAnimations().IsInitial();
  }

  inline bool RenderStyle::HasAnimationsOrTransitions() const noexcept
  {
    return HasAnimations() || HasTransitions();
  }

  // FIXME: Rename this function.
  inline bool RenderStyle::HasAppearance() const noexcept
  {
    return Appearance() != StyleAppearance::None && Appearance() != StyleAppearance::Base;
  }

  inline bool RenderStyle::HasAppleColorFilter() const noexcept
  {
    return !GetAppleColorFilter().IsNone();
  }

  inline bool RenderStyle::HasAspectRatio() const noexcept
  {
    return GetAspectRatio().HasRatio();
  }

  inline bool RenderStyle::HasAutoLeftAndRight() const noexcept
  {
    return Left().IsAuto() && Right().IsAuto();
  }

  inline bool RenderStyle::HasAutoLengthContainIntrinsicSize() const noexcept
  {
    return ContainIntrinsicWidth().HasAuto() || ContainIntrinsicHeight().HasAuto();
  }

  inline bool RenderStyle::HasAutoTopAndBottom() const noexcept
  {
    return Top().IsAuto() && Bottom().IsAuto();
  }

  inline bool RenderStyle::HasBackdropFilter() const noexcept
  {
    return !BackdropFilter().IsNone();
  }

  inline bool RenderStyle::HasBackground() const noexcept
  {
    return VisitedDependentColor(css::PropertyId::BackgroundColor).IsVisible() || HasBackgroundImage();
  }

  inline bool RenderStyle::HasBackgroundImage() const noexcept
  {
    return style::HasImageInAnyLayer(GetBackgroundLayers());
  }

  inline bool RenderStyle::HasBlendMode() const noexcept
  {
    return GetBlendMode() != BlendMode::Normal;
  }

  inline bool RenderStyle::HasBorder() const noexcept
  {
    return Border().HasBorder();
  }

  inline bool RenderStyle::HasBorderImage() const noexcept
  {
    return Border().HasBorderImage();
  }

  inline bool RenderStyle::HasBorderImageOutsets() const noexcept
  {
    return !GetBorderImageSource().IsNone() && !GetBorderImageOutset().IsZero();
  }

  inline bool RenderStyle::HasBorderRadius() const noexcept
  {
    return Border().HasBorderRadius();
  }

  inline bool RenderStyle::HasBoxShadow() const noexcept
  {
    return !GetBoxShadow().IsNone();
  }

  inline bool RenderStyle::HasClip() const noexcept
  {
    return !GetClip().IsAuto();
  }

  inline bool RenderStyle::HasClipPath() const noexcept
  {
    return !GetClipPath().IsNone();
  }

  inline bool RenderStyle::HasContent() const noexcept
  {
    return GetContent().IsData();
  }

  inline bool RenderStyle::HasFill() const noexcept
  {
    return !Fill().IsNone();
  }

  inline bool RenderStyle::HasFilter() const noexcept
  {
    return !GetFilter().IsNone();
  }

  inline bool RenderStyle::HasInFlowPosition() const noexcept
  {
    return GetPosition() == PositionType::Relative || GetPosition() == PositionType::Sticky;
  }

  inline bool RenderStyle::HasIsolation() const noexcept
  {
    return GetIsolation() != Isolation::Auto;
  }

  inline bool RenderStyle::HasMarkers() const noexcept
  {
    return !MarkerStart().IsNone() || !MarkerMid().IsNone() || !MarkerEnd().IsNone();
  }

  inline bool RenderStyle::HasMask() const noexcept
  {
    return style::HasImageInAnyLayer(GetMaskLayers()) || !GetMaskBorderSource().IsNone();
  }

  inline bool RenderStyle::HasOffsetPath() const noexcept
  {
    return !krys::HoldsAlternative<css::keywords::None>(GetOffsetPath());
  }

  inline bool RenderStyle::HasOpacity() const noexcept
  {
    return !GetOpacity().IsOpaque();
  }

  inline bool RenderStyle::HasOutline() const noexcept
  {
    return GetOutlineStyle() != OutlineStyle::None && OutlineWidth().IsPositive();
  }

  inline bool RenderStyle::HasOutlineInVisualOverflow() const noexcept
  {
    return HasOutline() && OutlineSize() > 0;
  }

  inline bool RenderStyle::HasOutOfFlowPosition() const noexcept
  {
    return GetPosition() == PositionType::Absolute || GetPosition() == PositionType::Fixed;
  }

  inline bool RenderStyle::HasPerspective() const noexcept
  {
    return !GetPerspective().IsNone();
  }

  inline bool RenderStyle::HasPositionedMask() const noexcept
  {
    return style::HasImageInAnyLayer(GetMaskLayers());
  }

  inline bool RenderStyle::HasRotate() const noexcept
  {
    return !GetRotate().IsNone();
  }

  inline bool RenderStyle::HasScale() const noexcept
  {
    return !GetScale().IsNone();
  }

  inline bool RenderStyle::HasScrollTimelines() const noexcept
  {
    return !GetScrollTimelines().isEmpty() || !ScrollTimelineNames().IsNone();
  }

  inline bool RenderStyle::HasSnapPosition() const noexcept
  {
    return !GetScrollSnapAlign().IsNone();
  }

  inline bool RenderStyle::HasStaticBlockPosition(bool horizontal) const noexcept
  {
    return horizontal ? HasAutoTopAndBottom() : HasAutoLeftAndRight();
  }

  inline bool RenderStyle::HasStaticInlinePosition(bool horizontal) const noexcept
  {
    return horizontal ? HasAutoLeftAndRight() : HasAutoTopAndBottom();
  }

  inline bool RenderStyle::HasStroke() const noexcept
  {
    return !Stroke().IsNone();
  }

  inline bool RenderStyle::HasTextCombine() const noexcept
  {
    return GetTextCombine() != TextCombine::None;
  }

  inline bool RenderStyle::HasTextShadow() const noexcept
  {
    return !GetTextShadow().IsNone();
  }

  inline bool RenderStyle::HasTransform() const noexcept
  {
    return !GetTransform().IsNone() || HasOffsetPath();
  }

  inline bool RenderStyle::HasTransformRelatedProperty() const noexcept
  {
    return HasTransform() || HasRotate() || HasScale() || HasTranslate()
           || GetTransformStyle3D() == TransformStyle3D::Preserve3D || HasPerspective();
  }

  inline bool RenderStyle::HasTransitions() const noexcept
  {
    return !GetTransitions().IsInitial();
  }

  inline bool RenderStyle::HasTranslate() const noexcept
  {
    return !GetTranslate().IsNone();
  }

  inline bool RenderStyle::HasUsedAppearance() const noexcept
  {
    return UsedAppearance() != StyleAppearance::None && UsedAppearance() != StyleAppearance::Base;
  }

  inline bool RenderStyle::HasUsedContentNone() const noexcept
  {
    return GetContent().IsNone()
           || (GetContent().IsNormal()
               && (GetPseudoElementType() == PseudoElementType::Before
                   || GetPseudoElementType() == PseudoElementType::After));
  }

  inline bool RenderStyle::HasViewportConstrainedPosition() const noexcept
  {
    return GetPosition() == PositionType::Fixed || GetPosition() == PositionType::Sticky;
  }

  inline bool RenderStyle::HasViewTimelines() const noexcept
  {
    return !GetViewTimelines().isEmpty() || !ViewTimelineNames().IsNone();
  }

  inline bool RenderStyle::HasVisibleBorder() const noexcept
  {
    return Border().HasVisibleBorder();
  }

  inline bool RenderStyle::HasVisibleBorderDecoration() const noexcept
  {
    return HasVisibleBorder() || HasBorderImage();
  }

  inline bool RenderStyle::HasPositiveStrokeWidth() const noexcept
  {
    // Without an explicit `stroke-width`, text is stroked with the initial `-webkit-text-stroke-width`, which
    // isn't positive.
    if (!HasExplicitlySetStrokeWidth())
    {
      return false;
    }
    return GetStrokeWidth().IsPossiblyPositive();
  }

  // MARK: is*() functions

  inline bool RenderStyle::IsColumnFlexDirection() const noexcept
  {
    return GetFlexDirection() == FlexDirection::Column || GetFlexDirection() == FlexDirection::ColumnReverse;
  }

  inline bool RenderStyle::IsRowFlexDirection() const noexcept
  {
    return GetFlexDirection() == FlexDirection::Row || GetFlexDirection() == FlexDirection::RowReverse;
  }

  constexpr bool RenderStyle::IsDisplayBlockLevel() const noexcept
  {
    return IsDisplayBlockType(Display());
  }

  constexpr bool RenderStyle::IsDisplayDeprecatedFlexibleBox(DisplayType display) noexcept
  {
    return display == DisplayType::Box || display == DisplayType::InlineBox;
  }

  constexpr bool RenderStyle::IsDisplayFlexibleBox(DisplayType display) noexcept
  {
    return display == DisplayType::Flex || display == DisplayType::InlineFlex;
  }

  constexpr bool RenderStyle::IsDisplayDeprecatedFlexibleBox() const noexcept
  {
    return IsDisplayDeprecatedFlexibleBox(Display());
  }

  constexpr bool
    RenderStyle::IsDisplayFlexibleBoxIncludingDeprecatedOrGridFormattingContextBox() const noexcept
  {
    return IsDisplayFlexibleOrGridFormattingContextBox() || IsDisplayDeprecatedFlexibleBox();
  }

  constexpr bool RenderStyle::IsDisplayFlexibleOrGridFormattingContextBox() const noexcept
  {
    return IsDisplayFlexibleOrGridFormattingContextBox(Display());
  }

  constexpr bool RenderStyle::IsDisplayFlexibleOrGridFormattingContextBox(DisplayType display) noexcept
  {
    return IsDisplayFlexibleBox(display) || IsDisplayGridFormattingContextBox(display);
  }

  constexpr bool RenderStyle::IsDisplayGridFormattingContextBox(DisplayType display) noexcept
  {
    return IsDisplayGridBox(display) || IsDisplayGridLanesBox(display);
  }

  constexpr bool RenderStyle::IsDisplayGridBox(DisplayType display) noexcept
  {
    return display == DisplayType::Grid || display == DisplayType::InlineGrid;
  }

  constexpr bool RenderStyle::IsDisplayGridLanesBox(DisplayType display) noexcept
  {
    return display == DisplayType::GridLanes || display == DisplayType::InlineGridLanes;
  }

  constexpr bool RenderStyle::IsDisplayInlineType() const noexcept
  {
    return IsDisplayInlineType(Display());
  }

  constexpr bool RenderStyle::IsDisplayListItemType(DisplayType display) noexcept
  {
    return display == DisplayType::ListItem;
  }

  constexpr bool RenderStyle::IsDisplayTableOrTablePart() const noexcept
  {
    return IsDisplayTableOrTablePart(Display());
  }

  constexpr bool RenderStyle::IsInternalTableBox() const noexcept
  {
    return IsInternalTableBox(Display());
  }

  constexpr bool RenderStyle::IsRubyContainerOrInternalRubyBox() const noexcept
  {
    return IsRubyContainerOrInternalRubyBox(Display());
  }

  inline bool RenderStyle::IsFixedTableLayout() const noexcept
  {
    return GetTableLayout() == TableLayoutType::Fixed
           && (LogicalWidth().IsSpecified() || LogicalWidth().IsFitContent()
               || LogicalWidth().IsFillAvailable() || LogicalWidth().IsMinContent());
  }

  inline bool RenderStyle::IsFloating() const noexcept
  {
    return Floating() != Float::None;
  }

  constexpr bool RenderStyle::IsOriginalDisplayBlockType() const noexcept
  {
    return IsDisplayBlockType(OriginalDisplay());
  }

  constexpr bool RenderStyle::IsOriginalDisplayInlineType() const noexcept
  {
    return IsDisplayInlineType(OriginalDisplay());
  }

  constexpr bool RenderStyle::IsOriginalDisplayListItemType() const noexcept
  {
    return IsDisplayListItemType(OriginalDisplay());
  }

  inline bool RenderStyle::IsOverflowVisible() const noexcept
  {
    return OverflowX() == Overflow::Visible || OverflowY() == Overflow::Visible;
  }

  inline bool RenderStyle::IsReverseFlexDirection() const noexcept
  {
    return GetFlexDirection() == FlexDirection::RowReverse
           || GetFlexDirection() == FlexDirection::ColumnReverse;
  }

  inline bool RenderStyle::IsSkippedRootOrSkippedContent() const noexcept
  {
    return UsedContentVisibility() != ContentVisibility::Visible;
  }

  // MARK: - Logical getters

  // MARK: logical inset value aliases

  inline const style::InsetEdge &RenderStyle::LogicalTop() const noexcept
  {
    return InsetBefore();
  }

  inline const style::InsetEdge &RenderStyle::LogicalRight() const noexcept
  {
    return InsetLogicalRight();
  }

  inline const style::InsetEdge &RenderStyle::LogicalBottom() const noexcept
  {
    return InsetAfter();
  }

  inline const style::InsetEdge &RenderStyle::LogicalLeft() const noexcept
  {
    return InsetLogicalLeft();
  }

  // MARK: logical aggregate border values

  inline const BorderValue &RenderStyle::BorderBefore() const noexcept
  {
    return BorderBefore(GetWritingMode());
  }

  inline const BorderValue &RenderStyle::BorderAfter() const noexcept
  {
    return BorderAfter(GetWritingMode());
  }

  inline const BorderValue &RenderStyle::BorderStart() const noexcept
  {
    return BorderStart(GetWritingMode());
  }

  inline const BorderValue &RenderStyle::BorderEnd() const noexcept
  {
    return BorderEnd(GetWritingMode());
  }

  // MARK: logical aspect-ratio values

  inline style::Number<css::NonNegative> RenderStyle::AspectRatioLogicalHeight() const noexcept
  {
    return GetWritingMode().IsHorizontal() ? GetAspectRatio().Height() : GetAspectRatio().Width();
  }

  inline style::Number<css::NonNegative> RenderStyle::AspectRatioLogicalWidth() const noexcept
  {
    return GetWritingMode().IsHorizontal() ? GetAspectRatio().Width() : GetAspectRatio().Height();
  }

  inline double RenderStyle::LogicalAspectRatio() const noexcept
  {
    auto ratio = GetAspectRatio().TryRatio();
    krys_debug_assert(ratio);

    if (GetWritingMode().IsHorizontal())
    {
      return ratio->Numerator.Value / ratio->Denominator.Value;
    }
    return ratio->Denominator.Value / ratio->Numerator.Value;
  }

  inline BoxSizing RenderStyle::BoxSizingForAspectRatio() const noexcept
  {
    return GetAspectRatio().IsAutoAndRatio() ? BoxSizing::ContentBox : GetBoxSizing();
  }

  // MARK: logical grid values

  inline const style::GapGutter &RenderStyle::Gap(style::GridTrackSizingDirection direction) const noexcept
  {
    return direction == style::GridTrackSizingDirection::Columns ? ColumnGap() : RowGap();
  }

  inline const style::GridTrackSizes &
    RenderStyle::GridAutoList(style::GridTrackSizingDirection direction) const noexcept
  {
    return direction == style::GridTrackSizingDirection::Columns ? GridAutoColumns() : GridAutoRows();
  }

  inline const style::GridPosition &
    RenderStyle::GridItemEnd(style::GridTrackSizingDirection direction) const noexcept
  {
    return direction == style::GridTrackSizingDirection::Columns ? GridItemColumnEnd() : GridItemRowEnd();
  }

  inline const style::GridPosition &
    RenderStyle::GridItemStart(style::GridTrackSizingDirection direction) const noexcept
  {
    return direction == style::GridTrackSizingDirection::Columns ? GridItemColumnStart() : GridItemRowStart();
  }

  inline const style::GridTemplateList &
    RenderStyle::GridTemplateList(style::GridTrackSizingDirection direction) const noexcept
  {
    return direction == style::GridTrackSizingDirection::Columns ? GridTemplateColumns() : GridTemplateRows();
  }
}
