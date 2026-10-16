#include "Krystal.Booey/Style/Difference.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Extractor.hpp"
#include "Krystal.Booey/Style/InlineTextBoxStyle.hpp"

namespace krys::boo::style
{
  // MARK: DifferenceResult::Layout

  static bool PositionChangeIsMovementOnly(const InsetBox &a, const InsetBox &b,
                                           const PreferredSize &width) noexcept
  {
    // If any unit types are different, then we can't guarantee
    // that this was just a movement.
    if (!a.Left().HasSameType(b.Left()) || !a.Right().HasSameType(b.Right()) || !a.Top().HasSameType(b.Top())
        || !a.Bottom().HasSameType(b.Bottom()))
    {
      return false;
    }

    // Only one unit can be non-auto in the horizontal direction and
    // in the vertical direction.  Otherwise the adjustment of values
    // is changing the size of the box.
    if (!a.Left().IsAuto() && !a.Right().IsAuto())
    {
      return false;
    }
    if (!a.Top().IsAuto() && !a.Bottom().IsAuto())
    {
      return false;
    }
    // If our width is auto and left or right is specified then this
    // is not just a movement - we need to resize to our container.
    if ((!a.Left().IsAuto() || !a.Right().IsAuto()) && width.IsIntrinsicOrLegacyIntrinsicOrAuto())
    {
      return false;
    }

    // One of the units is fixed or percent in both directions and stayed
    // that way in the new style.  Therefore all we are doing is moving.
    return true;
  }

  static bool ChangeAffectsVisualOverflow(const RenderStyle &a, const RenderStyle &b) noexcept
  {
    auto nonInheritedDataChangeAffectsVisualOverflow = [&]
    {
      if (&a.NonInheritedData() == &b.NonInheritedData())
      {
        return false;
      }

      if (a.NonInheritedData().MiscData.Ptr() != b.NonInheritedData().MiscData.Ptr()
          && a.NonInheritedData().MiscData->BoxShadow != b.NonInheritedData().MiscData->BoxShadow)
      {
        return true;
      }

      if (a.NonInheritedData().BackgroundData.Ptr() != b.NonInheritedData().BackgroundData.Ptr())
      {
        auto aHasOutlineInVisualOverflow = a.HasOutlineInVisualOverflow();
        auto bHasOutlineInVisualOverflow = b.HasOutlineInVisualOverflow();
        if (aHasOutlineInVisualOverflow != bHasOutlineInVisualOverflow
            || (aHasOutlineInVisualOverflow && bHasOutlineInVisualOverflow
                && a.OutlineSize() != b.OutlineSize()))
        {
          return true;
        }
      }

      return false;
    };

    auto textDecorationsDiffer = [&]
    {
      if (a.GetInheritedFlags().TextDecorationLineInEffect
          != b.GetInheritedFlags().TextDecorationLineInEffect)
      {
        return true;
      }

      if (&a.NonInheritedData() != &b.NonInheritedData()
          && a.NonInheritedData().RareData.Ptr() != b.NonInheritedData().RareData.Ptr())
      {
        if (a.NonInheritedData().RareData->TextDecorationStyle
              != b.NonInheritedData().RareData->TextDecorationStyle
            || a.NonInheritedData().RareData->TextDecorationThickness
                 != b.NonInheritedData().RareData->TextDecorationThickness)
        {
          return true;
        }
      }

      if (&a.RareInheritedData() != &b.RareInheritedData())
      {
        if (a.RareInheritedData().TextUnderlineOffset != b.RareInheritedData().TextUnderlineOffset
            || a.RareInheritedData().TextUnderlinePosition != b.RareInheritedData().TextUnderlinePosition)
        {
          return true;
        }
      }

      return false;
    };

    if (nonInheritedDataChangeAffectsVisualOverflow())
    {
      return true;
    }

    if (&a.RareInheritedData() != &b.RareInheritedData()
        && a.RareInheritedData().TextShadow != b.RareInheritedData().TextShadow)
    {
      return true;
    }

    if (textDecorationsDiffer())
    {
      // Underlines are always drawn outside of their textbox bounds when text-underline-position: under;
      // is specified. We can take an early out here.
      if (IsAlignedForUnder(a) || IsAlignedForUnder(b))
      {
        return true;
      }

      if (InkOverflowForDecorations(a).DiffersFrom(InkOverflowForDecorations(b)))
      {
        return true;
      }
    }

    return false;
  }

  static bool SvgDataChangeRequiresLayout(const SVGRenderStyle &a, const SVGRenderStyle &b) noexcept
  {
    // Markers influence layout, as marker boundaries are cached in RenderSVGPath.
    if (a.InheritedResourceData != b.InheritedResourceData)
    {
      return true;
    }

    // All text related properties influence layout.
    if (a.InheritedFlags.TextAnchor != b.InheritedFlags.TextAnchor
        || a.InheritedFlags.GlyphOrientationHorizontal != b.InheritedFlags.GlyphOrientationHorizontal
        || a.InheritedFlags.GlyphOrientationVertical != b.InheritedFlags.GlyphOrientationVertical
        || a.NonInheritedFlags.AlignmentBaseline != b.NonInheritedFlags.AlignmentBaseline
        || a.NonInheritedFlags.DominantBaseline != b.NonInheritedFlags.DominantBaseline)
    {
      return true;
    }

    // Text related properties influence layout.
    if (a.MiscData->BaselineShift != b.MiscData->BaselineShift)
    {
      return true;
    }

    // The x and y properties influence layout.
    if (a.LayoutData != b.LayoutData)
    {
      return true;
    }

    // Some stroke properties influence layout, as the cached stroke boundaries need to be recalculated.
    if (!a.StrokeData->Stroke.HasSameType(b.StrokeData->Stroke)
        || a.StrokeData->Stroke.UrlDisregardingType() != b.StrokeData->Stroke.UrlDisregardingType()
        || a.StrokeData->StrokeDashArray != b.StrokeData->StrokeDashArray
        || a.StrokeData->StrokeDashOffset != b.StrokeData->StrokeDashOffset
        || !a.StrokeData->VisitedLinkStroke.HasSameType(b.StrokeData->VisitedLinkStroke)
        || a.StrokeData->VisitedLinkStroke.UrlDisregardingType()
             != b.StrokeData->VisitedLinkStroke.UrlDisregardingType())
    {
      return true;
    }

    // vector-effect influences layout.
    if (a.NonInheritedFlags.VectorEffect != b.NonInheritedFlags.VectorEffect)
    {
      return true;
    }

    return false;
  }

