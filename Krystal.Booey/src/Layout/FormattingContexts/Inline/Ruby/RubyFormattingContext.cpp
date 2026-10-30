#include "Krystal.Booey/Layout/FormattingContexts/Inline/Ruby/RubyFormattingContext.hpp"
#include "Krystal.Booey/Graphics/FontMetrics.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentAligner.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingUtils.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineBox.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationUtils.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <ranges>

namespace krys::boo::layout
{
  namespace
  {
    KRYS_NODISCARD inline InlineLayoutUnit HalfOfAFullWidthCharacter(const Box &annotationBox) noexcept
    {
      return annotationBox.Style().ComputedFontSize() / 2.f;
    }

    KRYS_NODISCARD inline size_t BaseContentIndex(size_t rubyBaseStart,
                                                  const inline_display::Boxes &boxes) noexcept
    {
      auto baseContentIndex = rubyBaseStart + 1;
      if (boxes[baseContentIndex].LayoutBox().IsRubyAnnotationBox())
      {
        ++baseContentIndex;
      }
      return baseContentIndex;
    }

    KRYS_NODISCARD RubyPosition GetRubyPosition(const Box &rubyBaseLayoutBox) noexcept
    {
      krys_debug_assert(rubyBaseLayoutBox.IsRubyBase());
      auto computedRubyPosition = rubyBaseLayoutBox.Style().GetRubyPosition();
      if (!rubyBaseLayoutBox.GetWritingMode().IsVerticalTypographic())
      {
        return computedRubyPosition;
      }
      // inter-character: If the writing mode of the enclosing ruby container is vertical, this value has the
      // same effect as over.
      return rubyBaseLayoutBox.Style().IsInterCharacterRubyPosition() ? RubyPosition::Over
                                                                      : computedRubyPosition;
    }

    KRYS_NODISCARD inline InlineRect
      AnnotationMarginBoxVisualRect(const Box &annotationBox, InlineLayoutUnit lineHeight,
                                    InlineFormattingContext &inlineFormattingContext) noexcept
    {
      auto marginBoxLogicalRect = InlineRect {
        FloatRect {BoxGeometry::MarginBoxRect(inlineFormattingContext.GeometryForBox(annotationBox))}};
      auto writingMode = inlineFormattingContext.Root().GetWritingMode();
      if (writingMode.IsHorizontal())
      {
        return marginBoxLogicalRect;
      }
      auto visualTopLeft = marginBoxLogicalRect.TopLeft().TransposedPoint();
      if (writingMode.IsBlockFlipped())
      {
        return InlineRect {visualTopLeft, marginBoxLogicalRect.Size().TransposedSize()};
      }
      visualTopLeft.Move(lineHeight - marginBoxLogicalRect.Height(), {});
      return InlineRect {visualTopLeft, marginBoxLogicalRect.Size().TransposedSize()};
    }

    KRYS_NODISCARD InlineLayoutUnit BaseLogicalWidthFromRubyBaseEnd(
      const Box &rubyBaseLayoutBox, const Line::RunList &lineRuns,
      const InlineContentBreaker::ContinuousContent::RunList &candidateRuns) noexcept
    {
      krys_debug_assert(rubyBaseLayoutBox.IsRubyBase());
      // Canidate content is supposed to hold the base content and in case of soft wrap opportunities, line
      // may have some base content too.
      auto baseLogicalWidth = InlineLayoutUnit {0.f};
      auto hasSeenRubyBaseStart = false;
      for (auto &candidateRun : candidateRuns | std::views::reverse)
      {
        auto &inlineItem = candidateRun.Item;
        if (inlineItem.IsInlineBoxStart() && &inlineItem.LayoutBox() == &rubyBaseLayoutBox)
        {
          hasSeenRubyBaseStart = true;
          break;
        }
        baseLogicalWidth += candidateRun.ContentWidth();
      }
      if (hasSeenRubyBaseStart)
      {
        return baseLogicalWidth;
      }
      // Let's check the line for the rest of the base content.
      for (auto &lineRun : lineRuns | std::views::reverse)
      {
        if ((lineRun.IsInlineBoxStart() || lineRun.IsLineSpanningInlineBoxStart())
            && &lineRun.LayoutBox() == &rubyBaseLayoutBox)
        {
          break;
        }
        baseLogicalWidth += lineRun.LogicalWidth();
      }
      return baseLogicalWidth;
    }

