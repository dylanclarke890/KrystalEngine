#pragma once

#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Layout/LayoutUnits.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  /// @brief A layout rect that, in debug builds, asserts that each edge is set before it's read.
  class Rect
  {
  private:
#if KRYS_DEBUG
    bool _hasValidTop {false};
    bool _hasValidLeft {false};
    bool _hasValidWidth {false};
    bool _hasValidHeight {false};
#endif
    LayoutRect _rect;

  public:
    Rect() noexcept = default;

    Rect(LayoutUnit top, LayoutUnit left, LayoutUnit width, LayoutUnit height) noexcept
        : _rect(left, top, width, height)
    {
#if KRYS_DEBUG
      _hasValidTop = true;
      _hasValidLeft = true;
      _hasValidWidth = true;
      _hasValidHeight = true;
#endif
    }

    Rect(LayoutPoint topLeft, LayoutUnit width, LayoutUnit height) noexcept
        : Rect(topLeft.Y(), topLeft.X(), width, height)
    {
    }

    Rect(const LayoutPoint &topLeft, const LayoutSize &size) noexcept
        : Rect(topLeft.Y(), topLeft.X(), size.Width(), size.Height())
    {
    }

    explicit Rect(const FloatRect &rect) noexcept
        : Rect(LayoutPoint(rect.Location()), LayoutSize(rect.Size()))
    {
    }

    KRYS_NODISCARD LayoutUnit Top() const noexcept
    {
      krys_debug_assert(_hasValidTop);
      return _rect.Y();
    }

    KRYS_NODISCARD LayoutUnit Left() const noexcept
    {
      krys_debug_assert(_hasValidLeft);
      return _rect.X();
    }

    KRYS_NODISCARD LayoutPoint TopLeft() const noexcept
    {
      krys_debug_assert(HasValidPosition());
      return _rect.MinXMinYCorner();
    }

    KRYS_NODISCARD LayoutUnit Bottom() const noexcept
    {
      krys_debug_assert(_hasValidTop && _hasValidHeight);
      return _rect.MaxY();
    }

    KRYS_NODISCARD LayoutUnit Right() const noexcept
    {
      krys_debug_assert(_hasValidLeft && _hasValidWidth);
      return _rect.MaxX();
    }

    KRYS_NODISCARD LayoutPoint BottomRight() const noexcept
    {
      krys_debug_assert(HasValidGeometry());
      return _rect.MaxXMaxYCorner();
    }

    KRYS_NODISCARD LayoutUnit Width() const noexcept
    {
      krys_debug_assert(_hasValidWidth);
      return _rect.Width();
    }

    KRYS_NODISCARD LayoutUnit Height() const noexcept
    {
      krys_debug_assert(_hasValidHeight);
      return _rect.Height();
    }

    KRYS_NODISCARD LayoutSize Size() const noexcept
    {
      krys_debug_assert(HasValidSize());
      return _rect.Size();
    }

    void SetTop(LayoutUnit top) noexcept
    {
#if KRYS_DEBUG
      _hasValidTop = true;
#endif
      _rect.SetY(top);
    }

    void SetLeft(LayoutUnit left) noexcept
    {
#if KRYS_DEBUG
      _hasValidLeft = true;
#endif
      _rect.SetX(left);
    }

    void SetTopLeft(const LayoutPoint &topLeft) noexcept
    {
#if KRYS_DEBUG
      _hasValidTop = true;
      _hasValidLeft = true;
#endif
      _rect.SetLocation(topLeft);
    }

    void SetWidth(LayoutUnit width) noexcept
    {
#if KRYS_DEBUG
      _hasValidWidth = true;
#endif
      _rect.SetWidth(width);
    }

    void SetHeight(LayoutUnit height) noexcept
    {
#if KRYS_DEBUG
      _hasValidHeight = true;
#endif
      _rect.SetHeight(height);
    }

    void SetSize(const LayoutSize &size) noexcept
    {
#if KRYS_DEBUG
      _hasValidWidth = true;
      _hasValidHeight = true;
#endif
      _rect.SetSize(size);
    }

    void ShiftLeftTo(LayoutUnit left) noexcept
    {
      krys_debug_assert(_hasValidLeft);
      _rect.ShiftXEdgeTo(left);
    }

    void ShiftRightTo(LayoutUnit right) noexcept
    {
      krys_debug_assert(_hasValidLeft && _hasValidWidth);
      _rect.ShiftMaxXEdgeTo(right);
    }

    void ShiftTopTo(LayoutUnit top) noexcept
    {
      krys_debug_assert(_hasValidTop);
      _rect.ShiftYEdgeTo(top);
    }

    void ShiftBottomTo(LayoutUnit bottom) noexcept
    {
      krys_debug_assert(_hasValidTop && _hasValidHeight);
      _rect.ShiftMaxYEdgeTo(bottom);
    }

    void MoveHorizontally(LayoutUnit offset) noexcept
    {
      krys_debug_assert(_hasValidLeft);
      _rect.Move(LayoutSize {offset, 0_lu});
    }

    void MoveVertically(LayoutUnit offset) noexcept
    {
      krys_debug_assert(_hasValidTop);
      _rect.Move(LayoutSize {0_lu, offset});
    }

    void MoveBy(LayoutPoint offset) noexcept
    {
      krys_debug_assert(_hasValidTop);
      krys_debug_assert(_hasValidLeft);
      _rect.MoveBy(offset);
    }

    void Expand(Maybe<LayoutUnit> width, Maybe<LayoutUnit> height) noexcept
    {
      krys_debug_assert(!width || _hasValidWidth);
      krys_debug_assert(!height || _hasValidHeight);
      _rect.Expand(width.value_or(0_lu), height.value_or(0_lu));
    }

    void ExpandHorizontally(LayoutUnit delta) noexcept
    {
      Expand(delta, null);
    }

    void ExpandVertically(LayoutUnit delta) noexcept
    {
      Expand(null, delta);
    }

    void ExpandToContain(const Rect &rect) noexcept
    {
      krys_debug_assert(_hasValidWidth);
      krys_debug_assert(_hasValidHeight);
      _rect.UniteEvenIfEmpty(rect);
    }

    KRYS_NODISCARD bool Intersects(const Rect &rect) const noexcept
    {
      return _rect.Intersects(rect);
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _rect.IsEmpty();
    }

    KRYS_NODISCARD Rect Clone() const noexcept
    {
      Rect rect;
#if KRYS_DEBUG
      rect._hasValidTop = _hasValidTop;
      rect._hasValidLeft = _hasValidLeft;
      rect._hasValidWidth = _hasValidWidth;
      rect._hasValidHeight = _hasValidHeight;
#endif
      rect._rect = _rect;
      return rect;
    }

    operator LayoutRect() const noexcept
    {
      krys_debug_assert(HasValidGeometry());
      return _rect;
    }

    operator FloatRect() const noexcept
    {
      krys_debug_assert(HasValidGeometry());
      return _rect;
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
