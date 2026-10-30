#include "Krystal.Booey/Layout/FormattingContexts/Inline/IntrinsicWidthHandler.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineBuilder.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/TextOnlySimpleLineBuilder.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo::layout
{
  namespace
  {
    KRYS_NODISCARD bool IsBoxEligibleForNonLineBuilderMinimumWidth(const ElementBox &box) noexcept
    {
      // Note that hanging trailing content needs line builder (combination of wrapping is allowed but
      // whitespace is preserved).
      auto &style = box.Style();
      return TextUtil::IsWrappingAllowed(style)
             && (style.GetLineBreak() == LineBreak::Anywhere || style.GetWordBreak() == WordBreak::BreakAll
                 || style.GetWordBreak() == WordBreak::BreakWord)
             && style.GetWhiteSpaceCollapse() != WhiteSpaceCollapse::Preserve;
    }

    KRYS_NODISCARD bool
      IsContentEligibleForNonLineBuilderMaximumWidth(const ElementBox &rootBox,
                                                     const InlineItemList &inlineItemList) noexcept
    {
      if (inlineItemList.size() != 1
          || rootBox.Style().GetTextIndent() != style::ComputedStyle::InitialTextIndent())
      {
        return false;
      }

      auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItemList[0]);
      return inlineTextItem && !inlineTextItem->IsWhitespace();
    }

    KRYS_NODISCARD bool IsSubtreeEligibleForNonLineBuilderMinimumWidth(const ElementBox &root) noexcept
    {
      auto isSimpleBreakableContent = IsBoxEligibleForNonLineBuilderMinimumWidth(root);
      for (auto *child = root.FirstChild(); child && isSimpleBreakableContent; child = child->NextSibling())
      {
        if (child->IsFloatingPositioned())
        {
          isSimpleBreakableContent = false;
          break;
        }
        auto isInlineBoxWithInlineContent =
          child->IsInlineBox() && !child->IsInlineTextBox() && !child->IsLineBreakBox();
        if (isInlineBoxWithInlineContent)
        {
          isSimpleBreakableContent =
            IsSubtreeEligibleForNonLineBuilderMinimumWidth(Downcast<ElementBox>(*child));
        }
      }
      return isSimpleBreakableContent;
    }

    KRYS_NODISCARD bool
      IsContentEligibleForNonLineBuilderMinimumWidth(const ElementBox &rootBox,
                                                     bool mayUseSimplifiedTextOnlyInlineLayout) noexcept
    {
      return (mayUseSimplifiedTextOnlyInlineLayout && IsBoxEligibleForNonLineBuilderMinimumWidth(rootBox))
             || (!mayUseSimplifiedTextOnlyInlineLayout
                 && IsSubtreeEligibleForNonLineBuilderMinimumWidth(rootBox));
    }

    KRYS_NODISCARD bool
      MayUseContentWidthBetweenLineBreaksAsMaximumSize(const ElementBox &rootBox,
                                                       const InlineItemList &inlineItemList) noexcept
    {
      if (!TextUtil::ShouldPreserveSpacesAndTabs(rootBox))
      {
        return false;
      }
      for (auto &inlineItem : inlineItemList)
      {
        if (auto *inlineTextItem = DynamicDowncast<InlineTextItem>(inlineItem);
            inlineTextItem && !inlineTextItem->Width())
        {
          // We can't accumulate individual inline items when width depends on position (e.g. tab).
          return false;
        }
      }
      return true;
    }
  }

  IntrinsicWidthHandler::IntrinsicWidthHandler(InlineFormattingContext &inlineFormattingContext,
                                               const InlineContentCache::InlineItems &inlineItems) noexcept
      : _inlineFormattingContext(inlineFormattingContext), _inlineItems(inlineItems)
  {
    auto initializeRangeAndTextOnlyBuilderEligibility = [&]
    {
      _inlineItemRange = {0, inlineItems.Content().size()};
      _mayUseSimplifiedTextOnlyInlineLayoutInRange =
        TextOnlySimpleLineBuilder::IsEligibleForSimplifiedInlineLayoutByStyle(FormattingContextRoot());
      if (!_mayUseSimplifiedTextOnlyInlineLayoutInRange)
      {
        return;
      }

      _mayUseSimplifiedTextOnlyInlineLayoutInRange =
        inlineItems.HasTextAndLineBreakOnlyContent() && !inlineItems.RequiresVisualReordering();
      if (!_mayUseSimplifiedTextOnlyInlineLayoutInRange)
      {
        return;
      }
      // Non-bidi text only content maybe nested inside inline boxes e.g. <div>simple text</div>,
      // <div><span>simple text inside inline box</span></div> or <div>some text<span>and some more inside
      // inline box</span></div>
      auto inlineBoxCount = inlineItems.InlineBoxCount();
      if (!inlineBoxCount)
      {
        return;
      }

      auto &inlineItemList = inlineItems.Content();
      auto inlineBoxStartAndEndInlineItemsCount = 2 * inlineBoxCount;
      krys_debug_assert(inlineBoxStartAndEndInlineItemsCount <= inlineItemList.size());

      _mayUseSimplifiedTextOnlyInlineLayoutInRange =
        inlineBoxStartAndEndInlineItemsCount < inlineItemList.size();
      if (!_mayUseSimplifiedTextOnlyInlineLayoutInRange)
      {
        return;
      }

      for (size_t index = 0; index < inlineBoxCount; ++index)
      {
        auto &inlineItem = inlineItemList[index];
        auto isNestingInlineBox =
          inlineItem.IsInlineBoxStart() && inlineItemList[inlineItems.Size() - 1 - index].IsInlineBoxEnd();
        _mayUseSimplifiedTextOnlyInlineLayoutInRange =
          isNestingInlineBox
          && !GetFormattingContext().GeometryForBox(inlineItem.LayoutBox()).HorizontalMarginBorderAndPadding()
          && TextOnlySimpleLineBuilder::IsEligibleForSimplifiedInlineLayoutByStyle(inlineItem.LayoutBox());
        if (!_mayUseSimplifiedTextOnlyInlineLayoutInRange)
        {
          return;
        }
      }
      _inlineItemRange = {inlineBoxCount, inlineItemList.size() - inlineBoxCount};
    };
    initializeRangeAndTextOnlyBuilderEligibility();
  }

  InlineLayoutUnit IntrinsicWidthHandler::MinimumContentSize() noexcept
  {
    auto minimumContentSize = InlineLayoutUnit {};

    if (IsContentEligibleForNonLineBuilderMinimumWidth(FormattingContextRoot(),
                                                       _mayUseSimplifiedTextOnlyInlineLayoutInRange))
    {
      minimumContentSize = SimplifiedMinimumWidth(FormattingContextRoot());
    }
    else if (_mayUseSimplifiedTextOnlyInlineLayoutInRange)
    {
      auto simplifiedLineBuilder =
        TextOnlySimpleLineBuilder {GetFormattingContext(), LineBuilerRoot(), {}, GetInlineItemList()};
      minimumContentSize = ComputedIntrinsicWidthForConstraint(
        IntrinsicWidthMode::Minimum, simplifiedLineBuilder, MayCacheLayoutResult::No);
    }
    else
    {
      auto lineBuilder = LineBuilder {GetFormattingContext(), {}, GetInlineItemList()};
      minimumContentSize = ComputedIntrinsicWidthForConstraint(IntrinsicWidthMode::Minimum, lineBuilder,
                                                               MayCacheLayoutResult::No);
    }

    return minimumContentSize;
  }

  InlineLayoutUnit IntrinsicWidthHandler::MaximumContentSize() noexcept
  {
    auto mayCacheLayoutResult = _mayUseSimplifiedTextOnlyInlineLayoutInRange && !_inlineItemRange.StartIndex()
                                  ? MayCacheLayoutResult::Yes
                                  : MayCacheLayoutResult::No;
    auto maximumContentSize = InlineLayoutUnit {};

    if (IsContentEligibleForNonLineBuilderMaximumWidth(FormattingContextRoot(), GetInlineItemList()))
    {
      maximumContentSize = SimplifiedMaximumWidth(mayCacheLayoutResult);
    }
    else if (_mayUseSimplifiedTextOnlyInlineLayoutInRange)
    {
      if (_maximumContentWidthBetweenLineBreaks
          && MayUseContentWidthBetweenLineBreaksAsMaximumSize(FormattingContextRoot(), GetInlineItemList()))
      {
        maximumContentSize = *_maximumContentWidthBetweenLineBreaks;
      }
      else
      {
        auto simplifiedLineBuilder =
          TextOnlySimpleLineBuilder {GetFormattingContext(), LineBuilerRoot(), {}, GetInlineItemList()};
        maximumContentSize = ComputedIntrinsicWidthForConstraint(IntrinsicWidthMode::Maximum,
                                                                 simplifiedLineBuilder, mayCacheLayoutResult);
      }
    }
    else
    {
      auto lineBuilder = LineBuilder {GetFormattingContext(), {}, GetInlineItemList()};
      maximumContentSize =
        ComputedIntrinsicWidthForConstraint(IntrinsicWidthMode::Maximum, lineBuilder, mayCacheLayoutResult);
    }

    return maximumContentSize;
  }

  InlineLayoutUnit IntrinsicWidthHandler::ComputedIntrinsicWidthForConstraint(
    IntrinsicWidthMode intrinsicWidthMode, AbstractLineBuilder &lineBuilder,
    MayCacheLayoutResult mayCacheLayoutResult) noexcept
  {
    auto layoutRange = _inlineItemRange;
    if (layoutRange.IsEmpty())
    {
      return {};
    }

    auto availableWidth = intrinsicWidthMode == IntrinsicWidthMode::Maximum ? MaxInlineLayoutUnit() : 0.f;
    auto maximumContentWidth = InlineLayoutUnit {};
    struct ContentWidthBetweenLineBreaks
    {
      InlineLayoutUnit Maximum {};
      InlineLayoutUnit Current {};
    };
    auto contentWidthBetweenLineBreaks = ContentWidthBetweenLineBreaks {};
    auto previousLineEnd = Maybe<InlineItemPosition> {};
    auto previousLine = Maybe<PreviousLine> {};
    size_t lineIndex = 0;
    auto isFirstFormattedLineCandidate = true;
    lineBuilder.SetIntrinsicWidthMode(intrinsicWidthMode);

    while (true)
    {
      auto lineLayoutResult = lineBuilder.LayoutInlineContent({layoutRange, {0.f, 0.f, availableWidth, 0.f}},
                                                              previousLine, isFirstFormattedLineCandidate);
      auto floatContentWidth = [&]
      {
        auto leftWidth = InlineLayoutUnit {};
        auto rightWidth = InlineLayoutUnit {};
        for (auto &floatItem : lineLayoutResult.Floats.PlacedFloats)
        {
          mayCacheLayoutResult = MayCacheLayoutResult::No;
          auto marginBoxRect = BoxGeometry::MarginBoxRect(floatItem.GetBoxGeometry());
          if (floatItem.IsStartPositioned())
          {
            leftWidth = std::max<InlineLayoutUnit>(leftWidth, marginBoxRect.Right().ToFloat());
          }
          else
          {
            rightWidth =
              std::max<InlineLayoutUnit>(rightWidth, availableWidth - marginBoxRect.Left().ToFloat());
          }
        }
        return InlineLayoutUnit {leftWidth + rightWidth};
      };

      auto lineContentLogicalWidth = [&]
      {
        auto contentWidth = lineLayoutResult.Geometry.LogicalTopLeft.X()
                            + lineLayoutResult.Content.LogicalWidth + floatContentWidth();
        if (lineLayoutResult.Runs.empty())
        {
          return contentWidth;
        }
        auto &leadingRun = lineLayoutResult.Runs.front();
        if (leadingRun.IsListMarkerOutside())
        {
          contentWidth -= leadingRun.LogicalRight();
        }
        return contentWidth;
      }();
      maximumContentWidth = std::max(maximumContentWidth, lineContentLogicalWidth);
      contentWidthBetweenLineBreaks.Current +=
        (lineContentLogicalWidth + lineLayoutResult.Hanging.LogicalWidth);
      if (lineLayoutResult.EndsWithLineBreak())
      {
        contentWidthBetweenLineBreaks = {
          std::max(contentWidthBetweenLineBreaks.Maximum, contentWidthBetweenLineBreaks.Current), {}};
      }

      layoutRange.Start = InlineFormattingUtils::LeadingInlineItemPositionForNextLine(
        lineLayoutResult.ItemRange.End, previousLineEnd,
        !lineLayoutResult.Floats.HasIntrusiveFloat.IsEmpty() || !lineLayoutResult.Floats.PlacedFloats.empty(),
        layoutRange.End);
      if (layoutRange.IsEmpty())
      {
        auto cacheLineBreakingResultForSubsequentLayoutIfApplicable = [&]
        {
          _maximumIntrinsicWidthResultForSingleLine = {};
          if (mayCacheLayoutResult == MayCacheLayoutResult::No)
          {
            return;
          }
          _maximumIntrinsicWidthResultForSingleLine = krys::move(lineLayoutResult);
        };
        cacheLineBreakingResultForSubsequentLayoutIfApplicable();
        break;
      }

      // Support single line only.
      mayCacheLayoutResult = MayCacheLayoutResult::No;
      previousLineEnd = layoutRange.Start;
      previousLine = PreviousLine {lineIndex++,
                                   lineLayoutResult.Content.TrailingOverflowingContentWidth,
                                   lineLayoutResult.EndsWithLineBreak(),
                                   {},
                                   krys::move(lineLayoutResult.Floats.SuspendedFloats)};
      isFirstFormattedLineCandidate &= !lineLayoutResult.HasContentfulInFlowContent();
    }
    _maximumContentWidthBetweenLineBreaks =
      std::max(contentWidthBetweenLineBreaks.Current, contentWidthBetweenLineBreaks.Maximum);
    return maximumContentWidth;
  }

  InlineLayoutUnit IntrinsicWidthHandler::SimplifiedMinimumWidth(const ElementBox &root) const noexcept
  {
    auto maximumWidth = InlineLayoutUnit {};

    for (auto *child = root.FirstChild(); child; child = child->NextInFlowSibling())
    {
      if (auto *inlineTextBox = DynamicDowncast<InlineTextBox>(*child))
      {
        krys_debug_assert(inlineTextBox->Style().GetWhiteSpaceCollapse() != WhiteSpaceCollapse::Preserve);
        auto &fontCascade = inlineTextBox->Style().GetFontCascade();
        auto &content = inlineTextBox->Content();
        auto contentLength = content.length();
        size_t index = 0;
        auto isTreatedAsSpaceCharacter = [&](char8 character)
        {
          return character == u8' ' || character == u8'\n' || character == u8'\t';
        };
        while (index < contentLength)
        {
          auto characterLength =
            TextUtil::FirstUserPerceivedCharacterLength(*inlineTextBox, index, contentLength - index);
          krys_debug_assert(characterLength);
          auto isCollapsedWhitespace = characterLength == 1 && isTreatedAsSpaceCharacter(content[index]);
          if (!isCollapsedWhitespace)
          {
            maximumWidth = std::max(
              maximumWidth, TextUtil::Width(*inlineTextBox, fontCascade, static_cast<uint32>(index),
                                            static_cast<uint32>(index + characterLength), {},
                                            TextUtil::UseTrailingWhitespaceMeasuringOptimization::No));
          }
          index += characterLength;
        }
        continue;
      }
      if (child->IsAtomicInlineBox() || child->IsReplacedBox())
      {
        maximumWidth = std::max<InlineLayoutUnit>(
          maximumWidth, GetFormattingContext().GeometryForBox(*child).MarginBoxWidth().ToFloat());
        continue;
      }
      auto isInlineBoxWithInlineContent = child->IsInlineBox() && !child->IsLineBreakBox();
      if (isInlineBoxWithInlineContent)
      {
        auto &boxGeometry = GetFormattingContext().GeometryForBox(*child);
        maximumWidth = std::max(
          maximumWidth, std::max<InlineLayoutUnit>(boxGeometry.MarginBorderAndPaddingStart().ToFloat(),
                                                   boxGeometry.MarginBorderAndPaddingEnd().ToFloat()));
        maximumWidth = std::max(maximumWidth, SimplifiedMinimumWidth(Downcast<ElementBox>(*child)));
        continue;
      }
    }
    return maximumWidth;
  }

  InlineLayoutUnit
    IntrinsicWidthHandler::SimplifiedMaximumWidth(MayCacheLayoutResult mayCacheLayoutResult) noexcept
  {
    krys_debug_assert(FormattingContextRoot().FirstChild()
                      && FormattingContextRoot().FirstChild() == FormattingContextRoot().LastChild());
    auto &inlineTextItem = Downcast<InlineTextItem>(GetInlineItemList()[0]);
    auto &style = inlineTextItem.FirstLineStyle();

    auto contentLogicalWidth = [&]
    {
      if (auto width = inlineTextItem.Width())
      {
        return *width;
      }
      return TextUtil::Width(inlineTextItem, style.GetFontCascade(), {});
    }();
    if (mayCacheLayoutResult == MayCacheLayoutResult::No)
    {
      return contentLogicalWidth;
    }

    auto line = Line {GetFormattingContext()};
    line.Initialize({}, true);
    line.AppendTextFast(inlineTextItem, style, contentLogicalWidth);
    auto lineContent = line.Close();

    krys_debug_assert(contentLogicalWidth == lineContent.ContentLogicalWidth);

    LineLayoutResult lineLayoutResult;
    lineLayoutResult.ItemRange = {0, 1};
    lineLayoutResult.Runs = krys::move(lineContent.Runs);
    lineLayoutResult.Content = {{}, lineContent.ContentLogicalWidth, lineContent.ContentLogicalRight, {}};
    lineLayoutResult.FirstLast = {IsFirstFormattedLine::Yes, true};
    lineLayoutResult.ContentEnding = LineLayoutResult::InlineContentEnding::Generic;
    _maximumIntrinsicWidthResultForSingleLine = krys::move(lineLayoutResult);
    return contentLogicalWidth;
  }

  const ElementBox &IntrinsicWidthHandler::FormattingContextRoot() const noexcept
  {
    return _inlineFormattingContext.Root();
  }

  const ElementBox &IntrinsicWidthHandler::LineBuilerRoot() const noexcept
  {
    if (!_inlineItemRange.StartIndex())
    {
      return FormattingContextRoot();
    }

    auto rootBoxIndex = _inlineItemRange.StartIndex() - 1;
    auto &inlineItems = GetInlineItemList();
    if (rootBoxIndex >= inlineItems.size())
    {
      krys_debug_assert(false);
      return FormattingContextRoot();
    }

    if (auto *inlineBox = DynamicDowncast<ElementBox>(inlineItems[rootBoxIndex].LayoutBox());
        inlineBox && inlineBox->IsInlineBox())
    {
      // We are running a range based line building where we only need to layout the inner text content (e.g.
      // <span>inner text content</span>)
      return *inlineBox;
    }

    krys_debug_assert(false);
    return FormattingContextRoot();
  }
}