    KRYS_NODISCARD bool AnnotationOverlapCheck(const inline_display::Box &adjacentDisplayBox,
                                               const InlineLayoutRect &overhangingRect,
                                               InlineLayoutUnit lineLogicalHeight,
                                               InlineFormattingContext &inlineFormattingContext) noexcept
    {
      // We are in the middle of a line, should not see any line breaks or ellipsis boxes here.
      krys_debug_assert(!adjacentDisplayBox.IsEllipsis() && !adjacentDisplayBox.IsRootInlineBox());
      // Skip empty content like <span></span>
      if (adjacentDisplayBox.VisualRectIgnoringBlockDirection().IsEmpty())
      {
        return false;
      }

      if (adjacentDisplayBox.InkOverflow().Intersects(overhangingRect))
      {
        return true;
      }
      auto &adjacentLayoutBox = adjacentDisplayBox.LayoutBox();
      // Adjacent ruby may have overlapping annotation.
      if (adjacentLayoutBox.IsRubyBase() && adjacentLayoutBox.AssociatedRubyAnnotationBox())
      {
        return AnnotationMarginBoxVisualRect(*adjacentLayoutBox.AssociatedRubyAnnotationBox(),
                                             lineLogicalHeight, inlineFormattingContext)
          .Intersects(overhangingRect);
      }
      return false;
    }
  }

  InlineLayoutUnit RubyFormattingContext::AnnotationBoxLogicalWidth(
    const Box &rubyBaseLayoutBox, InlineFormattingContext &inlineFormattingContext) noexcept
  {
    krys_debug_assert(rubyBaseLayoutBox.IsRubyBase());
    auto *annotationBox = rubyBaseLayoutBox.AssociatedRubyAnnotationBox();
    if (!annotationBox)
    {
      return {};
    }

    inlineFormattingContext.GetIntegrationUtils().LayoutWithFormattingContextForBox(*annotationBox);

    return inlineFormattingContext.GeometryForBox(*annotationBox).MarginBoxWidth().ToFloat();
  }

  InlineLayoutUnit RubyFormattingContext::BaseEndAdditionalLogicalWidth(
    const Box &rubyBaseLayoutBox, const Line::RunList &lineRuns,
    const InlineContentBreaker::ContinuousContent::RunList &candidateRuns,
    InlineFormattingContext &inlineFormattingContext) noexcept
  {
    krys_debug_assert(rubyBaseLayoutBox.IsRubyBase());
    if (HasInterlinearAnnotation(rubyBaseLayoutBox))
    {
      // Base is supposed be at least as wide as the annotation is.
      // Let's adjust the inline box end width to accomodate such overflowing interlinear annotations.
      auto rubyBaseContentWidth = BaseLogicalWidthFromRubyBaseEnd(rubyBaseLayoutBox, lineRuns, candidateRuns);
      krys_debug_assert(rubyBaseContentWidth >= 0);
      return std::max(0.f, AnnotationBoxLogicalWidth(rubyBaseLayoutBox, inlineFormattingContext)
                             - rubyBaseContentWidth);
    }
    // While inter-character annotations don't participate in inline layout, they take up space.
    return AnnotationBoxLogicalWidth(rubyBaseLayoutBox, inlineFormattingContext);
  }

  size_t RubyFormattingContext::ApplyRubyAlignOnBaseContent(
    size_t rubyBaseStart, Line &line, HashMap<const Box *, InlineLayoutUnit> &alignmentOffsetList,
    InlineFormattingContext &inlineFormattingContext) noexcept
  {
    auto &runs = line.Runs();
    if (runs.empty())
    {
      krys_debug_assert(false);
      return rubyBaseStart;
    }
    auto &rubyBaseLayoutBox = runs[rubyBaseStart].LayoutBox();
    auto rubyBaseEnd = [&]() -> Maybe<size_t>
    {
      auto &rubyBox = rubyBaseLayoutBox.Parent();
      for (auto index = rubyBaseStart + 1; index < runs.size(); ++index)
      {
        if (&runs[index].LayoutBox().Parent() == &rubyBox)
        {
          return index;
        }
      }
      // We somehow managed to break content inside the base.
      return {};
    }();
    if (rubyBaseEnd && *rubyBaseEnd - rubyBaseStart == 1)
    {
      // Blank base needs no alignment.
      return *rubyBaseEnd;
    }
    auto *annotationBox = rubyBaseLayoutBox.AssociatedRubyAnnotationBox();
    if (!annotationBox)
    {
      return rubyBaseStart + 1;
    }

    inlineFormattingContext.GetIntegrationUtils().LayoutWithFormattingContextForBox(*annotationBox);

    auto annotationBoxLogicalWidth =
      InlineLayoutUnit {inlineFormattingContext.GeometryForBox(*annotationBox).MarginBoxWidth().ToFloat()};
    auto baseContentLogicalWidth =
      (rubyBaseEnd ? runs[*rubyBaseEnd].LogicalLeft() : runs.back().LogicalRight())
      - runs[rubyBaseStart].LogicalRight();
    if (annotationBoxLogicalWidth <= baseContentLogicalWidth)
    {
      return rubyBaseStart + 1;
    }

    auto spaceToDistribute = annotationBoxLogicalWidth - baseContentLogicalWidth;
    auto alignmentOffset = InlineContentAligner::ApplyRubyAlign(
      rubyBaseLayoutBox.Style().GetRubyAlign(), line.Runs(),
      {rubyBaseStart, rubyBaseEnd ? *rubyBaseEnd + 1 : runs.size()}, spaceToDistribute);
    if (rubyBaseEnd)
    {
      // Reset the spacing we added at LineBuilder.
      auto &rubyBaseEndRun = runs[*rubyBaseEnd];
      rubyBaseEndRun.ShrinkHorizontally(spaceToDistribute);
      rubyBaseEndRun.MoveHorizontally(2 * alignmentOffset);
    }

    krys_debug_assert(!alignmentOffsetList.contains(&rubyBaseLayoutBox));
    alignmentOffsetList.emplace(&rubyBaseLayoutBox, alignmentOffset);
    return rubyBaseEnd.value_or(runs.size());
  }

