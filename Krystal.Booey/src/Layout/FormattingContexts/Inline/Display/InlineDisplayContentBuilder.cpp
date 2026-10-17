#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayContentBuilder.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/FormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingUtils.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInitialContainingBlock.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/InlineTextBoxStyle.hpp"
#include "Krystal.Booey/Style/Values/Borders/Shadow.hpp"
#include "Krystal.Core/Text/Unicode.hpp"

#include <algorithm>
#include <ranges>

namespace krys::boo::layout
{
  namespace
  {
    KRYS_NODISCARD inline LayoutUnit MarginLineLeft(const BoxGeometry &boxGeometry,
                                                    WritingMode writingMode) noexcept
    {
      return writingMode.IsBidiLTR() ? boxGeometry.MarginStart() : boxGeometry.MarginEnd();
    }

    KRYS_NODISCARD inline LayoutUnit MarginLineRight(const BoxGeometry &boxGeometry,
                                                     WritingMode writingMode) noexcept
    {
      return writingMode.IsBidiLTR() ? boxGeometry.MarginEnd() : boxGeometry.MarginStart();
    }

    KRYS_NODISCARD inline LayoutUnit BorderLineLeft(const BoxGeometry &boxGeometry,
                                                    WritingMode writingMode) noexcept
    {
      return writingMode.IsBidiLTR() ? boxGeometry.BorderStart() : boxGeometry.BorderEnd();
    }

    KRYS_NODISCARD inline LayoutUnit BorderLineRight(const BoxGeometry &boxGeometry,
                                                     WritingMode writingMode) noexcept
    {
      return writingMode.IsBidiLTR() ? boxGeometry.BorderEnd() : boxGeometry.BorderStart();
    }

    KRYS_NODISCARD inline LayoutUnit PaddingLineLeft(const BoxGeometry &boxGeometry,
                                                     WritingMode writingMode) noexcept
    {
      return writingMode.IsBidiLTR() ? boxGeometry.PaddingStart() : boxGeometry.PaddingEnd();
    }

    KRYS_NODISCARD inline LayoutUnit PaddingLineRight(const BoxGeometry &boxGeometry,
                                                      WritingMode writingMode) noexcept
    {
      return writingMode.IsBidiLTR() ? boxGeometry.PaddingEnd() : boxGeometry.PaddingStart();
    }

    KRYS_NODISCARD inline EnumSet<inline_display::Box::PositionWithinInlineLevelBox>
      IsFirstLastBox(const InlineLevelBox &inlineBox) noexcept
    {
      auto positionWithinInlineLevelBox = EnumSet<inline_display::Box::PositionWithinInlineLevelBox> {};
      if (inlineBox.IsFirstBox())
      {
        positionWithinInlineLevelBox.Add(inline_display::Box::PositionWithinInlineLevelBox::First);
      }
      if (inlineBox.IsLastBox())
      {
        positionWithinInlineLevelBox.Add(inline_display::Box::PositionWithinInlineLevelBox::Last);
      }
      return positionWithinInlineLevelBox;
    }

    KRYS_NODISCARD inline bool ComputeInkOverflowForInlineLevelBox(const RenderStyle &style,
                                                                   FloatRect &inkOverflow) noexcept
    {
      auto hasInkOverflow = false;

      auto inflateWithOutline = [&]
      {
        if (!style.HasOutlineInVisualOverflow())
        {
          return;
        }
        inkOverflow.Inflate(style.OutlineSize());
        hasInkOverflow = true;
      };
      inflateWithOutline();

      auto inflateWithBoxShadow = [&]
      {
        auto [topBoxShadow, bottomBoxShadow] =
          style::ShadowVerticalExtent(style.GetBoxShadow(), style.UsedZoomForLength());
        auto [leftBoxShadow, rightBoxShadow] =
          style::ShadowHorizontalExtent(style.GetBoxShadow(), style.UsedZoomForLength());
        if (!topBoxShadow && !bottomBoxShadow && !leftBoxShadow && !rightBoxShadow)
        {
          return;
        }
        inkOverflow.Inflate(-leftBoxShadow.ToFloat(), -topBoxShadow.ToFloat(), rightBoxShadow.ToFloat(),
                            bottomBoxShadow.ToFloat());
        hasInkOverflow = true;
      };
      inflateWithBoxShadow();

      return hasInkOverflow;
    }

    KRYS_NODISCARD inline bool ComputeInkOverflowForInlineBox(const InlineLevelBox &inlineBox,
                                                              const RenderStyle &style,
                                                              FloatRect &inkOverflow) noexcept
    {
      krys_debug_assert(inlineBox.IsInlineBox());
      auto hasInkOverflow = ComputeInkOverflowForInlineLevelBox(style, inkOverflow);

      auto inflateWithAnnotation = [&]
      {
        if (!inlineBox.HasTextEmphasis())
        {
          return;
        }
        inkOverflow.Inflate(0.f, inlineBox.TextEmphasisAbove().value_or(0.f), 0.f,
                            inlineBox.TextEmphasisBelow().value_or(0.f));
        hasInkOverflow = true;
      };
      inflateWithAnnotation();

      return hasInkOverflow;
    }

    KRYS_NODISCARD inline bool
      IsNestedInlineBoxWithDifferentFontCascadeFromParent(const Box &layoutBox,
                                                          bool isFirstFormattedLine) noexcept
    {
      if (!layoutBox.Parent().IsInlineBox())
      {
        return false;
      }
      auto &style = isFirstFormattedLine ? layoutBox.FirstLineStyle() : layoutBox.Style();
      auto &parentStyle =
        isFirstFormattedLine ? layoutBox.Parent().FirstLineStyle() : layoutBox.Parent().Style();
      return !(style.GetFontCascade() == parentStyle.GetFontCascade());
    }

    KRYS_NODISCARD inline size_t RunIndex(size_t i, size_t listSize, bool isBidiLTR) noexcept
    {
      if (isBidiLTR)
      {
        return i;
      }
      auto lastIndex = listSize - 1;
      return lastIndex - i;
    }

    void SetGeometryForOutOfFlowBoxes(const List<size_t> &indexListOfOutOfFlowBoxes,
                                      Maybe<size_t> firstOutOfFlowIndexWithPreviousInflowSibling,
                                      const Line::RunList &lineRuns, const List<int32> &visualOrderList,
                                      InlineFormattingContext &formattingContext, const LineBox &lineBox,
                                      const ConstraintsForInlineContent &constraints) noexcept
    {
      auto isBidiLTR = formattingContext.Root().GetWritingMode().IsBidiLTR();
      auto outOfFlowBoxListSize = indexListOfOutOfFlowBoxes.size();

      auto outOfFlowBox = [&](size_t i) -> const Box &
      {
        auto outOfFlowRunIndex = indexListOfOutOfFlowBoxes[RunIndex(i, outOfFlowBoxListSize, isBidiLTR)];
        return (visualOrderList.empty() ? lineRuns[outOfFlowRunIndex]
                                        : lineRuns[static_cast<size_t>(visualOrderList[outOfFlowRunIndex])])
          .LayoutBox();
      };
      // Set geometry on "before inflow content" boxes first, followed by the "after inflow content" list.
      auto beforeAfterBoundary = firstOutOfFlowIndexWithPreviousInflowSibling.value_or(outOfFlowBoxListSize);
      // These out of flow boxes "sit" on the line start (they are before any inflow content e.g. <div><div
      // class=out-of-flow></div>some text<div>)
      auto logicalTopLeft =
        LayoutPoint {constraints.Horizontal().LogicalLeft, LayoutUnit {lineBox.LogicalRect().Top()}};
      for (size_t i = 0; i < beforeAfterBoundary; ++i)
      {
        formattingContext.GeometryForBox(outOfFlowBox(i)).SetTopLeft(logicalTopLeft);
      }
      // These out of flow boxes are all _after_ an inflow content and get "wrapped" to the next line.
      logicalTopLeft.SetY(LayoutUnit {lineBox.LogicalRect().Bottom()});
      for (size_t i = beforeAfterBoundary; i < outOfFlowBoxListSize; ++i)
      {
        formattingContext.GeometryForBox(outOfFlowBox(i)).SetTopLeft(logicalTopLeft);
      }
    }

    KRYS_NODISCARD float LogicalBottomForTextDecorationContent(const inline_display::Boxes &boxes,
                                                               bool isHorizontalWritingMode) noexcept
    {
      auto logicalBottom = Maybe<float> {};
      for (auto &displayBox : boxes)
      {
        if (displayBox.IsRootInlineBox())
        {
          continue;
        }
        if (!displayBox.Style().TextDecorationLineInEffect().HasUnderline())
        {
          continue;
        }
        if (displayBox.IsText()
            || displayBox.Style().GetTextDecorationSkipInk() == TextDecorationSkipInk::None)
        {
          auto contentLogicalBottom = isHorizontalWritingMode ? displayBox.Bottom() : displayBox.Right();
          logicalBottom =
            logicalBottom ? std::max(*logicalBottom, contentLogicalBottom) : contentLogicalBottom;
        }
      }
      // This function is not called unless there's at least one run on the line with an underline.
      krys_debug_assert(logicalBottom.has_value());
      return logicalBottom.value_or(0.f);
    }
  }