  static bool MiscDataChangeRequiresLayout(
    const StyleMiscNonInheritedData &a, const StyleMiscNonInheritedData &b,
    EnumSet<DifferenceContextSensitiveProperty> &changedContextSensitiveProperties) noexcept
  {
    krys_debug_assert(&a != &b);

    if (a.UsedAppearance != b.UsedAppearance || a.TextOverflow != b.TextOverflow)
    {
      return true;
    }

    if (a.FlexibleBox != b.FlexibleBox)
    {
      return true;
    }

    if (a.Order != b.Order || a.AlignContent != b.AlignContent || a.AlignItems != b.AlignItems
        || a.AlignSelf != b.AlignSelf || a.JustifyContent != b.JustifyContent
        || a.JustifyItems != b.JustifyItems || a.JustifySelf != b.JustifySelf)
    {
      return true;
    }

    if (a.MultiCol != b.MultiCol)
    {
      return true;
    }

    if (a.Transform.Ptr() != b.Transform.Ptr())
    {
      if (a.Transform->HasTransform() != b.Transform->HasTransform())
      {
        return true;
      }
      if (*a.Transform != *b.Transform)
      {
        changedContextSensitiveProperties.Add(DifferenceContextSensitiveProperty::Transform);
        // Don't return; keep looking for another change
      }
    }

    if (a.Opacity.IsOpaque() != b.Opacity.IsOpaque())
    {
      // FIXME: We would like to use SimplifiedLayout here, but we can't quite do that yet.
      // We need to make sure SimplifiedLayout can operate correctly on RenderInlines (we will need
      // to add a selfNeedsSimplifiedLayout bit in order to not get confused and taint every line).
      // In addition we need to solve the floating object issue when layers come and go. Right now
      // a full layout is necessary to keep floating object lists sane.
      return true;
    }

    if (a.HasFilters() != b.HasFilters())
    {
      return true;
    }

    if (a.AspectRatio != b.AspectRatio)
    {
      return true;
    }

    return false;
  }

  static bool RareDataChangeRequiresLayout(
    const StyleRareNonInheritedData &a, const StyleRareNonInheritedData &b,
    EnumSet<DifferenceContextSensitiveProperty> &changedContextSensitiveProperties) noexcept
  {
    krys_debug_assert(&a != &b);

    if (a.InitialLetter != b.InitialLetter)
    {
      return true;
    }

    if (a.ShapeMargin != b.ShapeMargin)
    {
      return true;
    }

    if (a.ColumnGap != b.ColumnGap || a.RowGap != b.RowGap)
    {
      return true;
    }

    // If the counter directives change, trigger a relayout to re-calculate counter values and rebuild the
    // counter node tree.
    if (a.CounterDirectives != b.CounterDirectives)
    {
      return true;
    }

    if (a.Scale != b.Scale || a.Rotate != b.Rotate || a.Translate != b.Translate)
    {
      changedContextSensitiveProperties.Add(DifferenceContextSensitiveProperty::Transform);
    }

    if (a.OffsetPath != b.OffsetPath || a.OffsetPosition != b.OffsetPosition
        || a.OffsetDistance != b.OffsetDistance || a.OffsetAnchor != b.OffsetAnchor
        || a.OffsetRotate != b.OffsetRotate)
    {
      changedContextSensitiveProperties.Add(DifferenceContextSensitiveProperty::Transform);
    }

    if (a.Grid != b.Grid || a.GridItem != b.GridItem)
    {
      return true;
    }

    if (a.WillChange != b.WillChange)
    {
      changedContextSensitiveProperties.Add(DifferenceContextSensitiveProperty::WillChange);
      // Don't return; keep looking for another change
    }

    if (a.BreakBefore != b.BreakBefore || a.BreakAfter != b.BreakAfter || a.BreakInside != b.BreakInside)
    {
      return true;
    }

    if (a.Isolation != b.Isolation)
    {
      // Ideally this would trigger a cheaper layout that just updates layer z-order trees
      // (webkit.org/b/190088).
      return true;
    }

    if (a.BackdropFilter->BackdropFilter.IsNone() != b.BackdropFilter->BackdropFilter.IsNone())
    {
      return true;
    }

    if (a.InputSecurity != b.InputSecurity)
    {
      return true;
    }

    if (a.UsedContain().Contains(ContainValue::Size) != b.UsedContain().Contains(ContainValue::Size)
        || a.UsedContain().Contains(ContainValue::InlineSize)
             != b.UsedContain().Contains(ContainValue::InlineSize)
        || a.UsedContain().Contains(ContainValue::Layout) != b.UsedContain().Contains(ContainValue::Layout))
    {
      return true;
    }

    // content-visibility:hidden turns on contain:size which requires relayout.
    if ((static_cast<ContentVisibility>(a.ContentVisibility) == ContentVisibility::Hidden)
        != (static_cast<ContentVisibility>(b.ContentVisibility) == ContentVisibility::Hidden))
    {
      return true;
    }

    if (a.ScrollPadding != b.ScrollPadding)
    {
      return true;
    }

    if (a.ScrollSnapType != b.ScrollSnapType)
    {
      return true;
    }

    if (a.ContainIntrinsicWidth != b.ContainIntrinsicWidth
        || a.ContainIntrinsicHeight != b.ContainIntrinsicHeight)
    {
      return true;
    }

    if (a.MarginTrim != b.MarginTrim)
    {
      return true;
    }

    if (a.ScrollbarGutter != b.ScrollbarGutter)
    {
      return true;
    }

    if (a.ScrollbarWidth != b.ScrollbarWidth)
    {
      return true;
    }

    if (a.TextBoxTrim != b.TextBoxTrim)
    {
      return true;
    }

    if (a.MaxLines != b.MaxLines)
    {
      return true;
    }

    if (a.OverflowContinue != b.OverflowContinue)
    {
      return true;
    }

    // CSS Anchor Positioning.
    if (a.AnchorScope != b.AnchorScope || a.PositionArea != b.PositionArea)
    {
      return true;
    }

    if (a.FieldSizing != b.FieldSizing)
    {
      return true;
    }

    return false;
  }

