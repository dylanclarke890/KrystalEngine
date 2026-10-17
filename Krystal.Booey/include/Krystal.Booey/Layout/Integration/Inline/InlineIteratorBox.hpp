#pragma once

#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorBoxModernPath.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class RenderBox;
  class RenderLineBreak;
}

namespace krys::boo::inline_iterator
{
  class InlineBoxIterator;
  class LeafBoxIterator;
  class LineBoxIterator;
  class TextBoxIterator;

  struct EndIterator
  {
  };

  /// @brief A box on a line: text, an inline box, an atomic inline box, a line break or a block level box.
  // FIXME: The selection state, once there's selection.
  class Box
  {
    friend class BoxIterator;
    friend class InlineBoxIterator;
    friend class LeafBoxIterator;
    friend class TextBoxIterator;

  protected:
    BoxModernPath _path;

  public:
    explicit Box(BoxModernPath &&path) noexcept : _path(krys::move(path))
    {
    }

    KRYS_NODISCARD bool IsText() const noexcept
    {
      return _path.IsText();
    }
    KRYS_NODISCARD bool IsInlineBox() const noexcept
    {
      return _path.IsInlineBox();
    }
    KRYS_NODISCARD bool IsRootInlineBox() const noexcept
    {
      return _path.IsRootInlineBox();
    }
    KRYS_NODISCARD bool IsLineBreak() const noexcept
    {
      return _path.IsLineBreak();
    }
    KRYS_NODISCARD bool IsBlockLevelBox() const noexcept
    {
      return _path.IsBlockLevelBox();
    }
    KRYS_NODISCARD bool IsAtomicInlineBox() const noexcept
    {
      return _path.IsAtomicInlineBox();
    }

    KRYS_NODISCARD FloatRect VisualRect() const noexcept;
    KRYS_NODISCARD FloatRect VisualRectIgnoringBlockDirection() const noexcept
    {
      return _path.VisualRectIgnoringBlockDirection();
    }

    /// @brief Visual in inline direction, logical for writing mode.
    KRYS_NODISCARD FloatRect LogicalRectIgnoringInlineDirection() const noexcept
    {
      auto rect = VisualRectIgnoringBlockDirection();
      return IsHorizontal() ? rect : rect.TransposedRect();
    }
    KRYS_NODISCARD float LogicalTop() const noexcept
    {
      return LogicalRectIgnoringInlineDirection().Y();
    }
    KRYS_NODISCARD float LogicalBottom() const noexcept
    {
      return LogicalRectIgnoringInlineDirection().MaxY();
    }
    KRYS_NODISCARD float LogicalHeight() const noexcept
    {
      return LogicalRectIgnoringInlineDirection().Height();
    }
    KRYS_NODISCARD float LogicalWidth() const noexcept
    {
      return LogicalRectIgnoringInlineDirection().Width();
    }
    KRYS_NODISCARD float LogicalLeft() const noexcept
    {
      return IsHorizontal() ? VisualRect().X() : VisualRect().Y();
    }
    KRYS_NODISCARD float LogicalRight() const noexcept
    {
      return LogicalLeft() + LogicalWidth();
    }

    /// @brief Line-relative left/right coords (they are still considered logical values as there's no flip
    /// for writing mode).
    KRYS_NODISCARD float LogicalLeftIgnoringInlineDirection() const noexcept
    {
      return LogicalRectIgnoringInlineDirection().X();
    }
    KRYS_NODISCARD float LogicalRightIgnoringInlineDirection() const noexcept
    {
      return LogicalRectIgnoringInlineDirection().MaxX();
    }

    KRYS_NODISCARD bool IsHorizontal() const noexcept
    {
      return _path.IsHorizontal();
    }

    KRYS_NODISCARD uint32 MinimumCaretOffset() const noexcept
    {
      return _path.MinimumCaretOffset();
    }
    KRYS_NODISCARD uint32 MaximumCaretOffset() const noexcept
    {
      return _path.MaximumCaretOffset();
    }
    KRYS_NODISCARD uint32 LeftmostCaretOffset() const noexcept
    {
      return IsLeftToRightDirection() ? MinimumCaretOffset() : MaximumCaretOffset();
    }
    KRYS_NODISCARD uint32 RightmostCaretOffset() const noexcept
    {
      return IsLeftToRightDirection() ? MaximumCaretOffset() : MinimumCaretOffset();
    }

    /// @note IsLeftToRightDirection() here is not the same as GetWritingMode().IsBidiLTR().
    KRYS_NODISCARD UBiDiLevel BidiLevel() const noexcept
    {
      return _path.BidiLevel();
    }
    KRYS_NODISCARD TextDirection Direction() const noexcept
    {
      return BidiLevel() % 2 ? TextDirection::RTL : TextDirection::LTR;
    }
    KRYS_NODISCARD bool IsLeftToRightDirection() const noexcept
    {
      return Direction() == TextDirection::LTR;
    }
    KRYS_NODISCARD bool IsInlineFlipped() const noexcept
    {
      return !(IsLeftToRightDirection() == GetWritingMode().IsLogicalLeftLineLeft());
    }

    KRYS_NODISCARD const RenderObject &Renderer() const noexcept
    {
      return _path.Renderer();
    }
    KRYS_NODISCARD const RenderBlockFlow &FormattingContextRoot() const noexcept
    {
      return _path.FormattingContextRoot();
    }
    KRYS_NODISCARD const RenderStyle &Style() const noexcept
    {
      return _path.Style();
    }
    KRYS_NODISCARD WritingMode GetWritingMode() const noexcept;