  struct DisplayBoxTree
  {
  public:
    struct Node
    {
      // Node's parent index in _displayBoxNodes.
      Maybe<size_t> ParentIndex {};
      // Associated display box index in inline_display::Boxes.
      size_t DisplayBoxIndex {0};
      // Child indexes in _displayBoxNodes.
      List<size_t> Children {};
    };

  private:
    List<Node> _displayBoxNodes;

  public:
    DisplayBoxTree() noexcept
    {
      _displayBoxNodes.reserve(10);
      _displayBoxNodes.push_back({});
    }

    KRYS_NODISCARD const Node &Root() const noexcept
    {
      return _displayBoxNodes.front();
    }
    KRYS_NODISCARD Node &At(size_t index) noexcept
    {
      return _displayBoxNodes[index];
    }
    KRYS_NODISCARD const Node &At(size_t index) const noexcept
    {
      return _displayBoxNodes[index];
    }

    size_t Append(size_t parentNodeIndex, size_t childDisplayBoxIndex) noexcept
    {
      auto childDisplayBoxNodeIndex = _displayBoxNodes.size();
      _displayBoxNodes.push_back({parentNodeIndex, childDisplayBoxIndex});
      At(parentNodeIndex).Children.push_back(childDisplayBoxNodeIndex);
      return childDisplayBoxNodeIndex;
    }
  };

  struct AncestorStack
  {
  private:
    List<size_t> _stack;
    List<const ElementBox *> _set;

  public:
    KRYS_NODISCARD Maybe<size_t> Unwind(const ElementBox &elementBox) noexcept
    {
      // Unwind the stack all the way to container box.
      if (std::ranges::find(_set, &elementBox) == _set.end())
      {
        return null;
      }
      while (_set.back() != &elementBox)
      {
        _stack.pop_back();
        _set.pop_back();
      }
      // Root is always a common ancestor.
      krys_debug_assert(!_stack.empty());
      return _stack.back();
    }

    void Push(size_t displayBoxNodeIndexForContainer, const ElementBox &elementBox) noexcept
    {
      _stack.push_back(displayBoxNodeIndexForContainer);
      krys_debug_assert(std::ranges::find(_set, &elementBox) == _set.end());
      _set.push_back(&elementBox);
    }
  };

  struct IsFirstLastIndex
  {
    Maybe<size_t> First {};
    Maybe<size_t> Last {};
  };
  using IsFirstLastIndexesMap = HashMap<const Box *, IsFirstLastIndex>;

  namespace
  {
    inline size_t CreateDisplayBoxNodeForContainerAndPushToAncestorStack(
      const ElementBox &elementBox, size_t displayBoxIndex, size_t parentDisplayBoxNodeIndex,
      DisplayBoxTree &displayBoxTree, AncestorStack &ancestorStack) noexcept
    {
      auto displayBoxNodeIndex = displayBoxTree.Append(parentDisplayBoxNodeIndex, displayBoxIndex);
      ancestorStack.Push(displayBoxNodeIndex, elementBox);
      return displayBoxNodeIndex;
    }
  }

  InlineDisplayContentBuilder::InlineDisplayContentBuilder(InlineFormattingContext &formattingContext,
                                                           const ConstraintsForInlineContent &constraints,
                                                           const LineBox &lineBox,
                                                           const inline_display::Line &displayLine) noexcept
      : _formattingContext(formattingContext), _constraints(constraints), _lineBox(lineBox),
        _displayLine(displayLine)
  {
    auto &initialContainingBlockGeometry =
      std::as_const(_formattingContext)
        .GeometryForBox(
          FormattingContext::GetInitialContainingBlock(Root()),
          InlineFormattingContext::EscapeReason::InkOverflowNeedsInitialContiningBlockForStrokeWidth);
    _initialContaingBlockSize = CeiledIntSize(LayoutSize {initialContainingBlockGeometry.ContentBoxWidth(),
                                                          initialContainingBlockGeometry.ContentBoxHeight()});
  }

  const ElementBox &InlineDisplayContentBuilder::Root() const noexcept
  {
    return _formattingContext.Root();
  }

  inline_display::Boxes InlineDisplayContentBuilder::Build(const LineLayoutResult &lineLayoutResult) noexcept
  {
    auto boxes = inline_display::Boxes {};
    boxes.reserve(lineLayoutResult.Runs.size() + GetLineBox().NonRootInlineLevelBoxes().size() + 1);

    auto contentNeedsBidiReordering = !lineLayoutResult.BidiDirectionality.VisualOrderList.empty();
    if (contentNeedsBidiReordering)
    {
      ProcessBidiContent(lineLayoutResult, boxes);
    }
    else
    {
      ProcessNonBidiContent(lineLayoutResult, boxes);
    }
    // FIXME: Process ruby content (annotation boxes, alignment offsets and overhang).

    CollectInkOverflowForTextDecorations(boxes);
    CollectInkOverflowForInlineBoxes(boxes);
    return boxes;
  }

  void InlineDisplayContentBuilder::AppendTextDisplayBox(const Line::Run &lineRun,
                                                         const InlineRect &textRunRect,
                                                         inline_display::Boxes &boxes) noexcept
  {
    krys_debug_assert(lineRun.TextContent() && Is<InlineTextBox>(lineRun.LayoutBox()));

    auto &inlineTextBox = Downcast<InlineTextBox>(lineRun.LayoutBox());
    auto &style = IsFirstFormattedLine() ? inlineTextBox.FirstLineStyle() : inlineTextBox.Style();
    auto &content = inlineTextBox.Content();
    auto &text = lineRun.TextContent();
    auto isContentful = true;

    _hasSeenTextDecoration =
      _hasSeenTextDecoration
      || !(IsFirstFormattedLine() ? inlineTextBox.Parent().FirstLineStyle().TextDecorationLineInEffect()
                                  : inlineTextBox.Parent().Style().TextDecorationLineInEffect())
            .IsNone();

    auto inkOverflow = [&]
    {
      auto inkOverflow = textRunRect;

      auto addLetterSpacingOverflow = [&]
      {
        auto letterSpacing = style.GetFontCascade().LetterSpacing();
        if (letterSpacing >= 0)
        {
          return;
        }
        // Large negative letter spacing can produce text boxes with negative width (when glyphs position
        // order gets completely backwards (123 turns into 321 starting at an offset)) Such spacing should go
        // to ink overflow.
        auto textRunWidth = textRunRect.Width();
        if (textRunWidth < 0)
        {
          inkOverflow.SetWidth({});
          inkOverflow.ShiftLeftTo(textRunWidth);
        }
        // Last letter's negative spacing shrinks logical rect. Push it to ink overflow.
        inkOverflow.Expand(-letterSpacing, {});
      };
      addLetterSpacingOverflow();

      auto addStrokeOverflow = [&]
      {
        inkOverflow.Inflate(std::ceil(style.UsedStrokeWidth(_initialContaingBlockSize)));
      };
      addStrokeOverflow();

      auto addTextShadow = [&]
      {
        auto textShadow = style::ShadowOutsetExtent(style.GetTextShadow(), style.UsedZoomForLength());
        inkOverflow.Inflate(-textShadow.Top().ToFloat(), textShadow.Right().ToFloat(),
                            textShadow.Bottom().ToFloat(), -textShadow.Left().ToFloat());
      };
      addTextShadow();

      auto addGlyphOverflow = [&]
      {
        if (inlineTextBox.CanUseSimpleFontCodePath())
        {
          // CanUseSimpleFontCodePath maps to CodePath::Simple (and content with potential glyph overflow
          // would says CodePath::SimpleWithGlyphOverflow).
          return;
        }
        auto enclosingAscentAndDescent = TextUtil::EnclosingGlyphBoundsForText(
          utf8_stringview {content}.substr(text->Start, text->Length), style,
          inlineTextBox.ShouldUseSimpleGlyphOverflowCodePath()
            ? TextUtil::ShouldUseSimpleGlyphOverflowCodePath::Yes
            : TextUtil::ShouldUseSimpleGlyphOverflowCodePath::No);
        // FIXME: Take fallback fonts into account.
        auto &fontMetrics = style.MetricsOfPrimaryFont();
        auto topOverflow = std::max(
          0.f, InlineFormattingUtils::SnapToInt(-enclosingAscentAndDescent.Ascent, inlineTextBox)
                 - InlineFormattingUtils::Ascent(fontMetrics, FontBaseline::Alphabetic, inlineTextBox));
        auto bottomOverflow = std::max(
          0.f, InlineFormattingUtils::SnapToInt(enclosingAscentAndDescent.Descent, inlineTextBox)
                 - InlineFormattingUtils::Descent(fontMetrics, FontBaseline::Alphabetic, inlineTextBox));
        inkOverflow.Inflate(topOverflow, {}, bottomOverflow, {});
      };
      addGlyphOverflow();

      return inkOverflow;
    }();

    _contentHasInkOverflow =
      _contentHasInkOverflow || (&inlineTextBox.Parent() != &Root() && textRunRect != inkOverflow);

    if (inlineTextBox.IsCombined())
    {
      static const auto objectReplacementCharacterString = utf8_string {u8"\uFFFC"};
      // The rendered text is the actual combined content, while the "original" one is blank. The box covers
      // the first character of the content.
      auto firstCharacterLength =
        text->Start < content.size()
          ? text::DecodeUTF8(Span<const char8>(content.data() + text->Start, content.size() - text->Start))
              .Length
          : size_t {0};
      boxes.push_back({LineIndex(), inline_display::Box::Type::Text, inlineTextBox, lineRun.BidiLevel(),
                       textRunRect, inkOverflow, IsFirstFormattedLine(), lineRun.Expansion(),
                       inline_display::Box::Text {text->Start, firstCharacterLength,
                                                  objectReplacementCharacterString, utf8_string {content}},
                       isContentful, IsLineFullyTruncatedInBlockDirection()});
      return;
    }

    auto adjustedContentToRender = [&]() -> Maybe<utf8_string>
    {
      if (!text->NeedsHyphen)
      {
        return null;
      }
      auto adjustedContent = utf8_string {utf8_stringview {content}.substr(text->Start, text->Length)};
      adjustedContent += style.HyphenString().View();
      return adjustedContent;
    };
    auto shapingBoundary = [&]
    {
      if (lineRun.IsShapingBoundaryStart())
      {
        return inline_display::Box::Text::ShapingBoundary::Start;
      }
      if (lineRun.IsShapingBoundaryEnd())
      {
        return inline_display::Box::Text::ShapingBoundary::End;
      }
      if (lineRun.IsInsideShapingBoundary())
      {
        return inline_display::Box::Text::ShapingBoundary::Inside;
      }
      return inline_display::Box::Text::ShapingBoundary::NotApplicable;
    };
    boxes.push_back(
      {LineIndex(),
       lineRun.IsWordSeparator() ? inline_display::Box::Type::WordSeparator : inline_display::Box::Type::Text,
       inlineTextBox, lineRun.BidiLevel(), textRunRect, inkOverflow, IsFirstFormattedLine(),
       lineRun.Expansion(),
       inline_display::Box::Text {text->Start, text->Length, content, adjustedContentToRender(),
                                  text->NeedsHyphen, shapingBoundary()},
       isContentful, IsLineFullyTruncatedInBlockDirection()});
  }

