#pragma once

#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief The selection gaps a block fills: left of the selected content, between it, and right of it.
  struct GapRects
  {
  private:
    LayoutRect _left;
    LayoutRect _center;
    LayoutRect _right;

  public:
    KRYS_NODISCARD const LayoutRect &Left() const noexcept
    {
      return _left;
    }
    KRYS_NODISCARD const LayoutRect &Center() const noexcept
    {
      return _center;
    }
    KRYS_NODISCARD const LayoutRect &Right() const noexcept
    {
      return _right;
    }

    void UniteLeft(const LayoutRect &r) noexcept
    {
      _left.Unite(r);
    }
    void UniteCenter(const LayoutRect &r) noexcept
    {
      _center.Unite(r);
    }
    void UniteRight(const LayoutRect &r) noexcept
    {
      _right.Unite(r);
    }
    void Unite(const GapRects &o) noexcept
    {
      UniteLeft(o.Left());
      UniteCenter(o.Center());
      UniteRight(o.Right());
    }

    operator LayoutRect() const noexcept
    {
      LayoutRect result = _left;
      result.Unite(_center);
      result.Unite(_right);
      return result;
    }

    KRYS_NODISCARD friend bool operator==(const GapRects &, const GapRects &) noexcept = default;
  };
}