  static bool RareInheritedDataChangeRequiresLayout(const StyleRareInheritedData &a,
                                                    const StyleRareInheritedData &b) noexcept
  {
    krys_debug_assert(&a != &b);

    if (a.TextIndent != b.TextIndent || a.TextAlignLast != b.TextAlignLast || a.TextJustify != b.TextJustify
        || a.TextBoxEdge != b.TextBoxEdge || a.LineFitEdge != b.LineFitEdge || a.UsedZoom != b.UsedZoom
        || a.WordBreak != b.WordBreak || a.OverflowWrap != b.OverflowWrap || a.LineBreak != b.LineBreak
        || a.TextSecurity != b.TextSecurity || a.Hyphens != b.Hyphens
        || a.HyphenateLimitBefore != b.HyphenateLimitBefore || a.HyphenateLimitAfter != b.HyphenateLimitAfter
        || a.HyphenateCharacter != b.HyphenateCharacter || a.RubyPosition != b.RubyPosition
        || a.RubyAlign != b.RubyAlign || a.RubyOverhang != b.RubyOverhang || a.TextCombine != b.TextCombine
        || a.TextEmphasisStyle != b.TextEmphasisStyle || a.TextEmphasisPosition != b.TextEmphasisPosition
        || a.TabSize != b.TabSize || a.LineBoxContain != b.LineBoxContain
        || a.ImageOrientation != b.ImageOrientation || a.LineAlign != b.LineAlign
        || a.HangingPunctuation != b.HangingPunctuation || a.UsedContentVisibility != b.UsedContentVisibility
        || a.ListStyleType != b.ListStyleType || a.ListStyleImage != b.ListStyleImage
        || a.BlockEllipsis != b.BlockEllipsis)
    {
      return true;
    }

    // These properties affect the cached stroke bounding box rects.
    if (a.CapStyle != b.CapStyle || a.JoinStyle != b.JoinStyle || a.StrokeWidth != b.StrokeWidth
        || a.StrokeMiterLimit != b.StrokeMiterLimit)
    {
      return true;
    }

    if (a.Quotes != b.Quotes)
    {
      return true;
    }

    return false;
  }