  HashMap<const Box *, InlineLayoutUnit>
    RubyFormattingContext::ApplyRubyAlign(Line &line,
                                          InlineFormattingContext &inlineFormattingContext) noexcept
  {
    HashMap<const Box *, InlineLayoutUnit> alignmentOffsetList;
    // https://drafts.csswg.org/css-ruby/#interlinear-inline
    // Within each base and annotation box, how the extra space is distributed when its content is narrower
    // than the measure of the box is specified by its ruby-align property.
    auto &runs = line.Runs();
    for (size_t index = 0; index < runs.size(); ++index)
    {
      auto &lineRun = runs[index];
      if (lineRun.IsInlineBoxStart() && lineRun.LayoutBox().IsRubyBase())
      {
        index = ApplyRubyAlignOnBaseContent(index, line, alignmentOffsetList, inlineFormattingContext);
      }
    }
    return alignmentOffsetList;
  }

  InlineLayoutUnit RubyFormattingContext::ApplyRubyAlignOnAnnotationBox(
    Line &line, InlineLayoutUnit spaceToDistribute, InlineFormattingContext &inlineFormattingContext) noexcept
  {
    return InlineContentAligner::ApplyRubyAlign(inlineFormattingContext.Root().Style().GetRubyAlign(),
                                                line.Runs(), {0, line.Runs().size()}, spaceToDistribute);
  }

  void RubyFormattingContext::ApplyAlignmentOffsetList(
    inline_display::Boxes &displayBoxes, const HashMap<const Box *, InlineLayoutUnit> &alignmentOffsetList,
    RubyBasesMayNeedResizing rubyBasesMayNeedResizing,
    InlineFormattingContext &inlineFormattingContext) noexcept
  {
    if (alignmentOffsetList.empty())
    {
      return;
    }
    InlineContentAligner::ApplyRubyBaseAlignmentOffset(
      displayBoxes, alignmentOffsetList,
      rubyBasesMayNeedResizing == RubyBasesMayNeedResizing::No
        ? InlineContentAligner::AdjustContentOnlyInsideRubyBase::Yes
        : InlineContentAligner::AdjustContentOnlyInsideRubyBase::No,
      inlineFormattingContext);
  }

  void RubyFormattingContext::ApplyAnnotationAlignmentOffset(
    inline_display::Boxes &displayBoxes, InlineLayoutUnit alignmentOffset,
    InlineFormattingContext &inlineFormattingContext) noexcept
  {
    if (!alignmentOffset)
    {
      return;
    }
    InlineContentAligner::ApplyRubyAnnotationAlignmentOffset(displayBoxes, alignmentOffset,
                                                             inlineFormattingContext);
  }

  InlineLayoutUnit RubyFormattingContext::BaseEndAdditionalLogicalWidth(
    const Box &rubyBaseLayoutBox, const inline_display::Box &, InlineLayoutUnit baseContentWidth,
    InlineFormattingContext &inlineFormattingContext) noexcept
  {
    if (!HasInterCharacterAnnotation(rubyBaseLayoutBox))
    {
      // FIXME: We may want to include interlinear annotations here too so that applyAlignmentOffsetList would
      // not need to initiate resizing (only moving base content).
      if (baseContentWidth)
      {
        return {};
      }
      auto *annotationBox = rubyBaseLayoutBox.AssociatedRubyAnnotationBox();
      if (!annotationBox)
      {
        return {};
      }
      auto &annotationBoxLogicalGeometry = inlineFormattingContext.GeometryForBox(*annotationBox);
      return annotationBoxLogicalGeometry.MarginBoxWidth().ToFloat();
    }
    // Note that intercharacter annotation stays vertical even when the ruby itself is vertical (which makes
    // it look like interlinear).
    return AnnotationBoxLogicalWidth(rubyBaseLayoutBox, inlineFormattingContext);
  }