  void InlineDisplayContentBuilder::AppendSoftLineBreakDisplayBox(const Line::Run &lineRun,
                                                                  const InlineRect &softLineBreakRunRect,
                                                                  inline_display::Boxes &boxes) noexcept
  {
    krys_debug_assert(lineRun.TextContent() && Is<InlineTextBox>(lineRun.LayoutBox()));

    auto &layoutBox = lineRun.LayoutBox();
    auto &text = lineRun.TextContent();
    auto isContentful = true;

    boxes.push_back(
      {LineIndex(), inline_display::Box::Type::SoftLineBreak, layoutBox, lineRun.BidiLevel(),
       softLineBreakRunRect, softLineBreakRunRect, IsFirstFormattedLine(), lineRun.Expansion(),
       inline_display::Box::Text {text->Start, text->Length, Downcast<InlineTextBox>(layoutBox).Content()},
       isContentful, IsLineFullyTruncatedInBlockDirection()});
  }

  void InlineDisplayContentBuilder::AppendHardLineBreakDisplayBox(const Line::Run &lineRun,
                                                                  const InlineRect &lineBreakBoxRect,
                                                                  inline_display::Boxes &boxes) noexcept
  {
    auto isContentful = true;
    boxes.push_back({LineIndex(), inline_display::Box::Type::LineBreakBox, lineRun.LayoutBox(),
                     lineRun.BidiLevel(), lineBreakBoxRect, lineBreakBoxRect, IsFirstFormattedLine(),
                     lineRun.Expansion(), null, isContentful, IsLineFullyTruncatedInBlockDirection()});
  }

  void InlineDisplayContentBuilder::AppendAtomicInlineLevelDisplayBox(const Line::Run &lineRun,
                                                                      const InlineRect &borderBoxRect,
                                                                      inline_display::Boxes &boxes) noexcept
  {
    krys_debug_assert(lineRun.LayoutBox().IsAtomicInlineBox());
    auto &layoutBox = lineRun.LayoutBox();

    auto isContentful = true;
    auto inkOverflow = [&]
    {
      auto inkOverflow = FloatRect {borderBoxRect};
      auto &style = IsFirstFormattedLine() ? layoutBox.FirstLineStyle() : layoutBox.Style();
      (void)ComputeInkOverflowForInlineLevelBox(style, inkOverflow);
      // Atomic inline box contribute to their inline box parents ink overflow at all times (e.g.
      // <span><img></span>).
      _contentHasInkOverflow = _contentHasInkOverflow || &layoutBox.Parent() != &Root();
      return inkOverflow;
    };

    boxes.push_back({LineIndex(), inline_display::Box::Type::AtomicInlineBox, layoutBox, lineRun.BidiLevel(),
                     borderBoxRect, inkOverflow(), IsFirstFormattedLine(), lineRun.Expansion(), null,
                     isContentful, IsLineFullyTruncatedInBlockDirection()});
  }

  void InlineDisplayContentBuilder::AppendBlockLevelDisplayBox(const Line::Run &lineRun,
                                                               const InlineRect &borderBoxRect,
                                                               inline_display::Boxes &boxes) noexcept
  {
    krys_debug_assert(lineRun.IsBlock());
    auto &layoutBox = lineRun.LayoutBox();

    auto isContentful = true;
    boxes.push_back({LineIndex(),
                     inline_display::Box::Type::BlockLevelBox,
                     layoutBox,
                     lineRun.BidiLevel(),
                     borderBoxRect,
                     borderBoxRect,
                     IsFirstFormattedLine(),
                     {},
                     null,
                     isContentful,
                     false});
  }

  void InlineDisplayContentBuilder::AppendRootInlineBoxDisplayBox(const InlineRect &rootInlineBoxVisualRect,
                                                                  bool lineHasContent,
                                                                  inline_display::Boxes &boxes) noexcept
  {
    boxes.push_back({LineIndex(),
                     inline_display::Box::Type::RootInlineBox,
                     Root(),
                     UBIDI_DEFAULT_LTR,
                     rootInlineBoxVisualRect,
                     rootInlineBoxVisualRect,
                     IsFirstFormattedLine(),
                     {},
                     null,
                     lineHasContent,
                     IsLineFullyTruncatedInBlockDirection()});
  }

  void InlineDisplayContentBuilder::AppendInlineBoxDisplayBox(const Line::Run &lineRun,
                                                              const InlineLevelBox &inlineBox,
                                                              const InlineRect &inlineBoxBorderBox,
                                                              inline_display::Boxes &boxes) noexcept
  {
    krys_debug_assert(lineRun.LayoutBox().IsInlineBox());
    krys_debug_assert(inlineBox.IsInlineBox());
    krys_debug_assert((inlineBox.IsFirstBox() && lineRun.IsInlineBoxStart())
                      || (!inlineBox.IsFirstBox() && lineRun.IsLineSpanningInlineBoxStart()));

    auto &layoutBox = lineRun.LayoutBox();
    _hasSeenNestedInlineBoxesWithDifferentFontCascade =
      _hasSeenNestedInlineBoxesWithDifferentFontCascade
      || IsNestedInlineBoxWithDifferentFontCascadeFromParent(layoutBox, IsFirstFormattedLine());

    auto inkOverflow = [&]
    {
      auto &style = IsFirstFormattedLine() ? layoutBox.FirstLineStyle() : layoutBox.Style();
      auto inkOverflow = FloatRect {inlineBoxBorderBox};
      _contentHasInkOverflow =
        ComputeInkOverflowForInlineBox(inlineBox, style, inkOverflow) || _contentHasInkOverflow;
      return inkOverflow;
    };

    boxes.push_back({LineIndex(),
                     inline_display::Box::Type::NonRootInlineBox,
                     layoutBox,
                     lineRun.BidiLevel(),
                     inlineBoxBorderBox,
                     inkOverflow(),
                     IsFirstFormattedLine(),
                     {},
                     null,
                     inlineBox.HasContent(),
                     IsLineFullyTruncatedInBlockDirection(),
                     IsFirstLastBox(inlineBox)});
  }