  static bool ChangeRequiresLayout(
    const RenderStyle &a, const RenderStyle &b,
    EnumSet<DifferenceContextSensitiveProperty> &changedContextSensitiveProperties) noexcept
  {
    if (&a.SvgStyle() != &b.SvgStyle() && SvgDataChangeRequiresLayout(a.SvgStyle(), b.SvgStyle()))
    {
      return true;
    }

    if (&a.NonInheritedData() != &b.NonInheritedData())
    {
      if (a.NonInheritedData().BoxData.Ptr() != b.NonInheritedData().BoxData.Ptr())
      {
        if (a.NonInheritedData().BoxData->Width != b.NonInheritedData().BoxData->Width
            || a.NonInheritedData().BoxData->MinWidth != b.NonInheritedData().BoxData->MinWidth
            || a.NonInheritedData().BoxData->MaxWidth != b.NonInheritedData().BoxData->MaxWidth
            || a.NonInheritedData().BoxData->Height != b.NonInheritedData().BoxData->Height
            || a.NonInheritedData().BoxData->MinHeight != b.NonInheritedData().BoxData->MinHeight
            || a.NonInheritedData().BoxData->MaxHeight != b.NonInheritedData().BoxData->MaxHeight)
        {
          return true;
        }

        if (a.NonInheritedData().BoxData->VerticalAlign != b.NonInheritedData().BoxData->VerticalAlign)
        {
          return true;
        }

        if (a.NonInheritedData().BoxData->BoxSizing != b.NonInheritedData().BoxData->BoxSizing)
        {
          return true;
        }

        if (a.NonInheritedData().BoxData->HasAutoUsedZIndex
            != b.NonInheritedData().BoxData->HasAutoUsedZIndex)
        {
          return true;
        }
      }

      if (a.NonInheritedData().SurroundData.Ptr() != b.NonInheritedData().SurroundData.Ptr())
      {
        if (a.NonInheritedData().SurroundData->Margin != b.NonInheritedData().SurroundData->Margin)
        {
          return true;
        }

        if (a.NonInheritedData().SurroundData->Padding != b.NonInheritedData().SurroundData->Padding)
        {
          return true;
        }

        // If our border widths change, then we need to layout. Other changes to borders only necessitate a
        // repaint.
        if (a.BorderLeftWidth() != b.BorderLeftWidth() || a.BorderTopWidth() != b.BorderTopWidth()
            || a.BorderBottomWidth() != b.BorderBottomWidth() || a.BorderRightWidth() != b.BorderRightWidth())
        {
          return true;
        }

        if (a.GetPosition() != PositionType::Static)
        {
          if (a.NonInheritedData().SurroundData->Inset != b.NonInheritedData().SurroundData->Inset)
          {
            // FIXME: We would like to use SimplifiedLayout for relative positioning, but we can't quite do
            // that yet. We need to make sure SimplifiedLayout can operate correctly on RenderInlines (we will
            // need to add a selfNeedsSimplifiedLayout bit in order to not get confused and taint every line).
            if (a.GetPosition() != PositionType::Absolute)
            {
              return true;
            }

            // Optimize for the case where a positioned layer is moving but not changing size.
            if (!PositionChangeIsMovementOnly(a.NonInheritedData().SurroundData->Inset,
                                              b.NonInheritedData().SurroundData->Inset,
                                              a.NonInheritedData().BoxData->Width))
            {
              return true;
            }
          }
        }
      }
    }

    // FIXME: We should add an optimized form of layout that just recomputes visual overflow.
    if (ChangeAffectsVisualOverflow(a, b))
    {
      return true;
    }

    if (&a.NonInheritedData() != &b.NonInheritedData())
    {
      if (a.NonInheritedData().MiscData.Ptr() != b.NonInheritedData().MiscData.Ptr()
          && MiscDataChangeRequiresLayout(*a.NonInheritedData().MiscData, *b.NonInheritedData().MiscData,
                                          changedContextSensitiveProperties))
      {
        return true;
      }

      if (a.NonInheritedData().RareData.Ptr() != b.NonInheritedData().RareData.Ptr()
          && RareDataChangeRequiresLayout(*a.NonInheritedData().RareData, *b.NonInheritedData().RareData,
                                          changedContextSensitiveProperties))
      {
        return true;
      }
    }

    if (&a.RareInheritedData() != &b.RareInheritedData()
        && RareInheritedDataChangeRequiresLayout(a.RareInheritedData(), b.RareInheritedData()))
    {
      return true;
    }

    if (&a.InheritedData() != &b.InheritedData())
    {
      if (a.InheritedData().LineHeight != b.InheritedData().LineHeight
          || a.InheritedData().BorderHorizontalSpacing != b.InheritedData().BorderHorizontalSpacing
          || a.InheritedData().BorderVerticalSpacing != b.InheritedData().BorderVerticalSpacing)
      {
        return true;
      }

      if (a.InheritedData().FontData != b.InheritedData().FontData)
      {
        return true;
      }
    }

    if (a.GetInheritedFlags().RtlOrdering != b.GetInheritedFlags().RtlOrdering
        || a.GetNonInheritedFlags().Position != b.GetNonInheritedFlags().Position
        || a.GetNonInheritedFlags().Floating != b.GetNonInheritedFlags().Floating
        || a.GetNonInheritedFlags().OriginalDisplay != b.GetNonInheritedFlags().OriginalDisplay)
    {
      return true;
    }

    if (static_cast<DisplayType>(a.GetNonInheritedFlags().EffectiveDisplay) >= DisplayType::Table)
    {
      if (a.GetInheritedFlags().BorderCollapse != b.GetInheritedFlags().BorderCollapse
          || a.GetInheritedFlags().EmptyCells != b.GetInheritedFlags().EmptyCells
          || a.GetInheritedFlags().CaptionSide != b.GetInheritedFlags().CaptionSide
          || a.TableLayout() != b.TableLayout())
      {
        return true;
      }

      // In the collapsing border model, 'hidden' suppresses other borders, while 'none'
      // does not, so these style differences can be width differences.
      if (a.GetInheritedFlags().BorderCollapse
          && ((a.BorderTopStyle() == BorderStyle::Hidden && b.BorderTopStyle() == BorderStyle::None)
              || (a.BorderTopStyle() == BorderStyle::None && b.BorderTopStyle() == BorderStyle::Hidden)
              || (a.BorderBottomStyle() == BorderStyle::Hidden && b.BorderBottomStyle() == BorderStyle::None)
              || (a.BorderBottomStyle() == BorderStyle::None && b.BorderBottomStyle() == BorderStyle::Hidden)
              || (a.BorderLeftStyle() == BorderStyle::Hidden && b.BorderLeftStyle() == BorderStyle::None)
              || (a.BorderLeftStyle() == BorderStyle::None && b.BorderLeftStyle() == BorderStyle::Hidden)
              || (a.BorderRightStyle() == BorderStyle::Hidden && b.BorderRightStyle() == BorderStyle::None)
              || (a.BorderRightStyle() == BorderStyle::None && b.BorderRightStyle() == BorderStyle::Hidden)))
      {
        return true;
      }
    }

    if (static_cast<DisplayType>(a.GetNonInheritedFlags().EffectiveDisplay) == DisplayType::ListItem)
    {
      if (a.GetInheritedFlags().ListStylePosition != b.GetInheritedFlags().ListStylePosition
          || a.RareInheritedData().ListStyleType != b.RareInheritedData().ListStyleType)
      {
        return true;
      }
    }

    if (a.GetInheritedFlags().TextAlign != b.GetInheritedFlags().TextAlign
        || a.GetInheritedFlags().TextTransform != b.GetInheritedFlags().TextTransform
        || a.GetInheritedFlags().WhiteSpaceCollapse != b.GetInheritedFlags().WhiteSpaceCollapse
        || a.GetInheritedFlags().TextWrapMode != b.GetInheritedFlags().TextWrapMode
        || a.GetInheritedFlags().TextWrapStyle != b.GetInheritedFlags().TextWrapStyle
        || a.GetNonInheritedFlags().Clear != b.GetNonInheritedFlags().Clear
        || a.GetNonInheritedFlags().UnicodeBidi != b.GetNonInheritedFlags().UnicodeBidi)
    {
      return true;
    }

    if (a.GetWritingMode() != b.GetWritingMode())
    {
      return true;
    }

    // Overflow returns a layout hint.
    if (a.GetNonInheritedFlags().OverflowX != b.GetNonInheritedFlags().OverflowX
        || a.GetNonInheritedFlags().OverflowY != b.GetNonInheritedFlags().OverflowY)
    {
      return true;
    }

    if ((a.UsedVisibility() == Visibility::Collapse) != (b.UsedVisibility() == Visibility::Collapse))
    {
      return true;
    }

    bool aHasFirstLineStyle = a.HasPseudoStyle(PseudoElementType::FirstLine);
    if (aHasFirstLineStyle != b.HasPseudoStyle(PseudoElementType::FirstLine))
    {
      return true;
    }

    if (aHasFirstLineStyle)
    {
      auto *aFirstLineStyle = a.GetCachedPseudoStyle({PseudoElementType::FirstLine});
      if (!aFirstLineStyle)
      {
        return true;
      }
      auto *bFirstLineStyle = b.GetCachedPseudoStyle({PseudoElementType::FirstLine});
      if (!bFirstLineStyle)
      {
        return true;
      }
      // FIXME: Not all first line style changes actually need layout.
      if (*aFirstLineStyle != *bFirstLineStyle)
      {
        return true;
      }
    }

    return false;
  }