  InlineLayoutPoint
    RubyFormattingContext::PlaceAnnotationBox(const Box &rubyBaseLayoutBox,
                                              const Rect &rubyBaseMarginBoxLogicalRect,
                                              InlineFormattingContext &inlineFormattingContext) noexcept
  {
    krys_debug_assert(rubyBaseLayoutBox.IsRubyBase());
    auto *annotationBox = rubyBaseLayoutBox.AssociatedRubyAnnotationBox();
    if (!annotationBox)
    {
      krys_debug_assert(false);
      return {};
    }
    auto &annotationBoxLogicalGeometry = inlineFormattingContext.GeometryForBox(*annotationBox);

    if (HasInterlinearAnnotation(rubyBaseLayoutBox))
    {
      // Move it over/under the base and make it border box positioned.
      auto leftOffset = annotationBoxLogicalGeometry.MarginStart();
      auto topOffset = GetRubyPosition(rubyBaseLayoutBox) == RubyPosition::Over
                         ? -annotationBoxLogicalGeometry.MarginBoxHeight()
                         : rubyBaseMarginBoxLogicalRect.Height();
      topOffset += annotationBoxLogicalGeometry.MarginBefore();

      auto logicalTopLeft = rubyBaseMarginBoxLogicalRect.TopLeft();
      logicalTopLeft.Move(LayoutSize {leftOffset, topOffset});
      return logicalTopLeft;
    }
    // Inter-character annotation box is stretched to the size of the base content box and vertically
    // centered.
    auto annotationContentBoxLogicalHeight = annotationBoxLogicalGeometry.ContentBoxHeight();
    auto annotationBorderTop = annotationBoxLogicalGeometry.BorderBefore();
    auto borderBoxRight = rubyBaseMarginBoxLogicalRect.Right() - annotationBoxLogicalGeometry.MarginBoxWidth()
                          + annotationBoxLogicalGeometry.MarginStart();
    return {borderBoxRight.ToFloat(),
            (rubyBaseMarginBoxLogicalRect.Top()
             + ((rubyBaseMarginBoxLogicalRect.Height() - annotationContentBoxLogicalHeight) / 2)
             - annotationBorderTop)
              .ToFloat()};
  }

  InlineLayoutSize
    RubyFormattingContext::SizeAnnotationBox(const Box &rubyBaseLayoutBox,
                                             const Rect &rubyBaseMarginBoxLogicalRect,
                                             InlineFormattingContext &inlineFormattingContext) noexcept
  {
    // FIXME: This is where we should take advantage of the ruby-column setup.
    krys_debug_assert(rubyBaseLayoutBox.IsRubyBase());
    auto *annotationBox = rubyBaseLayoutBox.AssociatedRubyAnnotationBox();
    if (!annotationBox)
    {
      krys_debug_assert(false);
      return {};
    }
    auto &annotationBoxLogicalGeometry = inlineFormattingContext.GeometryForBox(*annotationBox);
    if (HasInterlinearAnnotation(rubyBaseLayoutBox))
    {
      // Layout the annotation box again if we decided to change its size.
      auto newWidth =
        std::max(rubyBaseMarginBoxLogicalRect.Width(), annotationBoxLogicalGeometry.MarginBoxWidth());
      if (newWidth != annotationBoxLogicalGeometry.MarginBoxWidth())
      {
        inlineFormattingContext.GetIntegrationUtils().LayoutWithFormattingContextForBox(*annotationBox,
                                                                                        newWidth);
      }

      return {(newWidth - annotationBoxLogicalGeometry.HorizontalMarginBorderAndPadding()).ToFloat(),
              annotationBoxLogicalGeometry.ContentBoxHeight().ToFloat()};
    }

    return annotationBoxLogicalGeometry.ContentBoxSize();
  }

