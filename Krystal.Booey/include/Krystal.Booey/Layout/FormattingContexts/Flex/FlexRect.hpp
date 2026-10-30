#pragma once

#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::layout
{
  /// @brief A flex item's logical rect (content or border box, depending on its box-sizing) with its main and
  /// cross axis margins. In debug builds, asserts that each edge is set before it's read.
  class FlexRect
  {
  public:
    struct Margins
    {
      LayoutUnit Start;
      LayoutUnit End;
    };

  private:
#if KRYS_DEBUG
    bool _hasValidTop {false};
    bool _hasValidLeft {false};
    bool _hasValidWidth {false};
    bool _hasValidHeight {false};
#endif
    LayoutRect _rect;
    Margins _mainAxisMargins;
    Margins _crossAxisMargins;

  public:
    FlexRect() noexcept = default;

    FlexRect(LayoutUnit top, LayoutUnit left, LayoutUnit width, LayoutUnit height, Margins mainAxisMargins,
             Margins crossAxisMargins) noexcept
        : _rect(left, top, width, height), _mainAxisMargins(mainAxisMargins),
          _crossAxisMargins(crossAxisMargins)
    {
#if KRYS_DEBUG
      _hasValidTop = true;
      _hasValidLeft = true;
      _hasValidWidth = true;
      _hasValidHeight = true;
#endif
    }

    FlexRect(const LayoutRect &rect, Margins mainAxisMargins, Margins crossAxisMargins) noexcept
        : FlexRect(rect.Y(), rect.X(), rect.Width(), rect.Height(), mainAxisMargins, crossAxisMargins)
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

    KRYS_NODISCARD LayoutUnit MarginTop() const noexcept
    {
      return _crossAxisMargins.Start;
    }
    KRYS_NODISCARD LayoutUnit MarginBottom() const noexcept
    {
      return _crossAxisMargins.End;
    }
    KRYS_NODISCARD LayoutUnit MarginLeft() const noexcept
    {
      return _mainAxisMargins.Start;
    }
    KRYS_NODISCARD LayoutUnit MarginRight() const noexcept
    {
      return _mainAxisMargins.End;
    }

    void SetTop(LayoutUnit top) noexcept
    {
#if KRYS_DEBUG
      _hasValidTop = true;
#endif
      _rect.SetY(top);
    }

    void SetBottom(LayoutUnit bottom) noexcept
    {
#if KRYS_DEBUG
      _hasValidTop = true;
      _hasValidHeight = true;
#endif
      _rect.ShiftMaxYEdgeTo(bottom);
    }

    void SetLeft(LayoutUnit left) noexcept
    {
#if KRYS_DEBUG
      _hasValidLeft = true;
#endif
      _rect.SetX(left);
    }

    void SetRight(LayoutUnit right) noexcept
    {
#if KRYS_DEBUG
      _hasValidLeft = true;
      _hasValidWidth = true;
#endif
      _rect.ShiftMaxXEdgeTo(right);
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

    void SetMarginTop(LayoutUnit marginTop) noexcept
    {
      _crossAxisMargins.Start = marginTop;
    }
    void SetMarginBottom(LayoutUnit marginBottom) noexcept
    {
      _crossAxisMargins.End = marginBottom;
    }
    void SetMarginLeft(LayoutUnit marginLeft) noexcept
    {
      _mainAxisMargins.Start = marginLeft;
    }
    void SetMarginRight(LayoutUnit marginRight) noexcept
    {
      _mainAxisMargins.End = marginRight;
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

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      krys_debug_assert(HasValidGeometry());
      return _rect.IsEmpty();
    }

    explicit operator LayoutRect() const noexcept
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