  // MARK: DifferenceResult::LayoutOutOfFlowMovementOnly

  static bool
    ChangeRequiresOutOfFlowMovementLayoutOnly(const RenderStyle &a, const RenderStyle &b,
                                              EnumSet<DifferenceContextSensitiveProperty> &) noexcept
  {
    if (a.GetPosition() != PositionType::Absolute)
    {
      return false;
    }

    // Optimize for the case where a out-of-flow box is moving but not changing size.
    return a.NonInheritedData().SurroundData->Inset != b.NonInheritedData().SurroundData->Inset
           && PositionChangeIsMovementOnly(a.NonInheritedData().SurroundData->Inset,
                                           b.NonInheritedData().SurroundData->Inset,
                                           a.NonInheritedData().BoxData->Width);
  }

  // MARK: DifferenceResult::RepaintLayer

  static bool MiscDataChangeRequiresLayerRepaint(
    const StyleMiscNonInheritedData &a, const StyleMiscNonInheritedData &b,
    EnumSet<DifferenceContextSensitiveProperty> &changedContextSensitiveProperties) noexcept
  {
    if (a.Opacity != b.Opacity)
    {
      changedContextSensitiveProperties.Add(DifferenceContextSensitiveProperty::Opacity);
      // Don't return true; keep looking for another change.
    }

    if (a.Filter != b.Filter)
    {
      changedContextSensitiveProperties.Add(DifferenceContextSensitiveProperty::Filter);
      // Don't return true; keep looking for another change.
    }

    // FIXME: In SVG this needs to trigger a layout.
    if (a.Mask != b.Mask)
    {
      return true;
    }

    return false;
  }

  static bool RareDataChangeRequiresLayerRepaint(
    const StyleRareNonInheritedData &a, const StyleRareNonInheritedData &b,
    EnumSet<DifferenceContextSensitiveProperty> &changedContextSensitiveProperties) noexcept
  {
    if (a.EffectiveBlendMode != b.EffectiveBlendMode)
    {
      return true;
    }

    if (a.BackdropFilter != b.BackdropFilter)
    {
      changedContextSensitiveProperties.Add(DifferenceContextSensitiveProperty::Filter);
      // Don't return true; keep looking for another change.
    }

    // FIXME: In SVG this needs to trigger a layout.
    if (a.MaskBorder != b.MaskBorder)
    {
      return true;
    }

    return false;
  }

  static bool ChangeRequiresLayerRepaint(
    const RenderStyle &a, const RenderStyle &b,
    EnumSet<DifferenceContextSensitiveProperty> &changedContextSensitiveProperties) noexcept
  {
    // Style::Resolver has ensured that zIndex is non-auto only if it's applicable.

    if (&a.NonInheritedData() != &b.NonInheritedData())
    {
      if (a.NonInheritedData().BoxData.Ptr() != b.NonInheritedData().BoxData.Ptr())
      {
        if (a.NonInheritedData().BoxData->UsedZIndex() != b.NonInheritedData().BoxData->UsedZIndex())
        {
          return true;
        }
      }

      if (a.GetPosition() != PositionType::Static)
      {
        if (a.NonInheritedData().RareData.Ptr() != b.NonInheritedData().RareData.Ptr())
        {
          if (a.NonInheritedData().RareData->Clip != b.NonInheritedData().RareData->Clip)
          {
            changedContextSensitiveProperties.Add(DifferenceContextSensitiveProperty::ClipRect);
            return true;
          }
        }
      }

      if (a.NonInheritedData().MiscData.Ptr() != b.NonInheritedData().MiscData.Ptr()
          && MiscDataChangeRequiresLayerRepaint(*a.NonInheritedData().MiscData,
                                                *b.NonInheritedData().MiscData,
                                                changedContextSensitiveProperties))
      {
        return true;
      }

      if (a.NonInheritedData().RareData.Ptr() != b.NonInheritedData().RareData.Ptr()
          && RareDataChangeRequiresLayerRepaint(*a.NonInheritedData().RareData,
                                                *b.NonInheritedData().RareData,
                                                changedContextSensitiveProperties))
      {
        return true;
      }
    }

    if (&a.RareInheritedData() != &b.RareInheritedData()
        && a.RareInheritedData().DynamicRangeLimit != b.RareInheritedData().DynamicRangeLimit)
    {
      return true;
    }

    bool currentColorDiffers = a.InheritedData().Color != b.InheritedData().Color;
    if (currentColorDiffers)
    {
      if (a.GetFilter().HasFilterThatRequiresRepaintForCurrentColorChange()
          || a.BackdropFilter().HasFilterThatRequiresRepaintForCurrentColorChange())
      {
        return true;
      }
    }

    return false;
  }

  // MARK: DifferenceResult::Repaint

  static bool RequiresPainting(const RenderStyle &style) noexcept
  {
    if (style.UsedVisibility() == Visibility::Hidden)
    {
      return false;
    }
    if (style.GetOpacity().IsTransparent())
    {
      return false;
    }
    return true;
  }