  void RubyFormattingContext::AdjustLayoutBoundsAndStretchAncestorRubyBase(
    LineBox &lineBox, InlineLevelBox &rubyBaseInlineBox, MaximumLayoutBoundsStretchMap &descendantRubySet,
    const InlineFormattingContext &inlineFormattingContext) noexcept
  {
    auto &rubyBaseLayoutBox = rubyBaseInlineBox.LayoutBox();
    krys_debug_assert(rubyBaseLayoutBox.IsRubyBase());

    auto stretchAncestorRubyBaseIfApplicable = [&](InlineLevelBox::AscentAndDescent layoutBounds)
    {
      auto &rootBox = inlineFormattingContext.Root();
      for (const Box *ancestor = &rubyBaseLayoutBox.Parent(); ancestor != &rootBox;
           ancestor = &ancestor->Parent())
      {
        if (ancestor->IsRubyBase())
        {
          auto *ancestorInlineBox = lineBox.InlineLevelBoxFor(*ancestor);
          if (!ancestorInlineBox)
          {
            krys_debug_assert(false);
            break;
          }
          auto previousDescendantLayoutBounds = InlineLevelBox::AscentAndDescent {};
          if (auto entry = descendantRubySet.find(ancestorInlineBox); entry != descendantRubySet.end())
          {
            previousDescendantLayoutBounds = entry->second;
          }
          descendantRubySet.insert_or_assign(
            ancestorInlineBox, InlineLevelBox::AscentAndDescent {
                                 std::max(previousDescendantLayoutBounds.Ascent, layoutBounds.Ascent),
                                 std::max(previousDescendantLayoutBounds.Descent, layoutBounds.Descent)});
          break;
        }
      }
    };

    auto layoutBounds = rubyBaseInlineBox.LayoutBounds();
    auto *annotationBox = rubyBaseLayoutBox.AssociatedRubyAnnotationBox();
    if (!annotationBox || !HasInterlinearAnnotation(rubyBaseLayoutBox))
    {
      // Make sure descendant rubies with annotations are propagated.
      stretchAncestorRubyBaseIfApplicable(layoutBounds);
      return;
    }

    auto over = InlineLayoutUnit {};
    auto under = InlineLayoutUnit {};
    auto annotationBoxLogicalHeight =
      InlineLayoutUnit {inlineFormattingContext.GeometryForBox(*annotationBox).MarginBoxHeight().ToFloat()};
    auto isAnnotationBefore = GetRubyPosition(rubyBaseLayoutBox) == RubyPosition::Over;
    if (isAnnotationBefore)
    {
      over = annotationBoxLogicalHeight;
    }
    else
    {
      under = annotationBoxLogicalHeight;
    }

    // FIXME: The spec says annotation should not stretch the line unless line-height is not normal and
    // annotation does not fit (i.e. line is sized too small for the annotation) Legacy ruby behaves slightly
    // differently by stretching the line box as needed.
    auto isFirstFormattedLine = !lineBox.LineIndex();
    auto descendantLayoutBounds = InlineLevelBox::AscentAndDescent {};
    if (auto entry = descendantRubySet.find(&rubyBaseInlineBox); entry != descendantRubySet.end())
    {
      descendantLayoutBounds = entry->second;
    }
    auto ascent = std::max(rubyBaseInlineBox.Ascent(), descendantLayoutBounds.Ascent);
    auto descent = std::max(rubyBaseInlineBox.Descent(), descendantLayoutBounds.Descent);

    if (rubyBaseInlineBox.IsPreferredLineHeightFontMetricsBased())
    {
      auto extraSpaceForAnnotation = InlineLayoutUnit {};
      if (!isFirstFormattedLine)
      {
        // Note that annotation may leak into the half leading space (gap between lines).
        auto lineGap = InlineFormattingUtils::SnapToInt(
          rubyBaseLayoutBox.Style().MetricsOfPrimaryFont().LineSpacing(), rubyBaseLayoutBox);
        extraSpaceForAnnotation = std::max(0.f, (lineGap - (ascent + descent)) / 2);
      }
      auto ascentWithAnnotation = (ascent + over) - extraSpaceForAnnotation;
      auto descentWithAnnotation = (descent + under) - extraSpaceForAnnotation;

      layoutBounds.Ascent = std::max(ascentWithAnnotation, layoutBounds.Ascent);
      layoutBounds.Descent = std::max(descentWithAnnotation, layoutBounds.Descent);
    }
    else
    {
      auto ascentWithAnnotation = ascent + over;
      auto descentWithAnnotation = descent + under;
      // FIXME: Normally we would check if there's space for both the ascent and the descent parts of the
      // content but in order to keep ruby tight we let subsequent lines (potentially) overlap each other by
      // only checking against total height (this affects the annotation box vertical placement by letting it
      // overlap the previous line's descent) However we have to make sure there's enough space for the
      // annotation box on the first line. This tight content arrangement is a legacy ruby behavior (see
      // placeChildInlineBoxesInBlockDirection) and we may wanna reconsider it at some point.
      if (isFirstFormattedLine)
      {
        layoutBounds.Ascent = std::max(ascentWithAnnotation, layoutBounds.Ascent);
        layoutBounds.Descent = std::max(descentWithAnnotation, layoutBounds.Descent);
      }
      else if (layoutBounds.Height() < ascentWithAnnotation + descentWithAnnotation)
      {
        // In case line-height does not produce enough space for annotation.
        auto extraSpaceNeededForAnnotation =
          (ascentWithAnnotation + descentWithAnnotation) - layoutBounds.Height();
        // Note that this makes annotation leak into previous/next line's (bottom/top) half leading. It
        // ensures though that we don't overly stretch lines and break (logical) vertical rhythm too much.
        if (isAnnotationBefore)
        {
          layoutBounds.Ascent += extraSpaceNeededForAnnotation;
        }
        else
        {
          layoutBounds.Descent += extraSpaceNeededForAnnotation;
        }
      }
    }

    rubyBaseInlineBox.SetLayoutBounds(layoutBounds);
    stretchAncestorRubyBaseIfApplicable(layoutBounds);
  }