  void
    InlineDisplayContentBuilder::AppendInlineDisplayBoxAtBidiBoundary(const Box &layoutBox,
                                                                      inline_display::Boxes &boxes) noexcept
  {
    // Geometries for inline boxes at bidi boundaries are computed at a post-process step.
    auto isContentful = true;
    boxes.push_back({LineIndex(),
                     inline_display::Box::Type::NonRootInlineBox,
                     layoutBox,
                     UBIDI_DEFAULT_LTR,
                     {},
                     {},
                     IsFirstFormattedLine(),
                     {},
                     null,
                     isContentful,
                     IsLineFullyTruncatedInBlockDirection()});
  }

  void InlineDisplayContentBuilder::ProcessNonBidiContent(const LineLayoutResult &lineLayoutResult,
                                                          inline_display::Boxes &boxes) noexcept
  {
#ifndef NDEBUG
    auto hasContent = false;
    for (auto &lineRun : lineLayoutResult.Runs)
    {
      hasContent = hasContent || lineRun.IsContentful();
    }
    krys_debug_assert(lineLayoutResult.BidiDirectionality.InlineBaseDirection == TextDirection::LTR
                      || !hasContent);
#endif
    auto &lineBox = GetLineBox();
    auto writingMode = Root().Style().GetWritingMode();
    auto lineBoxLogicalRect = lineBox.LogicalRect();
    auto lineBoxVisualOffset = _displayLine.TopLeft();
    auto lineHasBlockContent = lineLayoutResult.IsBlockContent();

    auto rootLogicalRect = lineBox.LogicalRectForRootInlineBox();
    auto rootVisualRect = MapInlineRectLogicalToVisual(rootLogicalRect, lineBoxLogicalRect, writingMode);
    rootVisualRect.MoveBy(lineBoxVisualOffset);
    AppendRootInlineBoxDisplayBox(rootVisualRect, lineBox.RootInlineBox().HasContent(), boxes);

    List<size_t> blockLevelOutOfFlowBoxList;
    auto textSpacingAdjustment = 0.f;
    for (size_t index = 0; index < lineLayoutResult.Runs.size(); ++index)
    {
      auto &lineRun = lineLayoutResult.Runs[index];
      if (lineRun.IsWordBreakOpportunity() || lineRun.IsInlineBoxEnd())
      {
        continue;
      }
      auto &layoutBox = lineRun.LayoutBox();

      if (lineRun.IsOpaque())
      {
        if (layoutBox.Style().IsOriginalDisplayInlineType())
        {
          GetFormattingContext().GeometryForBox(layoutBox).SetTopLeft(
            LayoutPoint {LayoutUnit {lineBox.LogicalRect().Left()
                                     + lineBox.LogicalRectForRootInlineBox().Left() + lineRun.LogicalLeft()},
                         LayoutUnit {lineBox.LogicalRect().Top()}});
          continue;
        }
        blockLevelOutOfFlowBoxList.push_back(index);
        continue;
      }

      auto adjustLogicalRectForTextSpacing = [&](InlineRect &rect)
      {
        rect.ExpandHorizontally(-textSpacingAdjustment);
        rect.MoveHorizontally(textSpacingAdjustment);
      };

      auto logicalRect = [&]() -> InlineRect
      {
        if (lineRun.IsText())
        {
          auto rect = lineBox.LogicalRectForTextRun(lineRun);
          if (textSpacingAdjustment)
          {
            adjustLogicalRectForTextSpacing(rect);
            // text-spacing adjustment is retrivied while handling the run corresponding to the inline box
            // start and must be reset here.
            textSpacingAdjustment = 0;
          }
          return rect;
        }
        if (lineRun.IsSoftLineBreak())
        {
          return lineBox.LogicalRectForTextRun(lineRun);
        }
        if (lineRun.IsHardLineBreak())
        {
          return lineBox.LogicalRectForLineBreakBox(layoutBox);
        }

        auto &boxGeometry = GetFormattingContext().GeometryForBox(layoutBox);
        if (lineRun.IsAtomicInlineBox() || lineRun.IsListMarker())
        {
          return lineBox.LogicalBorderBoxForAtomicInlineBox(layoutBox, boxGeometry);
        }
        if (lineRun.IsInlineBoxStart())
        {
          auto rect = lineBox.LogicalBorderBoxForInlineBox(layoutBox, boxGeometry);
          // The Run representing the InlineBoxStart transports the text spacing adjustment. The adjustment is
          // used here and for the rect representing the following text node. Therefore, we should reset such
          // adjustment when handling the next run which is text.
          if ((textSpacingAdjustment = lineRun.TextSpacingAdjustment()))
          {
            adjustLogicalRectForTextSpacing(rect);
          }
          return rect;
        }
        if (lineRun.IsLineSpanningInlineBoxStart())
        {
          // Ideally spanning inline boxes on block lines would not have borders and padding.
          if (lineHasBlockContent)
          {
            return lineBox.LogicalContentBoxForInlineBox(layoutBox);
          }
          return lineBox.LogicalBorderBoxForInlineBox(layoutBox, boxGeometry);
        }
        if (lineRun.IsBlock())
        {
          krys_debug_assert(lineHasBlockContent);
          auto borderBoxRect = BoxGeometry::BorderBoxRect(boxGeometry);
          return {borderBoxRect.Top().ToFloat(), borderBoxRect.Left().ToFloat(),
                  borderBoxRect.Width().ToFloat(), borderBoxRect.Height().ToFloat()};
        }

        krys_debug_assert(false);
        return {};
      }();

      auto visualRectRelativeToRoot = [&]
      {
        auto visualRect = MapInlineRectLogicalToVisual(logicalRect, lineBoxLogicalRect, writingMode);
        visualRect.MoveBy(lineBoxVisualOffset);
        return visualRect;
      }();

      auto createDisplayBoxForRun = [&]
      {
        if (lineRun.IsText())
        {
          AppendTextDisplayBox(lineRun, visualRectRelativeToRoot, boxes);
        }
        else if (lineRun.IsSoftLineBreak())
        {
          AppendSoftLineBreakDisplayBox(lineRun, visualRectRelativeToRoot, boxes);
        }
        else if (lineRun.IsHardLineBreak())
        {
          AppendHardLineBreakDisplayBox(lineRun, visualRectRelativeToRoot, boxes);
        }
        else if (lineRun.IsAtomicInlineBox() || lineRun.IsListMarker())
        {
          AppendAtomicInlineLevelDisplayBox(lineRun, visualRectRelativeToRoot, boxes);
        }
        else if (lineRun.IsInlineBoxStart() || lineRun.IsLineSpanningInlineBoxStart())
        {
          AppendInlineBoxDisplayBox(lineRun, lineBox.InlineLevelBoxFor(lineRun), visualRectRelativeToRoot,
                                    boxes);
        }
        else if (lineRun.IsBlock())
        {
          // Block content should always be placed at the start of the content box even when floats shrink the
          // line.
          auto adjustedVisualRect = [&]
          {
            auto lineOffset = lineBoxLogicalRect.Left() - lineLayoutResult.Geometry.InitialLogicalLeft;
            auto rect = visualRectRelativeToRoot;
            if (writingMode.IsHorizontal())
            {
              rect.MoveHorizontally(-lineOffset);
            }
            else
            {
              rect.MoveVertically(-lineOffset);
            }
            return rect;
          };
          AppendBlockLevelDisplayBox(lineRun, adjustedVisualRect(), boxes);
        }
        else
        {
          krys_debug_assert(false);
        }
      };
      createDisplayBoxForRun();

      auto updateAssociatedBoxGeometry = [&]
      {
        if (lineRun.IsText() || lineRun.IsSoftLineBreak())
        {
          return;
        }
        if (!lineBox.HasContent() && lineRun.IsLineSpanningInlineBoxStart())
        {
          // When a spanning inline box (e.g. <div>text<span><br></span></div>) lands on an empty line
          // (empty here means no content at all including line breaks, not just visually empty) then we
          // don't extend the spanning line box over to this line.
          return;
        }
        auto &boxGeometry = GetFormattingContext().GeometryForBox(layoutBox);
        auto borderBoxLogicalTopLeft =
          LayoutPoint {lineBox.LogicalRect().TopLeft() + FloatSize {logicalRect.Left(), logicalRect.Top()}};
        if (lineRun.IsInlineBoxStart() || lineRun.IsLineSpanningInlineBoxStart())
        {
          // Inline boxes need special (stretchy) box geometry handling.
          SetInlineBoxGeometry(lineRun.LayoutBox(), boxGeometry,
                               {FloatPoint {borderBoxLogicalTopLeft}, logicalRect.Size()},
                               lineBox.InlineLevelBoxFor(lineRun).IsFirstBox());
          return;
        }
        boxGeometry.SetTopLeft(borderBoxLogicalTopLeft);
        boxGeometry.SetContentBoxSize(
          {LayoutUnit {logicalRect.Width()} - boxGeometry.HorizontalBorderAndPadding(),
           LayoutUnit {logicalRect.Height()} - boxGeometry.VerticalBorderAndPadding()});
      };
      updateAssociatedBoxGeometry();
    }
    SetGeometryForBlockLevelOutOfFlowBoxes(blockLevelOutOfFlowBoxList, lineLayoutResult.Runs);
  }