  static bool IsEquivalentForPainting(const StyleBackgroundData &a, const StyleBackgroundData &b,
                                      bool currentColorDiffers) noexcept
  {
    if (&a == &b)
    {
      krys_debug_assert(currentColorDiffers);
      return !a.ContainsCurrentColor();
    }

    if (a.Background != b.Background || a.BackgroundColor != b.BackgroundColor)
    {
      return false;
    }
    if (currentColorDiffers && a.BackgroundColor.ContainsCurrentColor())
    {
      return false;
    }
    if (!a.Outline.IsVisible() && !b.Outline.IsVisible())
    {
      return true;
    }
    if (currentColorDiffers && a.Outline.OutlineColor.ContainsCurrentColor())
    {
      return false;
    }
    return a.Outline == b.Outline;
  }

  static bool IsEquivalentForPainting(const BorderData &a, const BorderData &b,
                                      bool currentColorDiffers) noexcept
  {
    if (&a == &b)
    {
      krys_debug_assert(currentColorDiffers);
      return !a.ContainsCurrentColor();
    }

    if (a != b)
    {
      return false;
    }

    if (!currentColorDiffers)
    {
      return true;
    }

    return !a.ContainsCurrentColor();
  }

  static bool ColorChangeRequiresRepaint(const Color &a, const Color &b, bool currentColorDiffers) noexcept
  {
    if (a != b)
    {
      return true;
    }

    if (a.ContainsCurrentColor())
    {
      krys_debug_assert(b.ContainsCurrentColor());
      return currentColorDiffers;
    }

    return false;
  }

  static bool SvgDataChangeRequiresRepaint(const SVGRenderStyle &a, const SVGRenderStyle &b,
                                           bool currentColorDiffers) noexcept
  {
    if (&a == &b)
    {
      krys_debug_assert(currentColorDiffers);
      return ContainsCurrentColor(a.StrokeData->Stroke)
             || ContainsCurrentColor(a.StrokeData->VisitedLinkStroke)
             || a.MiscData->FloodColor.ContainsCurrentColor()
             || a.MiscData->LightingColor.ContainsCurrentColor()
             || ContainsCurrentColor(
               a.FillData->Fill); // FIXME: Should this be checking fillData->visitedLinkFill as well?
    }

    if (a.StrokeData->StrokeOpacity != b.StrokeData->StrokeOpacity
        || ColorChangeRequiresRepaint(a.StrokeData->Stroke.ColorDisregardingType(),
                                      b.StrokeData->Stroke.ColorDisregardingType(), currentColorDiffers)
        || ColorChangeRequiresRepaint(a.StrokeData->VisitedLinkStroke.ColorDisregardingType(),
                                      b.StrokeData->VisitedLinkStroke.ColorDisregardingType(),
                                      currentColorDiffers))
    {
      return true;
    }

    // Painting related properties only need repaints.
    if (ColorChangeRequiresRepaint(a.MiscData->FloodColor, b.MiscData->FloodColor, currentColorDiffers)
        || a.MiscData->FloodOpacity != b.MiscData->FloodOpacity
        || ColorChangeRequiresRepaint(a.MiscData->LightingColor, b.MiscData->LightingColor,
                                      currentColorDiffers))
    {
      return true;
    }

    // If fill data changes, we just need to repaint. Fill boundaries are not influenced by this, only by the
    // Path, that RenderSVGPath contains.
    if (!a.FillData->Fill.HasSameType(b.FillData->Fill)
        || ColorChangeRequiresRepaint(a.FillData->Fill.ColorDisregardingType(),
                                      b.FillData->Fill.ColorDisregardingType(), currentColorDiffers)
        || a.FillData->Fill.UrlDisregardingType() != b.FillData->Fill.UrlDisregardingType()
        || a.FillData->FillOpacity != b.FillData->FillOpacity)
    {
      return true;
    }

    // If gradient stops change, we just need to repaint. Style updates are already handled through
    // RenderSVGGradientStop.
    if (a.StopData != b.StopData)
    {
      return true;
    }

    // Changes of these flags only cause repaints.
    if (a.InheritedFlags.ShapeRendering != b.InheritedFlags.ShapeRendering
        || a.InheritedFlags.ClipRule != b.InheritedFlags.ClipRule
        || a.InheritedFlags.FillRule != b.InheritedFlags.FillRule
        || a.InheritedFlags.ColorInterpolation != b.InheritedFlags.ColorInterpolation
        || a.InheritedFlags.ColorInterpolationFilters != b.InheritedFlags.ColorInterpolationFilters)
    {
      return true;
    }

    if (a.NonInheritedFlags.BufferedRendering != b.NonInheritedFlags.BufferedRendering)
    {
      return true;
    }

    if (a.NonInheritedFlags.MaskType != b.NonInheritedFlags.MaskType)
    {
      return true;
    }

    return false;
  }

  static bool MiscDataChangeRequiresRepaint(const StyleMiscNonInheritedData &a,
                                            const StyleMiscNonInheritedData &b,
                                            EnumSet<DifferenceContextSensitiveProperty> &) noexcept
  {
    if (a.UserDrag != b.UserDrag || a.ObjectFit != b.ObjectFit || a.ObjectPosition != b.ObjectPosition)
    {
      return true;
    }

    return false;
  }

  static bool RareDataChangeRequiresRepaint(
    const StyleRareNonInheritedData &a, const StyleRareNonInheritedData &b,
    EnumSet<DifferenceContextSensitiveProperty> &changedContextSensitiveProperties) noexcept
  {
    if (a.ShapeOutside != b.ShapeOutside)
    {
      return true;
    }

    // FIXME: this should probably be moved to changeRequiresLayerRepaint().
    if (a.ClipPath != b.ClipPath)
    {
      changedContextSensitiveProperties.Add(DifferenceContextSensitiveProperty::ClipPath);
      // Don't return true; keep looking for another change.
    }

    if (a.TextDecorationStyle != b.TextDecorationStyle || a.TextDecorationColor != b.TextDecorationColor
        || a.TextDecorationThickness != b.TextDecorationThickness)
    {
      return true;
    }

    return false;
  }