  void RubyFormattingContext::ApplyAnnotationContributionToLayoutBounds(
    LineBox &lineBox, const InlineFormattingContext &inlineFormattingContext) noexcept
  {
    // In order to ensure consistent spacing of lines, documents with ruby typically ensure that the
    // line-height is large enough to accommodate ruby between lines of text. Therefore, ordinarily, ruby
    // annotation containers and ruby annotation boxes do not contribute to the measured height of a line's
    // inline contents; line-height calculations are performed using only the ruby base container, exactly as
    // if it were a normal inline. However, if the line-height specified on the ruby container is less than
    // the distance between the top of the top ruby annotation container and the bottom of the bottom ruby
    // annotation container, then additional leading is added on the appropriate side(s).
    MaximumLayoutBoundsStretchMap descentRubySet;
    for (auto &inlineLevelBox : lineBox.NonRootInlineLevelBoxes() | std::views::reverse)
    {
      if (!inlineLevelBox.IsInlineBox() || !inlineLevelBox.LayoutBox().IsRubyBase())
      {
        continue;
      }
      AdjustLayoutBoundsAndStretchAncestorRubyBase(lineBox, inlineLevelBox, descentRubySet,
                                                   inlineFormattingContext);
    }
  }

  InlineLayoutUnit RubyFormattingContext::OverhangForAnnotationBefore(
    const Box &rubyBaseLayoutBox, size_t rubyBaseStart, const inline_display::Boxes &boxes,
    InlineLayoutUnit lineLogicalHeight, InlineFormattingContext &inlineFormattingContext) noexcept
  {
    // [root inline box][ruby container][ruby base][ruby annotation]
    krys_debug_assert(rubyBaseStart >= 2);
    auto *annotationBox = rubyBaseLayoutBox.AssociatedRubyAnnotationBox();
    if (!annotationBox || !HasInterlinearAnnotation(rubyBaseLayoutBox) || rubyBaseStart <= 2)
    {
      return {};
    }
    if (rubyBaseStart + 1 >= boxes.size())
    {
      // We have to have some base content.
      krys_debug_assert(false);
      return {};
    }
    auto isHorizontalWritingMode = inlineFormattingContext.Root().GetWritingMode().IsHorizontal();
    auto baseContentStart = BaseContentIndex(rubyBaseStart, boxes);
    if (baseContentStart >= boxes.size())
    {
      krys_debug_assert(false);
      return {};
    }
    // FIXME: Usually the first content box is visually the leftmost, but we should really look for content
    // shifted to the left through negative margins on inline boxes.
    auto gapBetweenBaseAndContent = [&]
    {
      auto contentVisualRect = boxes[baseContentStart].VisualRectIgnoringBlockDirection();
      auto baseVisualRect = boxes[rubyBaseStart].VisualRectIgnoringBlockDirection();
      if (isHorizontalWritingMode)
      {
        return std::max(0.f, contentVisualRect.X() - baseVisualRect.X());
      }
      return std::max(0.f, contentVisualRect.Y() - baseVisualRect.Y());
    };
    auto overhangValue = std::min(HalfOfAFullWidthCharacter(*annotationBox), gapBetweenBaseAndContent());
    auto wouldAnnotationOrBaseOverlapAdjacentContent = [&]
    {
      // Check of adjacent (previous) content for overlapping.
      auto overhangingAnnotationVisualRect =
        AnnotationMarginBoxVisualRect(*annotationBox, lineLogicalHeight, inlineFormattingContext);
      auto baseContentBoxRect = boxes[baseContentStart].InkOverflow();
      // This is how much the annotation box/base content would be closer to content outside of base.
      auto offset = isHorizontalWritingMode ? InlineLayoutPoint(-overhangValue, 0.f)
                                            : InlineLayoutPoint(0.f, -overhangValue);
      overhangingAnnotationVisualRect.MoveBy(offset);
      baseContentBoxRect.MoveBy(offset);

      for (size_t index = 1; index < rubyBaseStart - 1; ++index)
      {
        auto &previousDisplayBox = boxes[index];
        if (AnnotationOverlapCheck(previousDisplayBox, overhangingAnnotationVisualRect, lineLogicalHeight,
                                   inlineFormattingContext))
        {
          return true;
        }
        if (AnnotationOverlapCheck(previousDisplayBox, baseContentBoxRect, lineLogicalHeight,
                                   inlineFormattingContext))
        {
          return true;
        }
      }
      return false;
    };
    return wouldAnnotationOrBaseOverlapAdjacentContent() ? 0.f : overhangValue;
  }