    KRYS_NODISCARD const inline_display::Box *GetInlineBox() const noexcept
    {
      return &_path.GetBox();
    }

    // Text-relative left/right
    KRYS_NODISCARD LeafBoxIterator NextLineRightwardOnLine() const noexcept;
    KRYS_NODISCARD LeafBoxIterator NextLineLeftwardOnLine() const noexcept;
    KRYS_NODISCARD LeafBoxIterator NextLineRightwardOnLineIgnoringLineBreak() const noexcept;
    KRYS_NODISCARD LeafBoxIterator NextLineLeftwardOnLineIgnoringLineBreak() const noexcept;

    // Coordinate-relative left/right
    KRYS_NODISCARD LeafBoxIterator NextLogicalRightwardOnLine() const noexcept;
    KRYS_NODISCARD LeafBoxIterator NextLogicalLeftwardOnLine() const noexcept;
    KRYS_NODISCARD LeafBoxIterator NextLogicalRightwardOnLineIgnoringLineBreak() const noexcept;
    KRYS_NODISCARD LeafBoxIterator NextLogicalLeftwardOnLineIgnoringLineBreak() const noexcept;

    KRYS_NODISCARD InlineBoxIterator ParentInlineBox() const noexcept;
    KRYS_NODISCARD LineBoxIterator GetLineBox() const noexcept;
    KRYS_NODISCARD size_t LineIndex() const noexcept
    {
      return _path.LineIndex();
    }

    KRYS_NODISCARD const BoxModernPath &ModernPath() const noexcept
    {
      return _path;
    }

  private:
    KRYS_NODISCARD bool HasRenderer() const noexcept
    {
      return _path.HasRenderer();
    }
  };

  /// @brief Iterates the boxes of a line, in visual order.
  class BoxIterator
  {
  protected:
    Box _box;

  public:
    /// @brief An iterator at the end.
    BoxIterator() noexcept : _box(BoxModernPath {})
    {
    }
    explicit BoxIterator(BoxModernPath &&path) noexcept : _box(krys::move(path))
    {
    }
    explicit BoxIterator(const Box &box) noexcept : _box(box)
    {
    }

    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return !AtEnd();
    }

    KRYS_NODISCARD bool operator==(const BoxIterator &other) const noexcept;
    KRYS_NODISCARD bool operator==(EndIterator) const noexcept
    {
      return AtEnd();
    }

    KRYS_NODISCARD const Box &operator*() const noexcept
    {
      return _box;
    }
    KRYS_NODISCARD const Box *operator->() const noexcept
    {
      return &_box;
    }

    BoxIterator &TraverseLineRightwardOnLine() noexcept;
    BoxIterator &TraverseLineRightwardOnLineSkippingChildren() noexcept;

    BoxIterator &operator++() noexcept
    {
      return TraverseLineRightwardOnLine();
    }

    KRYS_NODISCARD bool AtEnd() const noexcept
    {
      return _box._path.AtEnd();
    }
  };

  /// @brief Iterates the leaf boxes (everything but inline boxes) of a line.
  class LeafBoxIterator : public BoxIterator
  {
  public:
    LeafBoxIterator() noexcept = default;
    explicit LeafBoxIterator(BoxModernPath &&path) noexcept : BoxIterator(krys::move(path))
    {
    }
    explicit LeafBoxIterator(const Box &box) noexcept : BoxIterator(box)
    {
    }

    // Text-relative left/right
    LeafBoxIterator &TraverseLineRightwardOnLine() noexcept;
    LeafBoxIterator &TraverseLineLeftwardOnLine() noexcept;
    LeafBoxIterator &TraverseLineRightwardOnLineIgnoringLineBreak() noexcept;
    LeafBoxIterator &TraverseLineLeftwardOnLineIgnoringLineBreak() noexcept;

    // Coordinate-relative left/right
    LeafBoxIterator &TraverseLogicalRightwardOnLine() noexcept;
    LeafBoxIterator &TraverseLogicalLeftwardOnLine() noexcept;
    LeafBoxIterator &TraverseLogicalRightwardOnLineIgnoringLineBreak() noexcept;
    LeafBoxIterator &TraverseLogicalLeftwardOnLineIgnoringLineBreak() noexcept;

    LeafBoxIterator &operator++() noexcept
    {
      return TraverseLineRightwardOnLine();
    }
  };

  /// @brief A range from an iterator to the end, for range-based for loops.
  template <typename TIterator>
  class BoxRange
  {
  private:
    TIterator _begin;

  public:
    explicit BoxRange(TIterator begin) noexcept : _begin(begin)
    {
    }

    KRYS_NODISCARD TIterator begin() const noexcept
    {
      return _begin;
    }
    KRYS_NODISCARD EndIterator end() const noexcept
    {
      return {};
    }
  };

  KRYS_NODISCARD LeafBoxIterator BoxFor(const RenderLineBreak &renderer) noexcept;
  KRYS_NODISCARD LeafBoxIterator BoxFor(const RenderBox &renderer) noexcept;
  KRYS_NODISCARD LeafBoxIterator BoxFor(const layout_integration::InlineContent &content,
                                        size_t boxIndex) noexcept;
}