  static bool RareInheritedDataChangeRequiresRepaint(const StyleRareInheritedData &a,
                                                     const StyleRareInheritedData &b) noexcept
  {
    return a.EffectiveInert != b.EffectiveInert || a.UserModify != b.UserModify
           || a.UserSelect != b.UserSelect || a.AppleColorFilter != b.AppleColorFilter
           || a.ImageRendering != b.ImageRendering || a.AccentColor != b.AccentColor
           || a.InsideDefaultButton != b.InsideDefaultButton || a.InsideSubmitButton != b.InsideSubmitButton
           || a.ColorScheme != b.ColorScheme;
  }

  static bool ChangedCustomPaintWatchedProperty(const RenderStyle &a, const StyleRareNonInheritedData &aData,
                                                const RenderStyle &b,
                                                const StyleRareNonInheritedData &bData) noexcept
  {
    auto &propertiesA = aData.CustomPaintWatchedProperties;
    auto &propertiesB = bData.CustomPaintWatchedProperties;

    if (!propertiesA.empty() || !propertiesB.empty()) KRYS_UNLIKELY
    {
      // FIXME: We should not need to use Style::Extractor here.
      Extractor extractor(static_cast<dom::Element *>(nullptr));
      auto &pool = css::ValuePool::MainThreadPool();

      for (auto *watchPropertiesMap : {&propertiesA, &propertiesB})
      {
        for (auto &name : *watchPropertiesMap)
        {
          if (css::IsCustomPropertyName(name.View()))
          {
            auto *valueA = a.CustomPropertyValue(name);
            auto *valueB = b.CustomPropertyValue(name);

            if (valueA != valueB && (!valueA || !valueB || *valueA != *valueB))
            {
              return true;
            }
          }
          else if (auto propertyId = css::PropertyIdFromName(name.View());
                   propertyId != css::PropertyId::Invalid)
          {
            auto valueA = extractor.PropertyValueInStyle(a, propertyId, pool);
            auto valueB = extractor.PropertyValueInStyle(b, propertyId, pool);

            if (valueA != valueB && (!valueA || !valueB || *valueA != *valueB))
            {
              return true;
            }
          }
        }
      }
    }

    return false;
  }

  static bool ChangeRequiresRepaint(
    const RenderStyle &a, const RenderStyle &b,
    EnumSet<DifferenceContextSensitiveProperty> &changedContextSensitiveProperties) noexcept
  {
    bool currentColorDiffers = a.InheritedData().Color != b.InheritedData().Color;

    if (currentColorDiffers || &a.SvgStyle() != &b.SvgStyle())
    {
      if (SvgDataChangeRequiresRepaint(a.SvgStyle(), b.SvgStyle(), currentColorDiffers))
      {
        return true;
      }
    }

    if (!RequiresPainting(a) && !RequiresPainting(b))
    {
      return false;
    }

    if (a.UsedVisibility() != b.UsedVisibility()
        || a.GetInheritedFlags().PrintColorAdjust != b.GetInheritedFlags().PrintColorAdjust
        || a.GetInheritedFlags().InsideLink != b.GetInheritedFlags().InsideLink)
    {
      return true;
    }

    if (currentColorDiffers || &a.NonInheritedData() != &b.NonInheritedData())
    {
      if (currentColorDiffers
          || a.NonInheritedData().BackgroundData.Ptr() != b.NonInheritedData().BackgroundData.Ptr())
      {
        if (!IsEquivalentForPainting(*a.NonInheritedData().BackgroundData,
                                     *b.NonInheritedData().BackgroundData, currentColorDiffers))
        {
          return true;
        }
      }

      if (currentColorDiffers
          || a.NonInheritedData().SurroundData.Ptr() != b.NonInheritedData().SurroundData.Ptr())
      {
        if (!IsEquivalentForPainting(a.NonInheritedData().SurroundData->Border,
                                     b.NonInheritedData().SurroundData->Border, currentColorDiffers))
        {
          return true;
        }
      }
    }

    if (&a.NonInheritedData() != &b.NonInheritedData())
    {
      if (a.NonInheritedData().MiscData.Ptr() != b.NonInheritedData().MiscData.Ptr()
          && MiscDataChangeRequiresRepaint(*a.NonInheritedData().MiscData, *b.NonInheritedData().MiscData,
                                           changedContextSensitiveProperties))
      {
        return true;
      }

      if (a.NonInheritedData().RareData.Ptr() != b.NonInheritedData().RareData.Ptr()
          && RareDataChangeRequiresRepaint(*a.NonInheritedData().RareData, *b.NonInheritedData().RareData,
                                           changedContextSensitiveProperties))
      {
        return true;
      }
    }

    if (&a.RareInheritedData() != &b.RareInheritedData()
        && RareInheritedDataChangeRequiresRepaint(a.RareInheritedData(), b.RareInheritedData()))
    {
      return true;
    }

    if (ChangedCustomPaintWatchedProperty(a, *a.NonInheritedData().RareData, b,
                                          *b.NonInheritedData().RareData))
    {
      return true;
    }

    return false;
  }

  // MARK: DifferenceResult::RepaintIfText