  size_t InlineDisplayContentBuilder::EnsureDisplayBoxForContainer(const ElementBox &elementBox,
                                                                   DisplayBoxTree &displayBoxTree,
                                                                   AncestorStack &ancestorStack,
                                                                   inline_display::Boxes &boxes) noexcept
  {
    krys_debug_assert(elementBox.IsInlineBox() || &elementBox == &Root());
    if (auto lowestCommonAncestorIndex = ancestorStack.Unwind(elementBox))
    {
      return *lowestCommonAncestorIndex;
    }
    auto enclosingDisplayBoxNodeIndexForContainer =
      EnsureDisplayBoxForContainer(elementBox.Parent(), displayBoxTree, ancestorStack, boxes);
    AppendInlineDisplayBoxAtBidiBoundary(elementBox, boxes);
    return CreateDisplayBoxNodeForContainerAndPushToAncestorStack(
      elementBox, boxes.size() - 1, enclosingDisplayBoxNodeIndexForContainer, displayBoxTree, ancestorStack);
  }

  void InlineDisplayContentBuilder::AdjustVisualGeometryForDisplayBox(
    size_t displayBoxNodeIndex, InlineLayoutUnit &contentLineRightEdge, InlineLayoutUnit lineBoxLogicalTop,
    const DisplayBoxTree &displayBoxTree, inline_display::Boxes &boxes,
    const IsFirstLastIndexesMap &isFirstLastIndexesMap) noexcept
  {
    auto rootWritingMode = Root().GetWritingMode();
    auto isHorizontalWritingMode = rootWritingMode.IsHorizontal();
    // Non-inline box display boxes just need a horizontal adjustment while
    // inline box type of display boxes need
    // 1. horizontal adjustment and margin/border/padding start offsetting on the first box
    // 2. right edge computation including descendant content width and margin/border/padding end offsetting
    // on the last box
    auto &displayBox = boxes[displayBoxTree.At(displayBoxNodeIndex).DisplayBoxIndex];
    auto &layoutBox = displayBox.LayoutBox();

    if (!displayBox.IsNonRootInlineBox())
    {
      auto lineLogicalLeft = rootWritingMode.IsHorizontal() ? _displayLine.Left() : _displayLine.Top();

      if (displayBox.IsAtomicInlineBox() || displayBox.IsGenericInlineLevelBox())
      {
        auto &boxGeometry = GetFormattingContext().GeometryForBox(layoutBox);
        auto boxMarginLineLeft = MarginLineLeft(boxGeometry, layoutBox.Parent().GetWritingMode()).ToFloat();

        auto borderBoxLeft = InlineLayoutUnit {lineLogicalLeft + contentLineRightEdge + boxMarginLineLeft};
        boxGeometry.SetLeft(LayoutUnit {borderBoxLeft});
        auto logicalLeft =
          rootWritingMode.IsLogicalLeftLineLeft()
            ? borderBoxLeft
            : GetLineBox().LogicalRect().Width() - (borderBoxLeft + boxGeometry.BorderBoxWidth().ToFloat());
        SetLogicalLeft(displayBox, logicalLeft, rootWritingMode);

        contentLineRightEdge += boxGeometry.MarginBoxWidth().ToFloat();
      }
      else
      {
        auto wordSpacingMargin =
          displayBox.IsWordSeparator() ? layoutBox.Style().GetFontCascade().WordSpacing() : 0.0f;
        auto logicalLeft = contentLineRightEdge + wordSpacingMargin;
        auto logicalWidth = isHorizontalWritingMode ? displayBox.Width() : displayBox.Height();
        if (!rootWritingMode.IsLogicalLeftLineLeft())
        {
          logicalLeft = GetLineBox().LogicalRect().Width() - (logicalLeft + logicalWidth);
        }
        SetLogicalLeft(displayBox, lineLogicalLeft + logicalLeft, rootWritingMode);
        contentLineRightEdge += logicalWidth + wordSpacingMargin;
      }
      return;
    }

    auto &boxGeometry = GetFormattingContext().GeometryForBox(layoutBox);
    auto boxWritingMode = layoutBox.GetWritingMode();
    auto boxIsLTR = boxWritingMode.IsBidiLTR();
    auto isFirstLastIndexes = IsFirstLastIndex {};
    if (auto entry = isFirstLastIndexesMap.find(&layoutBox); entry != isFirstLastIndexesMap.end())
    {
      isFirstLastIndexes = entry->second;
    }
    auto isFirstBox = isFirstLastIndexes.First && *isFirstLastIndexes.First == displayBoxNodeIndex;
    auto isLastBox = isFirstLastIndexes.Last && *isFirstLastIndexes.Last == displayBoxNodeIndex;
    auto logicalRect = GetLineBox().LogicalBorderBoxForInlineBox(layoutBox, boxGeometry);
    auto beforeInlineBoxContent = [&]
    {
      auto shouldApplyLeftSide = (boxIsLTR && isFirstBox) || (!boxIsLTR && isLastBox);
      if (shouldApplyLeftSide)
      {
        contentLineRightEdge += MarginLineLeft(boxGeometry, boxWritingMode).ToFloat();
      }
      logicalRect.SetLeft(contentLineRightEdge);
      if (shouldApplyLeftSide)
      {
        contentLineRightEdge +=
          (BorderLineLeft(boxGeometry, boxWritingMode) + PaddingLineLeft(boxGeometry, boxWritingMode))
            .ToFloat();
      }
    };
    beforeInlineBoxContent();

    for (auto childDisplayBoxNodeIndex : displayBoxTree.At(displayBoxNodeIndex).Children)
    {
      AdjustVisualGeometryForDisplayBox(childDisplayBoxNodeIndex, contentLineRightEdge, lineBoxLogicalTop,
                                        displayBoxTree, boxes, isFirstLastIndexesMap);
    }

    auto afterInlineBoxContent = [&]
    {
      auto shouldApplyRightSide = (boxIsLTR && isLastBox) || (!boxIsLTR && isFirstBox);
      if (shouldApplyRightSide)
      {
        contentLineRightEdge +=
          (BorderLineRight(boxGeometry, boxWritingMode) + PaddingLineRight(boxGeometry, boxWritingMode))
            .ToFloat();
        // FIXME: Add the ruby base's end additional logical width for ruby bases.
        logicalRect.SetRight(contentLineRightEdge);
        contentLineRightEdge += MarginLineRight(boxGeometry, boxWritingMode).ToFloat();
      }
      else
      {
        logicalRect.SetRight(contentLineRightEdge);
      }
    };
    afterInlineBoxContent();

    auto visualRect = [&]
    {
      auto visualRect =
        MapInlineRectLogicalToVisual(logicalRect, GetLineBox().LogicalRect(), rootWritingMode);
      visualRect.MoveBy(_displayLine.TopLeft());
      return visualRect;
    }();
    displayBox.SetRect(visualRect, visualRect);

    auto *inlineBox = GetLineBox().InlineLevelBoxFor(layoutBox);
    krys_debug_assert(inlineBox);
    auto computeInkOverflow = [&]
    {
      auto inkOverflow = FloatRect {displayBox.VisualRectIgnoringBlockDirection()};
      _contentHasInkOverflow =
        ComputeInkOverflowForInlineBox(
          *inlineBox, IsFirstFormattedLine() ? layoutBox.FirstLineStyle() : layoutBox.Style(), inkOverflow)
        || _contentHasInkOverflow;
      displayBox.AdjustInkOverflow(inkOverflow);
    };
    computeInkOverflow();

    auto inlineBoxLeftInInlineDirectionVisualOrder =
      isHorizontalWritingMode ? displayBox.Left() : displayBox.Top();
    auto logicalWidthForBiDiFragment = isHorizontalWritingMode ? displayBox.Width() : displayBox.Height();
    auto logicalTopRelativeToRoot = GetLineBox().LogicalRect().Top() + logicalRect.Top();
    SetInlineBoxGeometry(layoutBox, boxGeometry,
                         {logicalTopRelativeToRoot, inlineBoxLeftInInlineDirectionVisualOrder,
                          logicalWidthForBiDiFragment, logicalRect.Height()},
                         isFirstBox);
    if (inlineBox->HasContent())
    {
      displayBox.SetHasContent();
    }
  }

