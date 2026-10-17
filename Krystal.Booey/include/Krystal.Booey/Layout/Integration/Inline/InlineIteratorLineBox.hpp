#pragma once

#include "Krystal.Booey/Graphics/FontBaseline.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorBox.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::inline_iterator
{
  class LineBoxIterator;

  /// @brief A position in a block container's display lines.
  class LineBoxIteratorModernPath
  {
  private:
    WeakPtr<const layout_integration::InlineContent> _inlineContent;
    size_t _lineIndex {0};

  public:
    /// @brief An iterator at the end, with no content.
    LineBoxIteratorModernPath() noexcept = default;
    LineBoxIteratorModernPath(const layout_integration::InlineContent &inlineContent,
                              size_t lineIndex) noexcept
        : _inlineContent(WeakPtr<const layout_integration::InlineContent>::WithRef(inlineContent)),
          _lineIndex(lineIndex)
    {
      krys_debug_assert(lineIndex <= Lines().size());
    }

    KRYS_NODISCARD float ContentLogicalTop() const noexcept
    {
      return GetLine().EnclosingContentLogicalTop();
    }
    KRYS_NODISCARD float ContentLogicalBottom() const noexcept
    {
      return GetLine().EnclosingContentLogicalBottom();
    }
    KRYS_NODISCARD float LogicalTop() const noexcept
    {
      return GetLine().LineBoxLogicalRect().Y();
    }
    KRYS_NODISCARD float LogicalBottom() const noexcept
    {
      return GetLine().LineBoxLogicalRect().MaxY();
    }
    KRYS_NODISCARD float LogicalWidth() const noexcept
    {
      return GetLine().LineBoxLogicalRect().Width();
    }
    KRYS_NODISCARD float InkOverflowLogicalTop() const noexcept
    {
      return GetLine().IsHorizontal() ? GetLine().InkOverflow().Y() : GetLine().InkOverflow().X();
    }
    KRYS_NODISCARD float InkOverflowLogicalBottom() const noexcept
    {
      return GetLine().IsHorizontal() ? GetLine().InkOverflow().MaxY() : GetLine().InkOverflow().MaxX();
    }
    KRYS_NODISCARD float ScrollableOverflowTop() const noexcept
    {
      return GetLine().ScrollableOverflow().Y();
    }
    KRYS_NODISCARD float ScrollableOverflowBottom() const noexcept
    {
      return GetLine().ScrollableOverflow().MaxY();
    }

    KRYS_NODISCARD bool HasEllipsis() const noexcept
    {
      return GetLine().HasEllipsis();
    }
    KRYS_NODISCARD FloatRect EllipsisVisualRectIgnoringBlockDirection() const noexcept
    {
      return GetLine().GetEllipsis()->VisualRect;
    }
    KRYS_NODISCARD TextRun EllipsisText() const noexcept
    {
      return TextRun {GetLine().GetEllipsis()->Text.View()};
    }

    KRYS_NODISCARD float ContentLogicalTopAdjustedForPrecedingLineBox() const noexcept;
    KRYS_NODISCARD float ContentLogicalBottomAdjustedForFollowingLineBox() const noexcept;
    KRYS_NODISCARD float ContentLogicalLeft() const noexcept;
    KRYS_NODISCARD float ContentLogicalRight() const noexcept
    {
      return ContentLogicalLeft() + GetLine().ContentLogicalWidth();
    }

    KRYS_NODISCARD bool IsHorizontal() const noexcept
    {
      return GetLine().IsHorizontal();
    }
    KRYS_NODISCARD FontBaseline BaselineType() const noexcept
    {
      return GetLine().BaselineType();
    }

    KRYS_NODISCARD const RenderBlockFlow &FormattingContextRoot() const noexcept
    {
      return _inlineContent->FormattingContextRoot();
    }

    KRYS_NODISCARD bool IsFirstAfterPageBreak() const noexcept
    {
      return GetLine().IsFirstAfterPageBreak();
    }
    KRYS_NODISCARD bool HasBlockLevelBox() const noexcept
    {
      return GetLine().HasBlockLevelBox();
    }

    KRYS_NODISCARD size_t LineIndex() const noexcept
    {
      return _lineIndex;
    }

    void TraverseNext() noexcept
    {
      krys_debug_assert(!AtEnd());
      ++_lineIndex;
    }

    void TraversePrevious() noexcept
    {
      krys_debug_assert(!AtEnd());
      if (!_lineIndex)
      {
        SetAtEnd();
        return;
      }
      --_lineIndex;
    }

    KRYS_NODISCARD friend bool operator==(const LineBoxIteratorModernPath &a,
                                          const LineBoxIteratorModernPath &b) noexcept
    {
      return a._inlineContent.get() == b._inlineContent.get() && a._lineIndex == b._lineIndex;
    }

    KRYS_NODISCARD bool AtEnd() const noexcept
    {
      return !_inlineContent || _lineIndex == Lines().size();
    }

    KRYS_NODISCARD BoxModernPath FirstLeafBox() const noexcept;
    KRYS_NODISCARD BoxModernPath LastLeafBox() const noexcept;

  private:
    void SetAtEnd() noexcept
    {
      _lineIndex = Lines().size();
    }

    KRYS_NODISCARD const inline_display::Lines &Lines() const noexcept
    {
      return _inlineContent->DisplayContent().LineList;
    }
    KRYS_NODISCARD const inline_display::Line &GetLine() const noexcept
    {
      return Lines()[_lineIndex];
    }
  };

  /// @brief A line of a block container's inline content.
  // FIXME: The ellipsis' selection state, once there's selection.
  class LineBox
  {
    friend class LineBoxIterator;

  private:
    LineBoxIteratorModernPath _path;

  public:
    explicit LineBox(LineBoxIteratorModernPath &&path) noexcept : _path(krys::move(path))
    {
    }

    KRYS_NODISCARD float LogicalTop() const noexcept
    {
      return _path.LogicalTop();
    }
    KRYS_NODISCARD float LogicalBottom() const noexcept
    {
      return _path.LogicalBottom();
    }
    KRYS_NODISCARD float LogicalHeight() const noexcept
    {
      return LogicalBottom() - LogicalTop();
    }
    KRYS_NODISCARD float LogicalWidth() const noexcept
    {
      return _path.LogicalWidth();
    }

    KRYS_NODISCARD float ContentLogicalTop() const noexcept
    {
      return _path.ContentLogicalTop();
    }
    KRYS_NODISCARD float ContentLogicalBottom() const noexcept
    {
      return _path.ContentLogicalBottom();
    }
    KRYS_NODISCARD float ContentLogicalLeft() const noexcept
    {
      return _path.ContentLogicalLeft();
    }
    KRYS_NODISCARD float ContentLogicalRight() const noexcept
    {
      return _path.ContentLogicalRight();
    }
    KRYS_NODISCARD float ContentLogicalWidth() const noexcept
    {
      return ContentLogicalRight() - ContentLogicalLeft();
    }
    KRYS_NODISCARD float ContentLogicalHeight() const noexcept
    {
      return ContentLogicalBottom() - ContentLogicalTop();
    }

    KRYS_NODISCARD float ContentLogicalTopAdjustedForPrecedingLineBox() const noexcept
    {
      return _path.ContentLogicalTopAdjustedForPrecedingLineBox();
    }
    KRYS_NODISCARD float ContentLogicalBottomAdjustedForFollowingLineBox() const noexcept
    {
      return _path.ContentLogicalBottomAdjustedForFollowingLineBox();
    }

    KRYS_NODISCARD float InkOverflowLogicalTop() const noexcept
    {
      return _path.InkOverflowLogicalTop();
    }
    KRYS_NODISCARD float InkOverflowLogicalBottom() const noexcept
    {
      return _path.InkOverflowLogicalBottom();
    }
    KRYS_NODISCARD float ScrollableOverflowTop() const noexcept
    {
      return _path.ScrollableOverflowTop();
    }
    KRYS_NODISCARD float ScrollableOverflowBottom() const noexcept
    {
      return _path.ScrollableOverflowBottom();
    }

    KRYS_NODISCARD const RenderStyle &Style() const noexcept;

    KRYS_NODISCARD bool HasEllipsis() const noexcept
    {
      return _path.HasEllipsis();
    }
    // FIXME: The rect adjusted for selection, once there's selection.
    KRYS_NODISCARD FloatRect EllipsisVisualRect() const noexcept;
    KRYS_NODISCARD TextRun EllipsisText() const noexcept
    {
      krys_debug_assert(HasEllipsis());
      return _path.EllipsisText();
    }

    KRYS_NODISCARD const RenderBlockFlow &FormattingContextRoot() const noexcept
    {
      return _path.FormattingContextRoot();
    }

    KRYS_NODISCARD bool IsHorizontal() const noexcept
    {
      return _path.IsHorizontal();
    }
    KRYS_NODISCARD FontBaseline BaselineType() const noexcept
    {
      return _path.BaselineType();
    }

    KRYS_NODISCARD bool IsFirst() const noexcept;
    KRYS_NODISCARD bool IsFirstAfterPageBreak() const noexcept
    {
      return _path.IsFirstAfterPageBreak();
    }
    KRYS_NODISCARD bool HasBlockContent() const noexcept
    {
      return _path.HasBlockLevelBox();
    }
    KRYS_NODISCARD LeafBoxIterator BlockLevelBox() const noexcept;

    // Text-relative left/right
    KRYS_NODISCARD LeafBoxIterator LineLeftmostLeafBox() const noexcept;
    KRYS_NODISCARD LeafBoxIterator LineRightmostLeafBox() const noexcept;

    // Coordinate-relative left/right
    KRYS_NODISCARD LeafBoxIterator LogicalLeftmostLeafBox() const noexcept;
    KRYS_NODISCARD LeafBoxIterator LogicalRightmostLeafBox() const noexcept;

    KRYS_NODISCARD LineBoxIterator Next() const noexcept;
    KRYS_NODISCARD LineBoxIterator Previous() const noexcept;

    KRYS_NODISCARD size_t LineIndex() const noexcept
    {
      return _path.LineIndex();
    }
  };

  struct EndLineBoxIterator
  {
  };

  /// @brief Iterates the lines of a block container.
  class LineBoxIterator
  {
  private:
    LineBox _lineBox;

  public:
    /// @brief An iterator at the end.
    LineBoxIterator() noexcept : _lineBox(LineBoxIteratorModernPath {})
    {
    }
    explicit LineBoxIterator(LineBoxIteratorModernPath &&path) noexcept : _lineBox(krys::move(path))
    {
    }
    explicit LineBoxIterator(const LineBox &lineBox) noexcept : _lineBox(lineBox)
    {
    }

    LineBoxIterator &operator++() noexcept
    {
      return TraverseNext();
    }
    LineBoxIterator &operator--() noexcept
    {
      return TraversePrevious();
    }

    LineBoxIterator &TraverseNext() noexcept;
    LineBoxIterator &TraversePrevious() noexcept;

    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return !AtEnd();
    }

    KRYS_NODISCARD bool operator==(const LineBoxIterator &other) const noexcept
    {
      return _lineBox._path == other._lineBox._path;
    }
    KRYS_NODISCARD bool operator==(EndLineBoxIterator) const noexcept
    {
      return AtEnd();
    }

    KRYS_NODISCARD const LineBox &operator*() const noexcept
    {
      return _lineBox;
    }
    KRYS_NODISCARD const LineBox *operator->() const noexcept
    {
      return &_lineBox;
    }

    KRYS_NODISCARD bool AtEnd() const noexcept
    {
      return _lineBox._path.AtEnd();
    }
  };

  KRYS_NODISCARD LineBoxIterator FirstLineBoxFor(const RenderBlockFlow &flow) noexcept;
  KRYS_NODISCARD LineBoxIterator LastLineBoxFor(const RenderBlockFlow &flow) noexcept;
  KRYS_NODISCARD LineBoxIterator LineBoxFor(const layout_integration::InlineContent &inlineContent,
                                            size_t lineIndex) noexcept;

  KRYS_NODISCARD float PreviousLineBoxContentBottomOrBorderAndPadding(const LineBox &lineBox) noexcept;
  KRYS_NODISCARD float ContentStartInBlockDirection(const LineBox &lineBox) noexcept;
}