  InlineLayoutUnit RubyFormattingContext::OverhangForAnnotationAfter(
    const Box &rubyBaseLayoutBox, IndexRange rubyBaseRange, const inline_display::Boxes &boxes,
    InlineLayoutUnit lineLogicalHeight, InlineFormattingContext &inlineFormattingContext) noexcept
  {
    auto *annotationBox = rubyBaseLayoutBox.AssociatedRubyAnnotationBox();
    if (!annotationBox || !HasInterlinearAnnotation(rubyBaseLayoutBox))
    {
      return {};
    }

    if (!rubyBaseRange || rubyBaseRange.Distance() == 1 || rubyBaseRange.End() == boxes.size())
    {
      return {};
    }

    auto isHorizontalWritingMode = inlineFormattingContext.Root().GetWritingMode().IsHorizontal();
    // FIXME: Usually the last content box is visually the rightmost, but negative margin may override it.
    // FIXME: Currently justified content always expands producing 0 value for gapBetweenBaseEndAndContent.
    auto rubyBaseContentEnd = rubyBaseRange.End() - 1;
    auto gapBetweenBaseEndAndContent = [&]
    {
      auto baseStartVisualRect = boxes[rubyBaseRange.Begin()].VisualRectIgnoringBlockDirection();
      auto baseContentEndVisualRect = boxes[rubyBaseContentEnd].VisualRectIgnoringBlockDirection();
      if (isHorizontalWritingMode)
      {
        return std::max(0.f, baseStartVisualRect.MaxX() - baseContentEndVisualRect.MaxX());
      }
      return std::max(0.f, baseStartVisualRect.MaxY() - baseContentEndVisualRect.MaxY());
    };
    auto overhangValue = std::min(HalfOfAFullWidthCharacter(*annotationBox), gapBetweenBaseEndAndContent());
    auto wouldAnnotationOrBaseOverlapLineContent = [&]
    {
      // Check of adjacent (next) content for overlapping.
      auto overhangingAnnotationVisualRect =
        AnnotationMarginBoxVisualRect(*annotationBox, lineLogicalHeight, inlineFormattingContext);
      auto baseContentBoxRect = boxes[rubyBaseContentEnd].InkOverflow();
      // This is how much the base content would be closer to content outside of base.
      auto offset = isHorizontalWritingMode ? InlineLayoutPoint(overhangValue, 0.f)
                                            : InlineLayoutPoint(0.f, overhangValue);
      overhangingAnnotationVisualRect.MoveBy(offset);
      baseContentBoxRect.MoveBy(offset);

      for (size_t index = boxes.size() - 1; index >= rubyBaseRange.End(); --index)
      {
        auto &previousDisplayBox = boxes[index];
        if (AnnotationOverlapCheck(previousDisplayBox, overhangingAnnotationVisualRect, lineLogicalHeight,
                                   inlineFormattingContext))
        {
          return true;
        }
        if (AnnotationOverlapCheck(previousDisplayBox, baseContentBoxRect, lineLogicalHeight,
                                   inlineFormattingContext))
        {
          return true;
        }
      }
      return false;
    };
    return wouldAnnotationOrBaseOverlapLineContent() ? 0.f : overhangValue;
  }

  bool RubyFormattingContext::HasInterlinearAnnotation(const Box &rubyBaseLayoutBox) noexcept
  {
    krys_debug_assert(rubyBaseLayoutBox.IsRubyBase());
    return rubyBaseLayoutBox.AssociatedRubyAnnotationBox() && !HasInterCharacterAnnotation(rubyBaseLayoutBox);
  }

  bool RubyFormattingContext::HasInterCharacterAnnotation(const Box &rubyBaseLayoutBox) noexcept
  {
    krys_debug_assert(rubyBaseLayoutBox.IsRubyBase());
    if (!rubyBaseLayoutBox.GetWritingMode().IsHorizontal())
    {
      // If the writing mode of the enclosing ruby container is vertical, this value has the same effect as
      // over.
      return false;
    }

    if (auto *annotationBox = rubyBaseLayoutBox.AssociatedRubyAnnotationBox())
    {
      return annotationBox->Style().IsInterCharacterRubyPosition();
    }
    return false;
  }