  void InlineDisplayContentBuilder::ProcessBidiContent(const LineLayoutResult &lineLayoutResult,
                                                       inline_display::Boxes &boxes) noexcept
  {
    krys_debug_assert(lineLayoutResult.BidiDirectionality.VisualOrderList.size()
                      <= lineLayoutResult.Runs.size());

    AncestorStack ancestorStack;
    auto displayBoxTree = DisplayBoxTree {};
    ancestorStack.Push({}, Root());

    auto &lineBox = GetLineBox();
    auto writingMode = Root().GetWritingMode();
    auto isHorizontalWritingMode = writingMode.IsHorizontal();

    auto lineLogicalTop = isHorizontalWritingMode ? _displayLine.Top() : _displayLine.Left();
    auto lineLogicalLeft = isHorizontalWritingMode ? _displayLine.Left() : _displayLine.Top();
    // Note that hangable punctuation does not contribute to ContentLogicalLeftIgnoringInlineDirection() as it
    // is considered more of an overflow.
    auto contentLineLeftEdge =
      _displayLine.ContentLogicalLeftIgnoringInlineDirection()
      - (writingMode.IsBidiLTR() ? lineLayoutResult.Hanging.HangablePunctuationStartWidth : 0.f);
    auto hasInlineBox = false;
    auto createDisplayBoxesInVisualOrder = [&]
    {
      auto lineBoxLogicalRect = lineBox.LogicalRect();
      auto lineBoxVisualOffset = _displayLine.TopLeft();
      auto rootLogicalRect = lineBox.LogicalRectForRootInlineBox();
      rootLogicalRect.SetLeft(_displayLine.ContentLogicalLeftIgnoringInlineDirection());
      auto rootVisualRect = MapInlineRectLogicalToVisual(rootLogicalRect, lineBoxLogicalRect, writingMode);
      rootVisualRect.MoveBy(lineBoxVisualOffset);
      AppendRootInlineBoxDisplayBox(rootVisualRect, lineBox.RootInlineBox().HasContent(), boxes);

      List<size_t> blockLevelOutOfFlowBoxList;
      auto contentLineRightEdge = contentLineLeftEdge;
      auto &inlineContent = lineLayoutResult.Runs;
      auto &visualOrderList = lineLayoutResult.BidiDirectionality.VisualOrderList;
      for (size_t index = 0; index < visualOrderList.size(); ++index)
      {
        auto logicalIndex = static_cast<size_t>(visualOrderList[index]);
        krys_debug_assert(inlineContent[logicalIndex].BidiLevel() != InlineItem::OpaqueBidiLevel
                          || inlineContent[logicalIndex].IsLineSpanningInlineBoxStart());

        auto &lineRun = inlineContent[logicalIndex];
        auto needsDisplayBoxOrGeometrySetting =
          !lineRun.IsWordBreakOpportunity() && !lineRun.IsInlineBoxEnd();
        if (!needsDisplayBoxOrGeometrySetting)
        {
          continue;
        }

        auto &layoutBox = lineRun.LayoutBox();
        auto parentDisplayBoxNodeIndex =
          EnsureDisplayBoxForContainer(layoutBox.Parent(), displayBoxTree, ancestorStack, boxes);
        hasInlineBox = hasInlineBox
                       || (!lineRun.IsBlock()
                           && (parentDisplayBoxNodeIndex || lineRun.IsInlineBoxStart()
                               || lineRun.IsLineSpanningInlineBoxStart()));

        if (lineRun.IsOpaque())
        {
          if (layoutBox.Style().IsOriginalDisplayInlineType())
          {
            // Note that out-of-flow handling (render tree integraton) really only needs logical coords (not
            // even "content in inline diretion visual order").
            GetFormattingContext().GeometryForBox(layoutBox).SetTopLeft(LayoutPoint {
              LayoutUnit {lineBox.LogicalRect().Left() + lineBox.LogicalRectForRootInlineBox().Left()
                          + lineRun.LogicalLeft()},
              LayoutUnit {lineBox.LogicalRect().Top()}});
            continue;
          }
          blockLevelOutOfFlowBoxList.push_back(index);
          continue;
        }

        auto logicalRect = [&]() -> InlineRect
        {
          if (lineRun.IsText() || lineRun.IsSoftLineBreak())
          {
            return lineBox.LogicalRectForTextRun(lineRun);
          }
          if (lineRun.IsHardLineBreak())
          {
            return lineBox.LogicalRectForLineBreakBox(layoutBox);
          }
          if (lineRun.IsInlineBoxStart() || lineRun.IsLineSpanningInlineBoxStart())
          {
            // Computed at a later stage.
            return {InlineLayoutPoint {}, InlineLayoutSize {}};
          }
          auto &boxGeometry = GetFormattingContext().GeometryForBox(layoutBox);
          if (lineRun.IsAtomicInlineBox() || lineRun.IsListMarker())
          {
            return lineBox.LogicalBorderBoxForAtomicInlineBox(layoutBox, boxGeometry);
          }
          if (lineRun.IsBlock())
          {
            auto borderBoxRect = BoxGeometry::BorderBoxRect(boxGeometry);
            return {borderBoxRect.Top().ToFloat(), borderBoxRect.Left().ToFloat(),
                    borderBoxRect.Width().ToFloat(), borderBoxRect.Height().ToFloat()};
          }
          krys_debug_assert(false);
          return {};
        }();
        logicalRect.SetLeft(contentLineRightEdge);

        auto visualRectRelativeToRoot = [&]
        {
          auto visualRect = MapInlineRectLogicalToVisual(logicalRect, lineBoxLogicalRect, writingMode);
          visualRect.MoveBy(lineBoxVisualOffset);
          return visualRect;
        }();
        auto moveInInlineDirection = [&](InlineLayoutUnit offset)
        {
          if (isHorizontalWritingMode)
          {
            visualRectRelativeToRoot.MoveHorizontally(offset);
          }
          else
          {
            visualRectRelativeToRoot.MoveVertically(offset);
          }
        };

        if (lineRun.IsText())
        {
          auto wordSpacingMargin =
            lineRun.IsWordSeparator() ? layoutBox.Style().GetFontCascade().WordSpacing() : 0.0f;
          moveInInlineDirection(wordSpacingMargin);
          AppendTextDisplayBox(lineRun, visualRectRelativeToRoot, boxes);
          contentLineRightEdge += logicalRect.Width() + wordSpacingMargin;
          displayBoxTree.Append(parentDisplayBoxNodeIndex, boxes.size() - 1);
          continue;
        }
        if (lineRun.IsSoftLineBreak())
        {
          krys_debug_assert((isHorizontalWritingMode && !visualRectRelativeToRoot.Width())
                            || (!isHorizontalWritingMode && !visualRectRelativeToRoot.Height()));
          AppendSoftLineBreakDisplayBox(lineRun, visualRectRelativeToRoot, boxes);
          displayBoxTree.Append(parentDisplayBoxNodeIndex, boxes.size() - 1);
          continue;
        }
        if (lineRun.IsHardLineBreak())
        {
          krys_debug_assert((isHorizontalWritingMode && !visualRectRelativeToRoot.Width())
                            || (!isHorizontalWritingMode && !visualRectRelativeToRoot.Height()));
          AppendHardLineBreakDisplayBox(lineRun, visualRectRelativeToRoot, boxes);

          auto &boxGeometry = GetFormattingContext().GeometryForBox(layoutBox);
          boxGeometry.SetTopLeft(LayoutPoint {LayoutUnit {lineLogicalLeft + contentLineRightEdge},
                                              LayoutUnit {lineLogicalTop + logicalRect.Top()}});
          boxGeometry.SetContentBoxSize(LayoutSize {logicalRect.Size()});

          displayBoxTree.Append(parentDisplayBoxNodeIndex, boxes.size() - 1);
          continue;
        }
        if (lineRun.IsAtomicInlineBox() || lineRun.IsListMarker())
        {
          auto parentWritingMode = layoutBox.Parent().GetWritingMode();
          auto &boxGeometry = GetFormattingContext().GeometryForBox(layoutBox);
          auto boxMarginLeft = MarginLineLeft(boxGeometry, parentWritingMode).ToFloat();
          moveInInlineDirection(boxMarginLeft);

          AppendAtomicInlineLevelDisplayBox(lineRun, visualRectRelativeToRoot, boxes);
          boxGeometry.SetTopLeft(LayoutPoint {LayoutUnit {lineLogicalLeft + contentLineRightEdge},
                                              LayoutUnit {lineLogicalTop + logicalRect.Top()}});

          contentLineRightEdge +=
            boxMarginLeft + logicalRect.Width() + MarginLineRight(boxGeometry, parentWritingMode).ToFloat();
          displayBoxTree.Append(parentDisplayBoxNodeIndex, boxes.size() - 1);
          continue;
        }
        if (lineRun.IsInlineBoxStart() || lineRun.IsLineSpanningInlineBoxStart())
        {
          // FIXME: While we should only get here with empty inline boxes, there are
          // some cases where the inline box has some content on the paragraph level (at bidi split) but line
          // breaking renders it empty or their content is completely collapsed. Such inline boxes should also
          // be handled here.
          if (!lineBox.HasContent())
          {
            // FIXME: It's expected to not have any inline boxes on empty lines. They make the line taller. We
            // should reconsider this.
            SetInlineBoxGeometry(layoutBox, GetFormattingContext().GeometryForBox(layoutBox),
                                 {InlineLayoutPoint {}, InlineLayoutSize {}}, true);
            continue;
          }
          auto isEmptyInlineBox = [&]
          {
            // FIXME: Maybe we should not tag ruby bases with annotation boxes only contentful?
            if (!lineBox.InlineLevelBoxFor(lineRun).HasContent())
            {
              return true;
            }
            if (!layoutBox.IsRubyBase())
            {
              return false;
            }
            auto *rubyBaseLayoutBox = DynamicDowncast<ElementBox>(layoutBox);
            if (!rubyBaseLayoutBox)
            {
              return false;
            }
            // Let's create empty inline boxes for ruby bases with annotation only.
            if (!rubyBaseLayoutBox->FirstChild()
                || (rubyBaseLayoutBox->FirstChild() == rubyBaseLayoutBox->LastChild()
                    && rubyBaseLayoutBox->FirstChild()->IsRubyAnnotationBox()))
            {
              return true;
            }
            // Let's check if we actually don't have a contentful run inside this ruby base.
            for (size_t nextLogicalRunIndex = logicalIndex + 1; nextLogicalRunIndex < inlineContent.size();
                 ++nextLogicalRunIndex)
            {
              auto &nextLineRun = inlineContent[nextLogicalRunIndex];
              if (nextLineRun.IsInlineBoxEnd() && &nextLineRun.LayoutBox() == rubyBaseLayoutBox)
              {
                break;
              }
              if (nextLineRun.IsContentful())
              {
                return false;
              }
            }
            return true;
          };
          if (isEmptyInlineBox())
          {
            AppendInlineDisplayBoxAtBidiBoundary(layoutBox, boxes);
            CreateDisplayBoxNodeForContainerAndPushToAncestorStack(
              Downcast<ElementBox>(layoutBox), boxes.size() - 1, parentDisplayBoxNodeIndex, displayBoxTree,
              ancestorStack);
          }
          continue;
        }
        if (lineRun.IsBlock())
        {
          krys_debug_assert(!hasInlineBox);

          auto &boxGeometry = GetFormattingContext().GeometryForBox(layoutBox);
          auto boxMarginLeft = MarginLineLeft(boxGeometry, writingMode).ToFloat();
          // Block content should always be placed at the start of the content box even when floats shrink the
          // line.
          auto lineOffset = lineBoxLogicalRect.Left() - lineLayoutResult.Geometry.InitialLogicalLeft;
          moveInInlineDirection(lineOffset + boxMarginLeft);

          AppendBlockLevelDisplayBox(lineRun, visualRectRelativeToRoot, boxes);
          boxGeometry.SetTopLeft(
            LayoutPoint {LayoutUnit {lineLogicalLeft + contentLineRightEdge - lineOffset},
                         LayoutUnit {lineLogicalTop + logicalRect.Top()}});
          continue;
        }
        krys_debug_assert(false);
      }
      SetGeometryForBlockLevelOutOfFlowBoxes(blockLevelOutOfFlowBoxList, inlineContent, visualOrderList);
    };
    createDisplayBoxesInVisualOrder();

    auto handleInlineBoxes = [&]
    {
      if (!hasInlineBox)
      {
        return;
      }

      IsFirstLastIndexesMap isFirstLastIndexesMap;
      auto computeIsFirstIsLastBox = [&]
      {
        krys_debug_assert(boxes[0].IsRootInlineBox());
        for (size_t index = 1; index < boxes.size(); ++index)
        {
          if (!boxes[index].IsNonRootInlineBox())
          {
            continue;
          }
          auto &layoutBox = boxes[index].LayoutBox();
          auto *inlineLevelBox = lineBox.InlineLevelBoxFor(layoutBox);
          krys_debug_assert(inlineLevelBox);
          auto isFirstBox = inlineLevelBox->IsFirstBox();
          auto isLastBox = inlineLevelBox->IsLastBox();
          if (!isFirstBox && !isLastBox)
          {
            continue;
          }
          if (isFirstBox)
          {
            auto isFirstLastIndexes = IsFirstLastIndex {};
            if (auto entry = isFirstLastIndexesMap.find(&layoutBox); entry != isFirstLastIndexesMap.end())
            {
              isFirstLastIndexes = entry->second;
            }
            if (!isFirstLastIndexes.First || isLastBox)
            {
              isFirstLastIndexesMap.insert_or_assign(
                &layoutBox, IsFirstLastIndex {isFirstLastIndexes.First.value_or(index),
                                              isLastBox ? Maybe<size_t> {index} : isFirstLastIndexes.Last});
            }
            continue;
          }
          if (isLastBox)
          {
            krys_debug_assert(!isFirstBox);
            isFirstLastIndexesMap.insert_or_assign(&layoutBox, IsFirstLastIndex {null, index});
            continue;
          }
        }
      };
      computeIsFirstIsLastBox();

      auto adjustVisualGeometryWithInlineBoxes = [&]
      {
        auto contentLineRightEdge = contentLineLeftEdge;

        for (auto childDisplayBoxNodeIndex : displayBoxTree.Root().Children)
        {
          AdjustVisualGeometryForDisplayBox(childDisplayBoxNodeIndex, contentLineRightEdge, lineLogicalTop,
                                            displayBoxTree, boxes, isFirstLastIndexesMap);
        }
      };
      adjustVisualGeometryWithInlineBoxes();
    };
    handleInlineBoxes();

    auto handleTrailingOpenInlineBoxes = [&]
    {
      for (auto &lineRun : lineLayoutResult.Runs | std::views::reverse)
      {
        if (!lineRun.IsInlineBoxStart() || lineRun.BidiLevel() != InlineItem::OpaqueBidiLevel)
        {
          break;
        }
        // These are trailing inline box start runs (without the closing inline box end <span> <-line breaks
        // here</span>). They don't participate in visual reordering (see createDisplayBoxesInVisualOrder
        // above) as we don't find them empty at inline item list building time due to trailing whitespace.
        // Non-empty inline boxes are normally get their display boxes generated when we process their content
        // runs, but these trailing runs have their content on the subsequent line(s).
        auto &inlineBox = lineBox.InlineLevelBoxFor(lineRun);
        AppendInlineBoxDisplayBox(lineRun, inlineBox, {0.f, _displayLine.Right(), 0.f, 0.f}, boxes);
        SetInlineBoxGeometry(lineRun.LayoutBox(), GetFormattingContext().GeometryForBox(lineRun.LayoutBox()),
                             {0.f, lineBox.LogicalRect().Right(), 0.f, 0.f}, inlineBox.IsFirstBox());
      }
    };
    handleTrailingOpenInlineBoxes();
  }