  static bool ChangeRequiresRepaintIfText(const RenderStyle &a, const RenderStyle &b,
                                          EnumSet<DifferenceContextSensitiveProperty> &) noexcept
  {
    // FIXME: Does this code need to consider currentColorDiffers? webkit.org/b/266833
    if (a.InheritedData().Color != b.InheritedData().Color)
    {
      return true;
    }

    // Note that we may reach this function with mutated text-decoration values (e.g. thickness), when visual
    // overflow recompute is not required. see `changeAffectsVisualOverflow`
    if (a.GetInheritedFlags().TextDecorationLineInEffect != b.GetInheritedFlags().TextDecorationLineInEffect
        || a.GetNonInheritedFlags().TextDecorationLine != b.GetNonInheritedFlags().TextDecorationLine)
    {
      return true;
    }

    if (&a.RareInheritedData() != &b.RareInheritedData())
    {
      if (a.RareInheritedData().TextDecorationSkipInk != b.RareInheritedData().TextDecorationSkipInk
          || a.RareInheritedData().TextFillColor != b.RareInheritedData().TextFillColor
          || a.RareInheritedData().TextEmphasisColor != b.RareInheritedData().TextEmphasisColor
          || a.RareInheritedData().TextEmphasisStyle != b.RareInheritedData().TextEmphasisStyle
          || a.RareInheritedData().StrokeColor != b.RareInheritedData().StrokeColor
          || a.RareInheritedData().CaretColor != b.RareInheritedData().CaretColor
          || a.RareInheritedData().TextUnderlineOffset != b.RareInheritedData().TextUnderlineOffset)
      {
        return true;
      }
    }

    return false;
  }

  // MARK: DifferenceResult::RecompositeLayer

  static bool ChangeRequiresRecompositeLayer(const RenderStyle &a, const RenderStyle &b,
                                             EnumSet<DifferenceContextSensitiveProperty> &) noexcept
  {
    if (a.GetInheritedFlags().PointerEvents != b.GetInheritedFlags().PointerEvents)
    {
      return true;
    }

    if (&a.NonInheritedData() != &b.NonInheritedData()
        && a.NonInheritedData().RareData.Ptr() != b.NonInheritedData().RareData.Ptr())
    {
      if (a.UsedTransformStyle3D() != b.UsedTransformStyle3D()
          || a.NonInheritedData().RareData->BackfaceVisibility
               != b.NonInheritedData().RareData->BackfaceVisibility
          || a.NonInheritedData().RareData->Perspective != b.NonInheritedData().RareData->Perspective
          || a.NonInheritedData().RareData->PerspectiveOrigin
               != b.NonInheritedData().RareData->PerspectiveOrigin
          || a.NonInheritedData().RareData->OverscrollBehaviorX
               != b.NonInheritedData().RareData->OverscrollBehaviorX
          || a.NonInheritedData().RareData->OverscrollBehaviorY
               != b.NonInheritedData().RareData->OverscrollBehaviorY)
      {
        return true;
      }
    }

    if (&a.RareInheritedData() != &b.RareInheritedData()
        && a.RareInheritedData().EffectiveInert != b.RareInheritedData().EffectiveInert)
    {
      return true;
    }

    return false;
  }

  // MARK: - Exported Functions

  Difference GetDifference(const RenderStyle &a, const RenderStyle &b) noexcept
  {
    auto changedContextSensitiveProperties = EnumSet<DifferenceContextSensitiveProperty>();

    if (ChangeRequiresLayout(a, b, changedContextSensitiveProperties))
    {
      return {DifferenceResult::Layout, changedContextSensitiveProperties};
    }

    if (ChangeRequiresOutOfFlowMovementLayoutOnly(a, b, changedContextSensitiveProperties))
    {
      return {DifferenceResult::LayoutOutOfFlowMovementOnly, changedContextSensitiveProperties};
    }

    if (ChangeRequiresLayerRepaint(a, b, changedContextSensitiveProperties))
    {
      return {DifferenceResult::RepaintLayer, changedContextSensitiveProperties};
    }

    if (ChangeRequiresRepaint(a, b, changedContextSensitiveProperties))
    {
      return {DifferenceResult::Repaint, changedContextSensitiveProperties};
    }

    if (ChangeRequiresRepaintIfText(a, b, changedContextSensitiveProperties))
    {
      return {DifferenceResult::RepaintIfText, changedContextSensitiveProperties};
    }

    // FIXME: RecompositeLayer should also behave as a priority bit (e.g when the style change requires
    // layout, we know that the content also needs repaint and it will eventually get repainted, but a repaint
    // type of change (e.g. color change) does not necessarily trigger recomposition).
    if (ChangeRequiresRecompositeLayer(a, b, changedContextSensitiveProperties))
    {
      return {DifferenceResult::RecompositeLayer, changedContextSensitiveProperties};
    }

    // Cursors are not checked, since they will be set appropriately in response to mouse events,
    // so they don't need to cause any repaint or layout.

    // Animations don't need to be checked either.  We always set the new style on the RenderObject, so we
    // will get a chance to fire off the resulting transition properly.
    return {DifferenceResult::Equal, changedContextSensitiveProperties};
  }

  bool DifferenceRequiresLayerRepaint(const RenderStyle &a, const RenderStyle &b, bool isComposited) noexcept
  {
    auto changedContextSensitiveProperties = EnumSet<DifferenceContextSensitiveProperty>();

    if (ChangeRequiresRepaint(a, b, changedContextSensitiveProperties))
    {
      return true;
    }

    if (isComposited && ChangeRequiresLayerRepaint(a, b, changedContextSensitiveProperties))
    {
      return changedContextSensitiveProperties.Contains(DifferenceContextSensitiveProperty::ClipRect);
    }

    return false;
  }

  bool BorderIsEquivalentForPainting(const RenderStyle &a, const RenderStyle &b) noexcept
  {
    bool colorDiffers = a.GetColor() != b.GetColor();

    if (!colorDiffers
        && (&a.NonInheritedData() == &b.NonInheritedData()
            || a.NonInheritedData().SurroundData.Ptr() == b.NonInheritedData().SurroundData.Ptr()
            || a.NonInheritedData().SurroundData->Border == b.NonInheritedData().SurroundData->Border))
    {
      return true;
    }

    return IsEquivalentForPainting(a.Border(), b.Border(), colorDiffers);
  }
}