  void RubyFormattingContext::ApplyRubyOverhang(
    InlineFormattingContext &parentFormattingContext, InlineLayoutUnit lineLogicalHeight,
    inline_display::Boxes &displayBoxes, const List<IndexRange> &interlinearRubyColumnRangeList) noexcept
  {
    // FIXME: We are only supposed to apply overhang when annotation box is wider than base, but at this point
    // we can't tell (this needs to be addressed together with annotation box sizing).
    if (interlinearRubyColumnRangeList.empty())
    {
      return;
    }

    auto isHorizontalWritingMode = parentFormattingContext.Root().GetWritingMode().IsHorizontal();
    for (auto startEndPair : interlinearRubyColumnRangeList)
    {
      krys_debug_assert(static_cast<bool>(startEndPair));
      if (startEndPair.Distance() == 1)
      {
        continue;
      }

      auto rubyBaseStart = startEndPair.Begin();
      auto &rubyBaseLayoutBox = displayBoxes[rubyBaseStart].LayoutBox();
      krys_debug_assert(rubyBaseLayoutBox.IsRubyBase());
      krys_debug_assert(HasInterlinearAnnotation(rubyBaseLayoutBox));
      if (rubyBaseLayoutBox.Style().GetRubyOverhang() == RubyOverhang::None)
      {
        continue;
      }

      auto beforeOverhang = OverhangForAnnotationBefore(rubyBaseLayoutBox, rubyBaseStart, displayBoxes,
                                                        lineLogicalHeight, parentFormattingContext);
      auto afterOverhang =
        OverhangForAnnotationAfter(rubyBaseLayoutBox, {rubyBaseStart, startEndPair.End()}, displayBoxes,
                                   lineLogicalHeight, parentFormattingContext);

      // FIXME: If this turns out to be a pref bottleneck, make sure we pass in the accumulated shift to
      // overhangForAnnotationBefore/after and offset all box geometry as we check for overlap.
      auto moveBoxRangeToVisualLeft = [&](size_t start, size_t end, InlineLayoutUnit shiftValue)
      {
        for (auto index = start; index <= end; ++index)
        {
          auto &displayBox = displayBoxes[index];
          if (isHorizontalWritingMode)
          {
            displayBox.MoveHorizontally(-shiftValue);
          }
          else
          {
            displayBox.MoveVertically(-shiftValue);
          }

          auto &layoutBox = displayBox.LayoutBox();
          if (displayBox.IsInlineLevelBox() && !displayBox.IsRootInlineBox())
          {
            parentFormattingContext.GeometryForBox(layoutBox).MoveHorizontally(LayoutUnit {-shiftValue});
          }

          if (layoutBox.IsRubyBase() && layoutBox.AssociatedRubyAnnotationBox())
          {
            parentFormattingContext.GeometryForBox(*layoutBox.AssociatedRubyAnnotationBox())
              .MoveHorizontally(LayoutUnit {-shiftValue});
          }
        }
      };
      auto hasJustifiedAdjacentAfterContent = [&]
      {
        if (startEndPair.End() == displayBoxes.size())
        {
          return false;
        }
        auto &afterRubyBaseDisplayBox = displayBoxes[startEndPair.End()];
        if (afterRubyBaseDisplayBox.LayoutBox().IsRubyBase())
        {
          // Adjacent content is also a ruby base.
          return false;
        }
        return afterRubyBaseDisplayBox.GetExpansion().HorizontalExpansion != 0.f;
      }();

      if (beforeOverhang)
      {
        // When "before" adjacent content slightly pulls the rest of the content on the line leftward, justify
        // content should stay intact.
        moveBoxRangeToVisualLeft(
          rubyBaseStart, hasJustifiedAdjacentAfterContent ? startEndPair.End() : displayBoxes.size() - 1,
          beforeOverhang);
      }
      if (afterOverhang)
      {
        // Normally we shift all the "after" boxes to the left here as one monolithic content but in case of
        // justified alignment we can only move the adjacent run under the annotation and expand the justified
        // space to keep the rest of the runs stationary.
        if (hasJustifiedAdjacentAfterContent)
        {
          auto &afterRubyBaseDisplayBox = displayBoxes[startEndPair.End()];
          auto expansion = afterRubyBaseDisplayBox.GetExpansion();
          auto inflateValue = afterOverhang + beforeOverhang;
          afterRubyBaseDisplayBox.SetExpansion(
            {expansion.Behavior, expansion.HorizontalExpansion + inflateValue});
          if (isHorizontalWritingMode)
          {
            afterRubyBaseDisplayBox.ExpandHorizontally(inflateValue);
          }
          else
          {
            afterRubyBaseDisplayBox.ExpandVertically(inflateValue);
          }
          moveBoxRangeToVisualLeft(startEndPair.End(), startEndPair.End(), afterOverhang);
        }
        else
        {
          moveBoxRangeToVisualLeft(startEndPair.End(), displayBoxes.size() - 1, afterOverhang);
        }
      }
    }
  }
}
