#pragma once

#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorBoxLegacyPath.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorBoxModernPath.hpp"
#include "Krystal.Booey/Rendering/RenderObjectHighlightState.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Visitor.hpp"

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
  class Box
  {
    friend class BoxIterator;
    friend class InlineBoxIterator;
    friend class LeafBoxIterator;
    friend class TextBoxIterator;

  public:
    using PathVariant = Variant<BoxModernPath, BoxLegacyPath>;

  protected:
    PathVariant _pathVariant;

  public:
    explicit Box(PathVariant &&path) noexcept : _pathVariant(krys::move(path))
    {
    }

    KRYS_NODISCARD bool IsSVGText() const noexcept;

    KRYS_NODISCARD bool IsText() const noexcept
    {
      return SwitchOn(_pathVariant, [&](const auto &path) -> decltype(auto) { return path.IsText(); });
    }
    KRYS_NODISCARD bool IsInlineBox() const noexcept
    {
      return SwitchOn(_pathVariant, [&](const auto &path) -> decltype(auto) { return path.IsInlineBox(); });
    }
    KRYS_NODISCARD bool IsRootInlineBox() const noexcept
    {
      return SwitchOn(_pathVariant,
                      [&](const auto &path) -> decltype(auto) { return path.IsRootInlineBox(); });
    }
    KRYS_NODISCARD bool IsLineBreak() const noexcept
    {
      return SwitchOn(_pathVariant, [&](const auto &path) -> decltype(auto) { return path.IsLineBreak(); });
    }
    KRYS_NODISCARD bool IsBlockLevelBox() const noexcept
    {
      return SwitchOn(_pathVariant,
                      [&](const auto &path) -> decltype(auto) { return path.IsBlockLevelBox(); });
    }
    KRYS_NODISCARD bool IsAtomicInlineBox() const noexcept
    {
      return SwitchOn(_pathVariant,
                      [&](const auto &path) -> decltype(auto) { return path.IsAtomicInlineBox(); });
    }

    KRYS_NODISCARD FloatRect VisualRect() const noexcept;
    KRYS_NODISCARD FloatRect VisualRectIgnoringBlockDirection() const noexcept
    {
      return SwitchOn(_pathVariant, [&](const auto &path) -> decltype(auto)
                      { return path.VisualRectIgnoringBlockDirection(); });
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
      return SwitchOn(_pathVariant, [&](const auto &path) -> decltype(auto) { return path.IsHorizontal(); });
    }

    KRYS_NODISCARD uint32 MinimumCaretOffset() const noexcept
    {
      return SwitchOn(_pathVariant,
                      [&](const auto &path) -> decltype(auto) { return path.MinimumCaretOffset(); });
    }
    KRYS_NODISCARD uint32 MaximumCaretOffset() const noexcept
    {
      return SwitchOn(_pathVariant,
                      [&](const auto &path) -> decltype(auto) { return path.MaximumCaretOffset(); });
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
      return SwitchOn(_pathVariant, [&](const auto &path) -> decltype(auto) { return path.BidiLevel(); });
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
      return SwitchOn(_pathVariant, [&](const auto &path) -> decltype(auto) { return path.Renderer(); });
    }
    KRYS_NODISCARD const RenderBlockFlow &FormattingContextRoot() const noexcept
    {
      return SwitchOn(_pathVariant,
                      [&](const auto &path) -> decltype(auto) { return path.FormattingContextRoot(); });
    }
    KRYS_NODISCARD const RenderStyle &Style() const noexcept
    {
      return SwitchOn(_pathVariant, [&](const auto &path) -> decltype(auto) { return path.Style(); });
    }
    KRYS_NODISCARD WritingMode GetWritingMode() const noexcept;

    // FIXME: Remove. For intermediate porting steps only.
    KRYS_NODISCARD const LegacyInlineBox *GetLegacyInlineBox() const noexcept
    {
      if (!std::holds_alternative<BoxLegacyPath>(_pathVariant))
      {
        return nullptr;
      }
      return std::get<BoxLegacyPath>(_pathVariant).GetLegacyInlineBox();
    }
    KRYS_NODISCARD const inline_display::Box *GetInlineBox() const noexcept
    {
      if (!std::holds_alternative<BoxModernPath>(_pathVariant))
      {
        return nullptr;
      }
      return &std::get<BoxModernPath>(_pathVariant).GetBox();
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
      return SwitchOn(_pathVariant, [&](const auto &path) -> decltype(auto) { return path.LineIndex(); });
    }

    KRYS_NODISCARD const BoxModernPath &ModernPath() const noexcept
    {
      return std::get<BoxModernPath>(_pathVariant);
    }
    KRYS_NODISCARD const BoxLegacyPath &LegacyPath() const noexcept
    {
      return std::get<BoxLegacyPath>(_pathVariant);
    }

    /// @brief How much of the selection the box holds: for text, which part of the renderer's selection is in
    /// the box.
    KRYS_NODISCARD RenderObjectHighlightState SelectionState() const noexcept;

  private:
    KRYS_NODISCARD bool HasRenderer() const noexcept
    {
      return SwitchOn(_pathVariant, [&](const auto &path) -> decltype(auto) { return path.HasRenderer(); });
    }
  };

  /// @brief Iterates the boxes of a line, in visual order.
  class BoxIterator
  {
  protected:
    Box _box;

  public:
    /// @brief An iterator at the end.
    BoxIterator() noexcept : _box(BoxLegacyPath {nullptr})
    {
    }
    explicit BoxIterator(Box::PathVariant &&path) noexcept : _box(krys::move(path))
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
      return SwitchOn(_box._pathVariant, [](const auto &path) { return path.AtEnd(); });
    }
  };

  /// @brief Iterates the leaf boxes (everything but inline boxes) of a line.
  class LeafBoxIterator : public BoxIterator
  {
  public:
    LeafBoxIterator() noexcept = default;
    explicit LeafBoxIterator(Box::PathVariant &&path) noexcept : BoxIterator(krys::move(path))
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
