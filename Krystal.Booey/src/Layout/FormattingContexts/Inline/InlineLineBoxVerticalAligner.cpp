#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineBoxVerticalAligner.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::layout
{
  LineBoxVerticalAligner::LineBoxVerticalAligner(
    const InlineFormattingContext &inlineFormattingContext) noexcept
      : _inlineFormattingContext(inlineFormattingContext)
  {
  }

  const InlineFormattingUtils &LineBoxVerticalAligner::FormattingUtils() const noexcept
  {
    return GetFormattingContext().FormattingUtils();
  }

  const ElementBox &LineBoxVerticalAligner::RootBox() const noexcept
  {
    return GetFormattingContext().Root();
  }

  const InlineLayoutState &LineBoxVerticalAligner::GetLayoutState() const noexcept
  {
    return GetFormattingContext().GetLayoutState();
  }

  InlineLayoutUnit LineBoxVerticalAligner::ComputeLogicalHeightAndAlign(LineBox &lineBox) const noexcept
  {
    auto canUseSimplifiedAlignment = [&]
    {
      if (!lineBox.HasContent())
      {
        return true;
      }

      if (RootBox().Style().LineBoxContain() != style::ComputedStyle::InitialLineBoxContain())
      {
        return false;
      }
      auto &rootInlineBox = lineBox.RootInlineBox();
      if (!GetLayoutState().InStandardsMode()
          || !HoldsAlternative<css::keywords::Baseline>(rootInlineBox.VerticalAlign()))
      {
        return false;
      }
      if (rootInlineBox.HasTextEmphasis())
      {
        return false;
      }

      for (auto &inlineLevelBox : lineBox.NonRootInlineLevelBoxes())
      {
        auto shouldUseSimplifiedAlignmentForInlineLevelBox = [&]
        {
          if (inlineLevelBox.HasTextEmphasis())
          {
            return false;
          }
          // Baseline aligned, non-stretchy direct children are considered to be simple for now.
          auto &layoutBox = inlineLevelBox.LayoutBox();
          if (&layoutBox.Parent() != &rootInlineBox.LayoutBox()
              || !HoldsAlternative<css::keywords::Baseline>(inlineLevelBox.VerticalAlign()))
          {
            return false;
          }

          if (inlineLevelBox.IsAtomicInlineBox())
          {
            auto &inlineLevelBoxGeometry = GetFormattingContext().GeometryForBox(layoutBox);
            return !inlineLevelBoxGeometry.MarginBefore() && !inlineLevelBoxGeometry.MarginAfter()
                   && inlineLevelBoxGeometry.MarginBoxHeight().ToFloat()
                        <= rootInlineBox.LayoutBounds().Ascent;
          }
          if (inlineLevelBox.IsLineBreakBox())
          {
            // Baseline aligned, non-stretchy line breaks e.g. <div><span><br></span></div> but not <div><span
            // style="font-size: 100px;"><br></span></div>.
            return inlineLevelBox.LayoutBounds().Ascent <= rootInlineBox.LayoutBounds().Ascent;
          }
          if (inlineLevelBox.IsInlineBox())
          {
            // Baseline aligned, non-stretchy inline boxes e.g. <div><span></span></div> but not <div><span
            // style="font-size: 100px;"></span></div>.
            return inlineLevelBox.LayoutBounds() == rootInlineBox.LayoutBounds();
          }
          return false;
        };
        if (!shouldUseSimplifiedAlignmentForInlineLevelBox())
        {
          return false;
        }
      }
      return true;
    };

    if (canUseSimplifiedAlignment())
    {
      return SimplifiedVerticalAlignment(lineBox);
    }
    // This function (partially) implements:
    // 2.2. Layout Within Line Boxes
    // https://www.w3.org/TR/css-inline-3/#line-layout
    // 1. Compute the line box height using the layout bounds geometry. This height computation strictly uses
    // layout bounds and not normal inline level box geometries.
    // 2. Compute the baseline/logical top position of the root inline box. Aligned boxes push the root inline
    // box around inside the line box.
    // 3. Finally align the inline level boxes using (mostly) normal inline level box geometries.
    krys_debug_assert(lineBox.HasContent());
    auto lineBoxAlignmentContent = ComputeLineBoxLogicalHeight(lineBox);
    ComputeRootInlineBoxVerticalPosition(lineBox, lineBoxAlignmentContent);

    auto lineBoxHeight = lineBoxAlignmentContent.Height();
    AlignInlineLevelBoxes(lineBox, lineBoxHeight);
    if (lineBoxAlignmentContent.HasTextEmphasis)
    {
      lineBoxHeight = AdjustForAnnotationIfNeeded(lineBox, lineBoxHeight);
    }
    return lineBoxHeight;
  }

  InlineLayoutUnit LineBoxVerticalAligner::SimplifiedVerticalAlignment(LineBox &lineBox) const noexcept
  {
    auto &rootInlineBox = lineBox.RootInlineBox();
    auto rootInlineBoxAscent = rootInlineBox.Ascent();

    if (!lineBox.HasContent())
    {
      rootInlineBox.SetLogicalTop(-rootInlineBoxAscent);
      for (auto &inlineLevelBox : lineBox.NonRootInlineLevelBoxes())
      {
        krys_debug_assert(inlineLevelBox.LayoutBox().IsWordBreakOpportunity()
                          || (inlineLevelBox.IsInlineBox() && !inlineLevelBox.HasContent()));
        inlineLevelBox.SetLogicalTop(rootInlineBoxAscent);
      }
      return {};
    }

    auto rootInlineBoxLayoutBounds = rootInlineBox.LayoutBounds();
    auto lineBoxLogicalTop = InlineLayoutUnit {0};
    auto lineBoxLogicalBottom = rootInlineBoxLayoutBounds.Height();
    auto rootInlineBoxLogicalTop = rootInlineBoxLayoutBounds.Ascent - rootInlineBoxAscent;
    for (auto &inlineLevelBox : lineBox.NonRootInlineLevelBoxes())
    {
      // Only baseline alignment for now.
      inlineLevelBox.SetLogicalTop(rootInlineBoxAscent - inlineLevelBox.Ascent());
      auto layoutBounds = inlineLevelBox.LayoutBounds();
      auto layoutBoundsLogicalTop = rootInlineBoxLayoutBounds.Ascent - layoutBounds.Ascent;
      lineBoxLogicalTop = std::min(lineBoxLogicalTop, layoutBoundsLogicalTop);
      lineBoxLogicalBottom = std::max(lineBoxLogicalBottom, layoutBoundsLogicalTop + layoutBounds.Height());
      rootInlineBoxLogicalTop = std::max(rootInlineBoxLogicalTop, layoutBounds.Ascent - rootInlineBoxAscent);
    }
    rootInlineBox.SetLogicalTop(rootInlineBoxLogicalTop);
    return lineBoxLogicalBottom - lineBoxLogicalTop;
  }

  InlineLayoutUnit LineBoxVerticalAligner::LogicalTopOffsetFromParentBaseline(
    const InlineLevelBox &inlineLevelBox, const InlineLevelBox &parentInlineBox,
    IsInlineLevelBoxAlignment isInlineLevelBoxAlignment) const noexcept
  {
    krys_debug_assert(parentInlineBox.IsInlineBox());

    auto ascent = isInlineLevelBoxAlignment == IsInlineLevelBoxAlignment::Yes
                    ? inlineLevelBox.Ascent()
                    : inlineLevelBox.LayoutBounds().Ascent;
    auto height = isInlineLevelBoxAlignment == IsInlineLevelBoxAlignment::Yes
                    ? inlineLevelBox.LogicalHeight()
                    : inlineLevelBox.LayoutBounds().Height();

    return krys::SwitchOn(
      inlineLevelBox.VerticalAlign(), [&](const css::keywords::Baseline &) -> InlineLayoutUnit
      { return ascent; }, [&](const css::keywords::Sub &) -> InlineLayoutUnit
      { return ascent - (parentInlineBox.FontSize() / 5 + 1); },
      [&](const css::keywords::Super &) -> InlineLayoutUnit
      { return ascent + parentInlineBox.FontSize() / 3 + 1; },
      [&](const css::keywords::Top &) -> InlineLayoutUnit
      {
        krys_debug_assert(false);
        return {};
      },
      [&](const css::keywords::TextTop &) -> InlineLayoutUnit
      {
        if (isInlineLevelBoxAlignment == IsInlineLevelBoxAlignment::No)
        {
          return parentInlineBox.Ascent();
        }
        // Note that text-top aligns with the inline box's font metrics top (ascent) and not the layout bounds
        // top.
        return parentInlineBox.Ascent() + (inlineLevelBox.Ascent() - inlineLevelBox.LayoutBounds().Ascent);
      },
      [&](const css::keywords::Middle &) -> InlineLayoutUnit
      { return height / 2 + parentInlineBox.PrimarymetricsOfPrimaryFont().XHeight().value_or(0.f) / 2; },
      [&](const css::keywords::Bottom &) -> InlineLayoutUnit
      {
        krys_debug_assert(false);
        return {};
      },
      [&](const css::keywords::TextBottom &) -> InlineLayoutUnit
      {
        if (isInlineLevelBoxAlignment == IsInlineLevelBoxAlignment::No)
        {
          return height - parentInlineBox.Descent();
        }
        // Note that text-bottom aligns with the inline box's font metrics bottom (descent) and not the layout
        // bounds bottom.
        return (inlineLevelBox.Ascent() + inlineLevelBox.LayoutBounds().Descent) - parentInlineBox.Descent();
      },
      [&](const css::keywords::WebkitBaselineMiddle &) -> InlineLayoutUnit { return height / 2; },
      [&](const InlineLayoutUnit &baselineOffset) -> InlineLayoutUnit { return baselineOffset + ascent; });
  }

  LineBoxVerticalAligner::LineBoxAlignmentContent
    LineBoxVerticalAligner::ComputeLineBoxLogicalHeight(LineBox &lineBox) const noexcept
  {
    // This function (partially) implements:
    // 2.2. Layout Within Line Boxes
    // https://www.w3.org/TR/css-inline-3/#line-layout
    // 1. Compute the line box height using the layout bounds geometry. This height computation strictly uses
    // layout bounds and not normal inline level box geometries.
    // 2. Compute the baseline/logical top position of the root inline box. Aligned boxes push the root inline
    // box around inside the line box.
    // 3. Finally align the inline level boxes using (mostly) normal inline level box geometries.
    auto &rootInlineBox = lineBox.RootInlineBox();
    auto &formattingUtils = FormattingUtils();
    auto contentHasTextEmphasis = rootInlineBox.HasTextEmphasis();

    // Line box height computation is based on the layout bounds of the inline boxes and not their logical
    // (ascent/descent) dimensions.
    struct AbsoluteTopAndBottom
    {
      InlineLayoutUnit Top {0};
      InlineLayoutUnit Bottom {0};
    };
    HashMap<const InlineLevelBox *, AbsoluteTopAndBottom> inlineLevelBoxAbsoluteTopAndBottomMap;

    auto minimumLogicalTop = Maybe<InlineLayoutUnit> {};
    auto maximumLogicalBottom = Maybe<InlineLayoutUnit> {};
    if (formattingUtils.InlineLevelBoxAffectsLineBox(rootInlineBox))
    {
      minimumLogicalTop = InlineLayoutUnit {};
      maximumLogicalBottom = rootInlineBox.LayoutBounds().Height();
      inlineLevelBoxAbsoluteTopAndBottomMap.emplace(
        &rootInlineBox, AbsoluteTopAndBottom {*minimumLogicalTop, *maximumLogicalBottom});
    }
    else
    {
      inlineLevelBoxAbsoluteTopAndBottomMap.emplace(&rootInlineBox, AbsoluteTopAndBottom {});
    }

    List<const InlineLevelBox *> lineBoxRelativeInlineLevelBoxes;
    for (auto &inlineLevelBox : lineBox.NonRootInlineLevelBoxes())
    {
      contentHasTextEmphasis = contentHasTextEmphasis || inlineLevelBox.HasTextEmphasis();

      if (inlineLevelBox.HasLineBoxRelativeAlignment())
      {
        lineBoxRelativeInlineLevelBoxes.push_back(&inlineLevelBox);
        continue;
      }
      auto &parentInlineBox = lineBox.ParentInlineBox(inlineLevelBox);
      auto inlineBoxTopOffsetFromParentBaseline =
        LogicalTopOffsetFromParentBaseline(inlineLevelBox, parentInlineBox);
      // Logical top is relative to the parent inline box's layout bounds.
      // Note that this logical top is not the final logical top of the inline level box.
      // This is the logical top in the context of the layout bounds geometry which may be very different from
      // the inline box's normal geometry.
      auto inlineLevelBoxLogicalTop =
        parentInlineBox.LayoutBounds().Ascent - inlineBoxTopOffsetFromParentBaseline;
      auto parentInlineBoxAbsoluteTopAndBottom = AbsoluteTopAndBottom {};
      if (auto entry = inlineLevelBoxAbsoluteTopAndBottomMap.find(&parentInlineBox);
          entry != inlineLevelBoxAbsoluteTopAndBottomMap.end())
      {
        parentInlineBoxAbsoluteTopAndBottom = entry->second;
      }
      auto absoluteLogicalTop = parentInlineBoxAbsoluteTopAndBottom.Top + inlineLevelBoxLogicalTop;
      auto absoluteLogicalBottom = absoluteLogicalTop + inlineLevelBox.LayoutBounds().Height();
      inlineLevelBoxAbsoluteTopAndBottomMap.emplace(
        &inlineLevelBox, AbsoluteTopAndBottom {absoluteLogicalTop, absoluteLogicalBottom});
      // Stretch the min/max absolute values if applicable.
      if (formattingUtils.InlineLevelBoxAffectsLineBox(inlineLevelBox))
      {
        minimumLogicalTop = std::min(minimumLogicalTop.value_or(absoluteLogicalTop), absoluteLogicalTop);
        maximumLogicalBottom =
          std::max(maximumLogicalBottom.value_or(absoluteLogicalBottom), absoluteLogicalBottom);
      }
    }
    // The line box height computation is as follows:
    // 1. Stretch the line box with the non-line-box relative aligned inline box absolute top and bottom
    // values.
    // 2. Check if the line box relative aligned inline boxes (top, bottom etc) have enough room and stretch
    // the line box further if needed.
    auto nonLineBoxRelativeAlignedBoxesMaximumHeight =
      maximumLogicalBottom.value_or(0.f) - minimumLogicalTop.value_or(0.f);
    auto topAlignedBoxesMaximumHeight = Maybe<InlineLayoutUnit> {};
    auto bottomAlignedBoxesMaximumHeight = Maybe<InlineLayoutUnit> {};
    for (auto *lineBoxRelativeInlineLevelBox : lineBoxRelativeInlineLevelBoxes)
    {
      if (!formattingUtils.InlineLevelBoxAffectsLineBox(*lineBoxRelativeInlineLevelBox))
      {
        continue;
      }
      // This line box relative aligned inline level box stretches the line box.
      auto inlineLevelBoxHeight = lineBoxRelativeInlineLevelBox->LayoutBounds().Height();
      if (HoldsAlternative<css::keywords::Top>(lineBoxRelativeInlineLevelBox->VerticalAlign()))
      {
        topAlignedBoxesMaximumHeight =
          std::max(inlineLevelBoxHeight, topAlignedBoxesMaximumHeight.value_or(0.f));
        continue;
      }
      if (HoldsAlternative<css::keywords::Bottom>(lineBoxRelativeInlineLevelBox->VerticalAlign()))
      {
        bottomAlignedBoxesMaximumHeight =
          std::max(inlineLevelBoxHeight, bottomAlignedBoxesMaximumHeight.value_or(0.f));
        continue;
      }
      krys_debug_assert(false);
    }
    return {nonLineBoxRelativeAlignedBoxesMaximumHeight,
            {topAlignedBoxesMaximumHeight, bottomAlignedBoxesMaximumHeight},
            contentHasTextEmphasis};
  }

  void LineBoxVerticalAligner::ComputeRootInlineBoxVerticalPosition(
    LineBox &lineBox, const LineBoxAlignmentContent &lineBoxAlignmentContent) const noexcept
  {
    auto &rootInlineBox = lineBox.RootInlineBox();
    auto &formattingUtils = FormattingUtils();
    auto hasTopAlignedInlineLevelBox = false;

    HashMap<const InlineLevelBox *, InlineLayoutUnit> inlineLevelBoxAbsoluteBaselineOffsetMap;
    inlineLevelBoxAbsoluteBaselineOffsetMap.emplace(&rootInlineBox, InlineLayoutUnit {});

    auto maximumTopOffsetFromRootInlineBoxBaseline = Maybe<InlineLayoutUnit> {};
    if (formattingUtils.InlineLevelBoxAffectsLineBox(rootInlineBox))
    {
      maximumTopOffsetFromRootInlineBoxBaseline = rootInlineBox.LayoutBounds().Ascent;
    }

    auto affectsRootInlineBoxVerticalPosition = [&](const InlineLevelBox &inlineLevelBox)
    {
      return formattingUtils.InlineLevelBoxAffectsLineBox(inlineLevelBox);
    };

    for (auto &inlineLevelBox : lineBox.NonRootInlineLevelBoxes())
    {
      auto layoutBounds = inlineLevelBox.LayoutBounds();

      if (inlineLevelBox.HasLineBoxRelativeAlignment())
      {
        auto &verticalAlign = inlineLevelBox.VerticalAlign();
        if (HoldsAlternative<css::keywords::Top>(verticalAlign))
        {
          hasTopAlignedInlineLevelBox =
            hasTopAlignedInlineLevelBox || affectsRootInlineBoxVerticalPosition(inlineLevelBox);
          inlineLevelBoxAbsoluteBaselineOffsetMap.emplace(&inlineLevelBox, rootInlineBox.LayoutBounds().Ascent
                                                                             - layoutBounds.Ascent);
        }
        else if (HoldsAlternative<css::keywords::Bottom>(verticalAlign))
        {
          inlineLevelBoxAbsoluteBaselineOffsetMap.emplace(
            &inlineLevelBox, layoutBounds.Descent - rootInlineBox.LayoutBounds().Descent);
        }
        else
        {
          krys_debug_assert(false);
        }
        continue;
      }
      auto &parentInlineBox = lineBox.ParentInlineBox(inlineLevelBox);
      auto inlineBoxTopOffsetFromParentBaseline =
        LogicalTopOffsetFromParentBaseline(inlineLevelBox, parentInlineBox);
      auto baselineOffsetFromParentBaseline = inlineBoxTopOffsetFromParentBaseline - layoutBounds.Ascent;
      auto parentAbsoluteBaselineOffset = InlineLayoutUnit {};
      if (auto entry = inlineLevelBoxAbsoluteBaselineOffsetMap.find(&parentInlineBox);
          entry != inlineLevelBoxAbsoluteBaselineOffsetMap.end())
      {
        parentAbsoluteBaselineOffset = entry->second;
      }
      auto absoluteBaselineOffset = parentAbsoluteBaselineOffset + baselineOffsetFromParentBaseline;
      inlineLevelBoxAbsoluteBaselineOffsetMap.emplace(&inlineLevelBox, absoluteBaselineOffset);

      if (affectsRootInlineBoxVerticalPosition(inlineLevelBox))
      {
        auto topOffsetFromRootInlineBoxBaseline = absoluteBaselineOffset + layoutBounds.Ascent;
        if (maximumTopOffsetFromRootInlineBoxBaseline)
        {
          maximumTopOffsetFromRootInlineBoxBaseline =
            std::max(*maximumTopOffsetFromRootInlineBoxBaseline, topOffsetFromRootInlineBoxBaseline);
        }
        else
        {
          // We are is quirk mode and the root inline box has no content. The root inline box's baseline is
          // anchored at 0. However negative ascent (e.g negative top margin) can "push" the root inline box
          // upwards and have a negative value.
          maximumTopOffsetFromRootInlineBoxBaseline = layoutBounds.Ascent >= 0
                                                        ? std::max(0.0f, topOffsetFromRootInlineBoxBaseline)
                                                        : topOffsetFromRootInlineBoxBaseline;
        }
      }
    }

    if (!maximumTopOffsetFromRootInlineBoxBaseline && hasTopAlignedInlineLevelBox)
    {
      // vertical-align: top is a line box relative alignment. It stretches the line box downwards meaning
      // that it does not affect the root inline box's baseline position, but in quirks mode we have to ensure
      // that the root inline box does not end up at 0px.
      maximumTopOffsetFromRootInlineBoxBaseline = rootInlineBox.Ascent();
    }
    // vertical-align: bottom stretches the top of the line box pushing the root inline box downwards.
    auto &topAndBottomAlignedMaximumHeight = lineBoxAlignmentContent.TopAndBottomAlignedMaximumHeightValue;
    auto bottomAlignedBoxStretch = topAndBottomAlignedMaximumHeight.Bottom.value_or(0.f);
    if (bottomAlignedBoxStretch)
    {
      // If top happens to stretch the line box, we don't need to push the root inline box anymore.
      if (bottomAlignedBoxStretch <= topAndBottomAlignedMaximumHeight.Top.value_or(0.f))
      {
        bottomAlignedBoxStretch = {};
      }

      // However non-line box relative content needs some space. Root inline box may not end up being at the
      // very bottom.
      if (bottomAlignedBoxStretch)
      {
        if (lineBoxAlignmentContent.NonLineBoxRelativeAlignedMaximumHeight > 0)
        {
          // Negative vertical margin can make aligned boxes have negative height.
          bottomAlignedBoxStretch -=
            std::max(0.f, lineBoxAlignmentContent.NonLineBoxRelativeAlignedMaximumHeight);
        }
        bottomAlignedBoxStretch = std::max(0.f, bottomAlignedBoxStretch);
      }
    }
    auto rootInlineBoxLogicalTop = bottomAlignedBoxStretch
                                   + maximumTopOffsetFromRootInlineBoxBaseline.value_or(0.f)
                                   - rootInlineBox.Ascent();
    rootInlineBox.SetLogicalTop(rootInlineBoxLogicalTop);
  }

  Maybe<InlineLevelBox::AscentAndDescent> LineBoxVerticalAligner::LayoutBoundsForInlineBoxSubtree(
    const LineBox::InlineLevelBoxList &nonRootInlineLevelBoxes, size_t inlineBoxIndex) const noexcept
  {
    // https://w3c.github.io/csswg-drafts/css2/#propdef-vertical-align
    //
    // top/bottom values align the element relative to the line box.
    // Since the element may have children aligned relative to it (which in turn may have descendants aligned
    // relative to them), these values use the bounds of the aligned subtree. The aligned subtree of an inline
    // element contains that element and the aligned subtrees of all children inline elements whose computed
    // vertical-align value is not top or bottom. The top of the aligned subtree is the highest of the tops of
    // the boxes in the subtree, and the bottom is analogous.
    krys_debug_assert(nonRootInlineLevelBoxes[inlineBoxIndex].IsInlineBox());
    auto &formattingUtils = FormattingUtils();
    auto maximumAscent = Maybe<InlineLayoutUnit> {};
    auto maximumDescent = Maybe<InlineLayoutUnit> {};
    auto &inlineBox = nonRootInlineLevelBoxes[inlineBoxIndex];
    auto &inlineBoxParent = inlineBox.LayoutBox().Parent();
    for (size_t index = inlineBoxIndex + 1; index < nonRootInlineLevelBoxes.size(); ++index)
    {
      auto &descendantInlineLevelBox = nonRootInlineLevelBoxes[index];
      if (&descendantInlineLevelBox.LayoutBox().Parent() == &inlineBoxParent)
      {
        // We are at the end of the descendant list.
        break;
      }
      if (!formattingUtils.InlineLevelBoxAffectsLineBox(descendantInlineLevelBox)
          || descendantInlineLevelBox.HasLineBoxRelativeAlignment())
      {
        continue;
      }

      // ascent/descent here really mean enclosing geometry adjusted by vertical alignemnt, which is in case
      // of baseline alignment is simply layout bounds but e.g. with middle alignment, "ascent and descent"
      // are inline level box height / 2.
      auto ascent = LogicalTopOffsetFromParentBaseline(descendantInlineLevelBox, inlineBox);
      auto descent = descendantInlineLevelBox.LayoutBounds().Height() - ascent;
      maximumAscent = std::max(ascent, maximumAscent.value_or(ascent));
      maximumDescent = std::max(descent, maximumDescent.value_or(descent));
    }
    if (maximumAscent)
    {
      krys_debug_assert(maximumDescent.has_value());
      return InlineLevelBox::AscentAndDescent {*maximumAscent, *maximumDescent};
    }
    return null;
  }

  void LineBoxVerticalAligner::AlignInlineLevelBoxes(LineBox &lineBox,
                                                     InlineLayoutUnit lineBoxLogicalHeight) const noexcept
  {
    List<size_t> lineBoxRelativeInlineLevelBoxes;
    auto &nonRootInlineLevelBoxes = lineBox.NonRootInlineLevelBoxes();
    for (size_t index = 0; index < nonRootInlineLevelBoxes.size(); ++index)
    {
      auto &inlineLevelBox = nonRootInlineLevelBoxes[index];
      if (inlineLevelBox.HasLineBoxRelativeAlignment())
      {
        lineBoxRelativeInlineLevelBoxes.push_back(index);
        continue;
      }
      auto &parentInlineBox = lineBox.ParentInlineBox(inlineLevelBox);
      auto inlineBoxTopOffsetFromParentBaseline =
        LogicalTopOffsetFromParentBaseline(inlineLevelBox, parentInlineBox, IsInlineLevelBoxAlignment::Yes);
      auto inlineLevelBoxLogicalTop = parentInlineBox.Ascent() - inlineBoxTopOffsetFromParentBaseline;
      inlineLevelBox.SetLogicalTop(inlineLevelBoxLogicalTop);
    }

    for (auto index : lineBoxRelativeInlineLevelBoxes)
    {
      auto &inlineLevelBox = nonRootInlineLevelBoxes[index];
      auto logicalTop = InlineLayoutUnit {};
      krys::SwitchOn(
        inlineLevelBox.VerticalAlign(),
        [&](const css::keywords::Top &)
        {
          auto ascent = inlineLevelBox.LayoutBounds().Ascent;
          if (inlineLevelBox.IsInlineBox())
          {
            if (auto descendantsEnclosingGeometry =
                  LayoutBoundsForInlineBoxSubtree(nonRootInlineLevelBoxes, index))
            {
              ascent = !inlineLevelBox.HasContent() ? descendantsEnclosingGeometry->Ascent
                                                    : std::max(descendantsEnclosingGeometry->Ascent, ascent);
            }
          }
          // Note that this logical top is not relative to the parent inline box.
          logicalTop = ascent - inlineLevelBox.Ascent();
        },
        [&](const css::keywords::Bottom &)
        {
          auto descent = inlineLevelBox.LayoutBounds().Descent;
          if (inlineLevelBox.IsInlineBox())
          {
            if (auto descendantsEnclosingGeometry =
                  LayoutBoundsForInlineBoxSubtree(nonRootInlineLevelBoxes, index))
            {
              descent = !inlineLevelBox.HasContent()
                          ? descendantsEnclosingGeometry->Descent
                          : std::max(descendantsEnclosingGeometry->Descent, descent);
            }
          }
          // Note that this logical top is not relative to the parent inline box.
          logicalTop = lineBoxLogicalHeight - (inlineLevelBox.Ascent() + descent);
        },
        [](const auto &) { krys_debug_assert(false); });
      inlineLevelBox.SetLogicalTop(logicalTop);
    }
  }

  InlineLayoutUnit
    LineBoxVerticalAligner::AdjustForAnnotationIfNeeded(LineBox &lineBox,
                                                        InlineLayoutUnit lineBoxHeight) const noexcept
  {
    auto lineBoxTop = InlineLayoutUnit {};
    auto lineBoxBottom = lineBoxHeight;
    // At this point we have a properly aligned set of inline level boxes. Let's find out if annotation marks
    // have enough space.
    auto adjustLineBoxHeightIfNeeded = [&]
    {
      auto adjustLineBoxTopAndBottomForInlineBox = [&](const InlineLevelBox &inlineLevelBox)
      {
        krys_debug_assert(inlineLevelBox.IsInlineBox() || inlineLevelBox.IsAtomicInlineBox());
        auto inlineBoxTop = lineBox.InlineLevelBoxAbsoluteTop(inlineLevelBox);
        auto inlineBoxBottom = inlineBoxTop + inlineLevelBox.LogicalHeight();

        auto defaultCase = [&]
        {
          if (auto aboveSpace = inlineLevelBox.TextEmphasisAbove())
          {
            lineBoxTop = std::min(lineBoxTop, inlineBoxTop - *aboveSpace);
          }
          if (auto belowSpace = inlineLevelBox.TextEmphasisBelow())
          {
            lineBoxBottom = std::max(lineBoxBottom, inlineBoxBottom + *belowSpace);
          }
        };

        krys::SwitchOn(
          inlineLevelBox.VerticalAlign(),
          [&](const css::keywords::Top &)
          {
            // FIXME: Check if horizontal vs. vertical writing mode should be taking into account.
            auto annotationSpace = inlineLevelBox.TextEmphasisAbove().value_or(0.f)
                                   + inlineLevelBox.TextEmphasisBelow().value_or(0.f);
            lineBoxBottom = std::max(lineBoxBottom, inlineBoxBottom + annotationSpace);
          },
          [&](const auto &) { defaultCase(); });
      };

      adjustLineBoxTopAndBottomForInlineBox(lineBox.RootInlineBox());
      for (auto &inlineLevelBox : lineBox.NonRootInlineLevelBoxes())
      {
        if (inlineLevelBox.IsInlineBox() || inlineLevelBox.IsAtomicInlineBox())
        {
          adjustLineBoxTopAndBottomForInlineBox(inlineLevelBox);
        }
      }

      return lineBoxBottom - lineBoxTop;
    };
    auto adjustedLineBoxHeight = adjustLineBoxHeightIfNeeded();

    if (lineBoxHeight != adjustedLineBoxHeight)
    {
      // Annotations needs some space.
      auto adjustContentTopWithAnnotationSpace = [&]
      {
        auto &rootInlineBox = lineBox.RootInlineBox();
        auto rootInlineBoxTop = rootInlineBox.LogicalTop();
        auto annotationOffset = -lineBoxTop;
        rootInlineBox.SetLogicalTop(annotationOffset + rootInlineBoxTop);

        for (auto &inlineLevelBox : lineBox.NonRootInlineLevelBoxes())
        {
          krys::SwitchOn(
            inlineLevelBox.VerticalAlign(),
            [&](const css::keywords::Top &)
            {
              auto inlineBoxTop = inlineLevelBox.LayoutBounds().Ascent - inlineLevelBox.Ascent();
              inlineLevelBox.SetLogicalTop(inlineLevelBox.TextEmphasisAbove().value_or(0.f) + inlineBoxTop);
            },
            [&](const css::keywords::Bottom &)
            {
              auto inlineBoxTop =
                adjustedLineBoxHeight - (inlineLevelBox.LayoutBounds().Descent + inlineLevelBox.Ascent());
              inlineLevelBox.SetLogicalTop(inlineBoxTop - inlineLevelBox.TextEmphasisBelow().value_or(0.f));
            },
            [](const auto &)
            {
              // These alignment positions are relative to the root inline box's baseline.
            });
        }
      };
      adjustContentTopWithAnnotationSpace();
    }
    return adjustedLineBoxHeight;
  }
}
