#pragma once

#include "Krystal.Booey/Graphics/FontBaseline.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLevelBox.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLine.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineRect.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::layout
{
  class BoxGeometry;
  class LineBoxBuilder;
  class LineBoxVerticalAligner;
  class RubyFormattingContext;

  // ____________________________________________________________ Line Box
  // |                                    --------------------
  // |                                   |                    |
  // | ----------------------------------|--------------------|---------- Root Inline Box
  // ||   _____    ___      ___          |                    |
  // ||  |        /   \    /   \         |  Inline Level Box  |
  // ||  |_____  |     |  |     |        |                    |    ascent
  // ||  |       |     |  |     |        |                    |
  // ||__|________\___/____\___/_________|____________________|_______ alignment_baseline
  // ||
  // ||                                                      descent
  // ||_______________________________________________________________
  // |________________________________________________________________
  // The resulting rectangular area that contains the boxes that form a single line of inline-level content is
  // called a line box. https://www.w3.org/TR/css-inline-3/#model
  class LineBox
  {
    friend class LineBoxBuilder;
    friend class LineBoxVerticalAligner;
    friend class RubyFormattingContext;

  public:
    using InlineLevelBoxList = List<InlineLevelBox>;

  private:
    size_t _lineIndex {0};
    bool _hasContent {false};
    bool _isFirstFormattedLine {true};
    InlineRect _logicalRect;
    EnumSet<InlineLevelBox::Type> _boxTypes;

    FontBaseline _baselineType {FontBaseline::Alphabetic};
    InlineLevelBox _rootInlineBox;
    InlineLevelBoxList _nonRootInlineLevelBoxList;

    HashMap<const Box *, size_t> _nonRootInlineLevelBoxMap;

  public:
    LineBox(const Box &rootLayoutBox, InlineLayoutUnit contentLogicalLeft,
            InlineLayoutUnit contentLogicalWidth, size_t lineIndex, bool isFirstFormattedLine,
            size_t nonSpanningInlineLevelBoxCount) noexcept;

    // Note that the line can have many inline boxes and be "empty" the same time e.g.
    // <div><span></span><span></span></div>
    KRYS_NODISCARD bool HasContent() const noexcept
    {
      return _hasContent;
    }
    KRYS_NODISCARD bool HasInlineBox() const noexcept
    {
      return _boxTypes.Contains(InlineLevelBox::Type::InlineBox);
    }
    KRYS_NODISCARD bool HasNonInlineBox() const noexcept
    {
      return _boxTypes.ContainsAny({InlineLevelBox::Type::AtomicInlineBox, InlineLevelBox::Type::LineBreakBox,
                                    InlineLevelBox::Type::GenericInlineLevelBox});
    }
    KRYS_NODISCARD bool HasAtomicInlineBox() const noexcept
    {
      return _boxTypes.Contains(InlineLevelBox::Type::AtomicInlineBox);
    }

    KRYS_NODISCARD InlineRect LogicalRectForTextRun(const Line::Run &run) const noexcept;
    KRYS_NODISCARD InlineRect LogicalRectForLineBreakBox(const Box &layoutBox) const noexcept;
    KRYS_NODISCARD InlineRect LogicalRectForRootInlineBox() const noexcept
    {
      return _rootInlineBox.LogicalRect();
    }
    KRYS_NODISCARD InlineRect
      LogicalBorderBoxForAtomicInlineBox(const Box &layoutBox, const BoxGeometry &boxGeometry) const noexcept;
    KRYS_NODISCARD InlineRect LogicalBorderBoxForInlineBox(const Box &layoutBox,
                                                           const BoxGeometry &boxGeometry) const noexcept;
    KRYS_NODISCARD InlineRect LogicalContentBoxForInlineBox(const Box &layoutBox) const noexcept;

    KRYS_NODISCARD const InlineLevelBox *InlineLevelBoxFor(const Box &layoutBox) const noexcept
    {
      return const_cast<LineBox &>(*this).InlineLevelBoxFor(layoutBox);
    }
    KRYS_NODISCARD const InlineLevelBox &InlineLevelBoxFor(const Line::Run &lineRun) const noexcept
    {
      return const_cast<LineBox &>(*this).InlineLevelBoxFor(lineRun);
    }

    KRYS_NODISCARD const InlineLevelBox &RootInlineBox() const noexcept
    {
      return _rootInlineBox;
    }
    KRYS_NODISCARD const InlineLevelBoxList &NonRootInlineLevelBoxes() const noexcept
    {
      return _nonRootInlineLevelBoxList;
    }

    KRYS_NODISCARD FontBaseline BaselineType() const noexcept
    {
      return _baselineType;
    }

    KRYS_NODISCARD const InlineRect &LogicalRect() const noexcept
    {
      return _logicalRect;
    }

    KRYS_NODISCARD size_t LineIndex() const noexcept
    {
      return _lineIndex;
    }

    KRYS_NODISCARD bool IsFirstFormattedLine() const noexcept
    {
      return _isFirstFormattedLine;
    }

  private:
    void AddInlineLevelBox(InlineLevelBox &&inlineLevelBox) noexcept;
    KRYS_NODISCARD InlineLevelBoxList &NonRootInlineLevelBoxes() noexcept
    {
      return _nonRootInlineLevelBoxList;
    }

    KRYS_NODISCARD InlineLevelBox &RootInlineBox() noexcept
    {
      return _rootInlineBox;
    }

    KRYS_NODISCARD const InlineLevelBox &ParentInlineBox(const InlineLevelBox &inlineLevelBox) const noexcept
    {
      return const_cast<LineBox &>(*this).ParentInlineBox(inlineLevelBox);
    }
    KRYS_NODISCARD InlineLevelBox &ParentInlineBox(const InlineLevelBox &inlineLevelBox) noexcept
    {
      return *InlineLevelBoxFor(inlineLevelBox.LayoutBox().Parent());
    }

    KRYS_NODISCARD const InlineLevelBox &ParentInlineBox(const Line::Run &lineRun) const noexcept
    {
      return const_cast<LineBox &>(*this).ParentInlineBox(lineRun);
    }
    KRYS_NODISCARD InlineLevelBox &ParentInlineBox(const Line::Run &lineRun) noexcept
    {
      return *InlineLevelBoxFor(lineRun.LayoutBox().Parent());
    }

    KRYS_NODISCARD InlineLevelBox &InlineLevelBoxFor(const Line::Run &lineRun) noexcept
    {
      return *InlineLevelBoxFor(lineRun.LayoutBox());
    }
    KRYS_NODISCARD InlineLevelBox *InlineLevelBoxFor(const Box &layoutBox) noexcept
    {
      if (&layoutBox == &_rootInlineBox.LayoutBox())
      {
        return &_rootInlineBox;
      }
      auto entry = _nonRootInlineLevelBoxMap.find(&layoutBox);
      if (entry == _nonRootInlineLevelBoxMap.end())
      {
        return nullptr;
      }
      return &_nonRootInlineLevelBoxList[entry->second];
    }

    KRYS_NODISCARD InlineRect LogicalRectForInlineLevelBox(const Box &layoutBox) const noexcept;

    void SetLogicalRect(const InlineRect &logicalRect) noexcept
    {
      _logicalRect = logicalRect;
    }
    void SetHasContent(bool hasContent) noexcept
    {
      _hasContent = hasContent;
    }
    void SetBaselineType(FontBaseline baselineType) noexcept
    {
      _baselineType = baselineType;
    }

    KRYS_NODISCARD InlineLayoutUnit
      InlineLevelBoxAbsoluteTop(const InlineLevelBox &inlineLevelBox) const noexcept;
  };
}