  void InlineDisplayContentBuilder::CollectInkOverflowForInlineBoxes(inline_display::Boxes &boxes) noexcept
  {
    if (!_contentHasInkOverflow && !_hasSeenNestedInlineBoxesWithDifferentFontCascade)
    {
      return;
    }
    // Visit the inline boxes and propagate ink overflow to their parents -except to the root inline box.
    // (e.g. <span style="font-size: 10px;">Small font size<span style="font-size: 300px;">Larger font size.
    // This overflows the top most span.</span></span>).
    auto accumulatedInkOverflowRect = InlineRect {InlineLayoutPoint {}, InlineLayoutSize {}};
    for (size_t index = boxes.size(); index--;)
    {
      auto &displayBox = boxes[index];

      auto mayHaveInkOverflow = displayBox.IsText() || displayBox.IsAtomicInlineBox()
                                || displayBox.IsGenericInlineLevelBox() || displayBox.IsNonRootInlineBox();
      if (!mayHaveInkOverflow)
      {
        continue;
      }
      if (displayBox.IsNonRootInlineBox() && !accumulatedInkOverflowRect.IsEmpty())
      {
        displayBox.AdjustInkOverflow(accumulatedInkOverflowRect);
      }

      // We stop collecting ink overflow for at root inline box (i.e. don't inflate the root inline box with
      // the inline content here).
      auto parentBoxIsRoot = &displayBox.LayoutBox().Parent() == &Root();
      if (parentBoxIsRoot)
      {
        accumulatedInkOverflowRect = InlineRect {InlineLayoutPoint {}, InlineLayoutSize {}};
      }
      else if (accumulatedInkOverflowRect.IsEmpty())
      {
        accumulatedInkOverflowRect = displayBox.InkOverflow();
      }
      else
      {
        accumulatedInkOverflowRect.ExpandToContain(displayBox.InkOverflow());
      }
    }
  }

