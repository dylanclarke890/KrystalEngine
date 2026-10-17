#pragma once

#include "Krystal.Booey/Layout/LayoutUnits.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  /// @brief A float rect for inline layout that, in debug builds, asserts that each edge is set before it's
  /// read.
  class InlineRect
  {
  private:
#if KRYS_DEBUG
    bool _hasValidTop {false};
    bool _hasValidLeft {false};
    bool _hasValidWidth {false};
    bool _hasValidHeight {false};
#endif
    InlineLayoutRect _rect;

  public:
    InlineRect() noexcept = default;

    InlineRect(InlineLayoutUnit top, InlineLayoutUnit left, InlineLayoutUnit width,
               InlineLayoutUnit height) noexcept
        : _rect(left, top, width, height)
    {
#if KRYS_DEBUG
      _hasValidTop = true;
      _hasValidLeft = true;
      _hasValidWidth = true;
      _hasValidHeight = true;
#endif
    }

    InlineRect(const InlineLayoutPoint &topLeft, InlineLayoutUnit width, InlineLayoutUnit height) noexcept
        : InlineRect(topLeft.Y(), topLeft.X(), width, height)
    {
    }

    InlineRect(const InlineLayoutPoint &topLeft, const InlineLayoutSize &size) noexcept
        : InlineRect(topLeft.Y(), topLeft.X(), size.Width(), size.Height())
    {
    }

    InlineRect(const FloatRect &rect) noexcept : InlineRect(rect.Y(), rect.X(), rect.Width(), rect.Height())
    {
    }

    KRYS_NODISCARD InlineLayoutUnit Top() const noexcept
    {
      krys_debug_assert(_hasValidTop);
      return _rect.Y();
    }

    KRYS_NODISCARD InlineLayoutUnit Left() const noexcept
    {
      krys_debug_assert(_hasValidLeft);
      return _rect.X();
    }

    KRYS_NODISCARD InlineLayoutPoint TopLeft() const noexcept
    {
      krys_debug_assert(HasValidPosition());
      return _rect.MinXMinYCorner();
    }

    KRYS_NODISCARD InlineLayoutUnit Bottom() const noexcept
    {
      krys_debug_assert(_hasValidTop && _hasValidHeight);
      return _rect.MaxY();
    }

    KRYS_NODISCARD InlineLayoutUnit Right() const noexcept
    {
      krys_debug_assert(_hasValidLeft && _hasValidWidth);
      return _rect.MaxX();
    }

    KRYS_NODISCARD InlineLayoutUnit Width() const noexcept
    {
      krys_debug_assert(_hasValidWidth);
      return _rect.Width();
    }

    KRYS_NODISCARD InlineLayoutUnit Height() const noexcept
    {
      krys_debug_assert(_hasValidHeight);
      return _rect.Height();
    }

    KRYS_NODISCARD InlineLayoutSize Size() const noexcept
    {
      krys_debug_assert(HasValidSize());
      return _rect.Size();
    }

    void SetTop(InlineLayoutUnit top) noexcept
    {
#if KRYS_DEBUG
      _hasValidTop = true;
#endif
      _rect.SetY(top);
    }

    void SetBottom(InlineLayoutUnit bottom) noexcept
    {
#if KRYS_DEBUG
      _hasValidTop = true;
      _hasValidHeight = true;
#endif
      _rect.ShiftMaxYEdgeTo(bottom);
    }

    void SetLeft(InlineLayoutUnit left) noexcept
    {
#if KRYS_DEBUG
      _hasValidLeft = true;
#endif
      _rect.SetX(left);
    }

    void SetRight(InlineLayoutUnit right) noexcept
    {
#if KRYS_DEBUG
      _hasValidLeft = true;
      _hasValidWidth = true;
#endif
      _rect.ShiftMaxXEdgeTo(right);
    }

    void SetTopLeft(const InlineLayoutPoint &topLeft) noexcept
    {
#if KRYS_DEBUG
      _hasValidTop = true;
      _hasValidLeft = true;
#endif
      _rect.SetLocation(topLeft);
    }

    void SetWidth(InlineLayoutUnit width) noexcept
    {
#if KRYS_DEBUG
      _hasValidWidth = true;
#endif
      _rect.SetWidth(width);
    }

    void SetHeight(InlineLayoutUnit height) noexcept
    {
#if KRYS_DEBUG
      _hasValidHeight = true;
#endif
      _rect.SetHeight(height);
    }

    void MoveHorizontally(InlineLayoutUnit offset) noexcept
    {
      krys_debug_assert(_hasValidLeft);
      _rect.Move(InlineLayoutSize {offset, 0});
    }

    void MoveVertically(InlineLayoutUnit offset) noexcept
    {
      krys_debug_assert(_hasValidTop);
      _rect.Move(InlineLayoutSize {0, offset});
    }

    void MoveBy(InlineLayoutPoint offset) noexcept
    {
      krys_debug_assert(_hasValidTop);
      krys_debug_assert(_hasValidLeft);
      _rect.MoveBy(offset);
    }

    void ShiftLeftTo(InlineLayoutUnit left) noexcept
    {
      krys_debug_assert(_hasValidLeft);
      _rect.ShiftXEdgeTo(left);
    }

    void ShiftLeftBy(InlineLayoutUnit offset) noexcept
    {
      krys_debug_assert(_hasValidLeft);
      _rect.ShiftXEdgeBy(offset);
    }

    void ShiftRightBy(InlineLayoutUnit offset) noexcept
    {
      krys_debug_assert(_hasValidLeft && _hasValidWidth);
      _rect.ShiftMaxXEdgeBy(offset);
    }

    void Expand(Maybe<InlineLayoutUnit> width, Maybe<InlineLayoutUnit> height) noexcept
    {
      krys_debug_assert(!width || _hasValidWidth);
      krys_debug_assert(!height || _hasValidHeight);
      _rect.Expand(width.value_or(0), height.value_or(0));
    }

    void ExpandToContain(const InlineRect &other) noexcept
    {
#if KRYS_DEBUG
      _hasValidTop = true;
      _hasValidLeft = true;
      _hasValidWidth = true;
      _hasValidHeight = true;
#endif
      _rect.UniteEvenIfEmpty(other);
    }

    void ExpandHorizontally(InlineLayoutUnit delta) noexcept
    {
      Expand(delta, {});
    }

    void ExpandVertically(InlineLayoutUnit delta) noexcept
    {
      Expand({}, delta);
    }

    void ExpandVerticallyToContain(const InlineRect &other) noexcept
    {
      auto containTop = std::min(Top(), other.Top());
      auto containBottom = std::max(Bottom(), other.Bottom());
      SetTop(containTop);
      SetBottom(containBottom);
    }

    void Inflate(InlineLayoutUnit inflate) noexcept
    {
      krys_debug_assert(HasValidGeometry());
      _rect.Inflate(inflate);
    }

    void Inflate(InlineLayoutUnit top, InlineLayoutUnit right, InlineLayoutUnit bottom,
                 InlineLayoutUnit left) noexcept
    {
      krys_debug_assert(HasValidGeometry());
      _rect.SetX(_rect.X() - left);
      _rect.SetY(_rect.Y() - top);
      _rect.SetWidth(_rect.Width() + left + right);
      _rect.SetHeight(_rect.Height() + top + bottom);
    }

    KRYS_NODISCARD bool Intersects(const InlineRect &other) const noexcept
    {
      return _rect.Intersects(other);
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      krys_debug_assert(HasValidGeometry());
      return _rect.IsEmpty();
    }

    operator InlineLayoutRect() const noexcept
    {
      krys_debug_assert(HasValidGeometry());
      return _rect;
    }

    KRYS_NODISCARD friend bool operator==(const InlineRect &a, const InlineRect &b) noexcept
    {
      return static_cast<InlineLayoutRect>(a) == static_cast<InlineLayoutRect>(b);
    }

  private:
#if KRYS_DEBUG
    KRYS_NODISCARD bool HasValidPosition() const noexcept
    {
      return _hasValidTop && _hasValidLeft;
    }
    KRYS_NODISCARD bool HasValidSize() const noexcept
    {
      return _hasValidWidth && _hasValidHeight;
    }
    KRYS_NODISCARD bool HasValidGeometry() const noexcept
    {
      return HasValidPosition() && HasValidSize();
    }
#endif
  };
}