  void InlineDisplayContentBuilder::SetGeometryForBlockLevelOutOfFlowBoxes(
    const List<size_t> &indexListOfOutOfFlowBoxes, const Line::RunList &lineRuns,
    const List<int32> &visualOrderList) noexcept
  {
    if (indexListOfOutOfFlowBoxes.empty())
    {
      return;
    }

    // Block level boxes are placed either at the start of the line or "under" depending whether they have
    // previous inflow sibling. Here we figure out if a particular out of flow box has an inflow sibling or
    // not.
    // 1. Find the first inflow content. Any out of flow box after this gets moved _under_ the line box.
    // 2. Loop through the out of flow boxes (indexListOfOutOfFlowBoxes) and set their vertical geometry
    // depending whether they are before or after the first inflow content. Note that there's an extra layer
    // of directionality here: in case of right to left inline direction, the before inflow content check
    // starts from the right edge and progresses in a leftward manner and not in visual order. However this is
    // not logical order either (which is more about bidi than inline direction). So LTR starts at the left
    // while RTL starts at the right and in both cases jumping from run to run in bidi order.
    auto &formattingContext = GetFormattingContext();
    auto isBidiLTR = Root().GetWritingMode().IsBidiLTR();

    auto firstContentfulInFlowRunIndex = Maybe<size_t> {};
    auto contentListSize = visualOrderList.empty() ? lineRuns.size() : visualOrderList.size();
    for (size_t i = 0; i < contentListSize; ++i)
    {
      auto index = RunIndex(i, contentListSize, isBidiLTR);
      auto &lineRun =
        visualOrderList.empty() ? lineRuns[index] : lineRuns[static_cast<size_t>(visualOrderList[index])];
      if (lineRun.LayoutBox().IsInFlow()
          && Line::Run::IsContentfulOrHasDecoration(lineRun, formattingContext))
      {
        firstContentfulInFlowRunIndex = index;
        break;
      }
    }

    if (!firstContentfulInFlowRunIndex)
    {
      SetGeometryForOutOfFlowBoxes(indexListOfOutOfFlowBoxes, null, lineRuns, visualOrderList,
                                   formattingContext, GetLineBox(), GetConstraints());
      return;
    }

    auto firstOutOfFlowIndexWithPreviousInflowSibling = Maybe<size_t> {};
    auto outOfFlowBoxListSize = indexListOfOutOfFlowBoxes.size();
    for (size_t i = 0; i < outOfFlowBoxListSize; ++i)
    {
      auto outOfFlowIndex = RunIndex(i, outOfFlowBoxListSize, isBidiLTR);
      auto hasPreviousInflowSibling =
        (isBidiLTR && indexListOfOutOfFlowBoxes[outOfFlowIndex] > *firstContentfulInFlowRunIndex)
        || (!isBidiLTR && indexListOfOutOfFlowBoxes[outOfFlowIndex] < *firstContentfulInFlowRunIndex);
      if (hasPreviousInflowSibling)
      {
        firstOutOfFlowIndexWithPreviousInflowSibling = outOfFlowIndex;
        break;
      }
    }
    SetGeometryForOutOfFlowBoxes(indexListOfOutOfFlowBoxes, firstOutOfFlowIndexWithPreviousInflowSibling,
                                 lineRuns, visualOrderList, formattingContext, GetLineBox(),
                                 GetConstraints());
  }

  void
    InlineDisplayContentBuilder::CollectInkOverflowForTextDecorations(inline_display::Boxes &boxes) noexcept
  {
    if (!_hasSeenTextDecoration)
    {
      return;
    }

    auto logicalBottomForTextDecoration = Maybe<float> {};
    auto writingMode = Root().GetWritingMode();
    auto isHorizontalWritingMode = writingMode.IsHorizontal();

    for (auto &displayBox : boxes)
    {
      if (!displayBox.IsText())
      {
        continue;
      }

      auto &parentStyle = displayBox.LayoutBox().Parent().Style();
      auto textDecorations = parentStyle.TextDecorationLineInEffect();
      if (textDecorations.IsNone())
      {
        continue;
      }

      auto decorationOverflow = [&]
      {
        if (!textDecorations.HasUnderline())
        {
          return InkOverflowForDecorations(parentStyle);
        }

        if (!logicalBottomForTextDecoration)
        {
          logicalBottomForTextDecoration =
            LogicalBottomForTextDecorationContent(boxes, isHorizontalWritingMode);
        }
        auto textRunLogicalOffsetFromLineBottom =
          *logicalBottomForTextDecoration
          - (isHorizontalWritingMode ? displayBox.Bottom() : displayBox.Right());
        return InkOverflowForDecorations(parentStyle,
                                         {displayBox.Height(), textRunLogicalOffsetFromLineBottom});
      }();

      if (!decorationOverflow.IsEmpty())
      {
        _contentHasInkOverflow = true;
        auto inflatedInkOverflowRect = [&]
        {
          auto inkOverflowRect = displayBox.InkOverflow();
          auto left = decorationOverflow.Left.ToFloat();
          auto top = decorationOverflow.Top.ToFloat();
          auto right = decorationOverflow.Right.ToFloat();
          auto bottom = decorationOverflow.Bottom.ToFloat();
          switch (writingMode.BlockDirection())
          {
            case FlowDirection::TopToBottom:
            case FlowDirection::BottomToTop: inkOverflowRect.Inflate(left, top, right, bottom); break;
            case FlowDirection::LeftToRight: inkOverflowRect.Inflate(bottom, right, top, left); break;
            case FlowDirection::RightToLeft: inkOverflowRect.Inflate(top, right, bottom, left); break;
            default:                         krys_debug_assert(false); break;
          }
          return inkOverflowRect;
        };
        displayBox.AdjustInkOverflow(inflatedInkOverflowRect());
      }
    }
  }

  InlineRect InlineDisplayContentBuilder::MapInlineRectLogicalToVisual(const InlineRect &logicalRect,
                                                                       const InlineRect &containerLogicalRect,
                                                                       WritingMode writingMode) noexcept
  {
    InlineRect visualRect = logicalRect;
    switch (writingMode.ComputedWritingMode())
    {
      case StyleWritingMode::HorizontalTb: return visualRect;
      case StyleWritingMode::HorizontalBt:
        visualRect.SetTop(containerLogicalRect.Height() - logicalRect.Bottom());
        return visualRect;
      case StyleWritingMode::VerticalRl:
      case StyleWritingMode::SidewaysRl:
        visualRect.SetLeft(logicalRect.Top());
        visualRect.SetTop(logicalRect.Left());
        visualRect.SetWidth(logicalRect.Height());
        visualRect.SetHeight(logicalRect.Width());
        return visualRect;
      case StyleWritingMode::VerticalLr:
        visualRect.SetLeft(containerLogicalRect.Height() - logicalRect.Bottom());
        visualRect.SetTop(logicalRect.Left());
        visualRect.SetWidth(logicalRect.Height());
        visualRect.SetHeight(logicalRect.Width());
        return visualRect;
      case StyleWritingMode::SidewaysLr:
        visualRect.SetLeft(logicalRect.Top());
        visualRect.SetTop(containerLogicalRect.Width() - logicalRect.Right());
        visualRect.SetWidth(logicalRect.Height());
        visualRect.SetHeight(logicalRect.Width());
        return visualRect;
      default: krys_debug_assert(false); return visualRect;
    }
  }

  void InlineDisplayContentBuilder::SetInlineBoxGeometry(const Box &inlineBox, BoxGeometry &boxGeometry,
                                                         const InlineRect &logicalRect,
                                                         bool isFirstInlineBoxFragment) noexcept
  {
    krys_debug_assert(inlineBox.IsInlineBox());

    auto borderBoxSize = LayoutSize {};
    if (!inlineBox.IsRubyBase())
    {
      borderBoxSize = {LayoutUnit::FromFloatCeil(logicalRect.Width()),
                       LayoutUnit::FromFloatCeil(logicalRect.Height())};
    }
    else
    {
      // While flooring here may produce fractional overflow, it ensures ruby content would never trigger
      // incorrect wrapping triggered by inflated annotation boxes inside a shrink-to-fit container (where
      // annotation box is sized to the ruby base but it also imposes a required minimum width)
      borderBoxSize = {LayoutUnit::FromFloatFloor(logicalRect.Width()),
                       LayoutUnit::FromFloatFloor(logicalRect.Height())};
    }

    auto borderBoxRect = Rect {LayoutPoint {logicalRect.TopLeft()}, borderBoxSize};
    if (!isFirstInlineBoxFragment)
    {
      borderBoxRect.ExpandToContain(BoxGeometry::BorderBoxRect(boxGeometry));
    }

    boxGeometry.SetTopLeft(borderBoxRect.TopLeft());
    boxGeometry.SetContentBoxWidth(borderBoxRect.Width() - boxGeometry.HorizontalBorderAndPadding());
    boxGeometry.SetContentBoxHeight(borderBoxRect.Height() - boxGeometry.VerticalBorderAndPadding());
  }

  void InlineDisplayContentBuilder::SetLogicalLeft(inline_display::Box &box, InlineLayoutUnit logicalLeft,
                                                   WritingMode writingMode) const noexcept
  {
    switch (writingMode.BlockDirection())
    {
      case FlowDirection::TopToBottom:
      case FlowDirection::BottomToTop: box.SetLeft(logicalLeft); break;
      case FlowDirection::LeftToRight:
      case FlowDirection::RightToLeft: box.SetTop(logicalLeft); break;
      default:                         krys_debug_assert(false); break;
    }
  }
}
