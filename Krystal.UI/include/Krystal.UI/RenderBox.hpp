#pragma once

#include "Krystal.Lib/Array.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys
{
  /// @brief Ordered by top, right, bottom, left.
  using EdgeSizes = Array<float, 4>;

  /// @brief Ordered by top-left, top-right, bottom-right, bottom-left.
  using CornerSizes = Array<float, 4>;

  /// @brief Provides the data needed to generate a mesh for a given element's box.
  class RenderBox
  {
  private:
    Maths::Vec2 _fillSize;
    Maths::Vec2 _borderOffset;
    EdgeSizes _borderWidths;
    CornerSizes _borderRadii;

  public:
    RenderBox(Maths::Vec2 fillSize, Maths::Vec2 borderOffset, EdgeSizes borderWidths, CornerSizes borderRadii)
        : _fillSize(fillSize), _borderOffset(borderOffset), _borderWidths(borderWidths),
          _borderRadii(borderRadii)
    {
    }

    /// @brief Returns the size of the fill area of the box.
    NO_DISCARD Maths::Vec2 GetFillSize() const noexcept
    {
      return _fillSize;
    }

    /// @brief Sets the size of the fill area of the box.
    void SetFillSize(Maths::Vec2 value) noexcept
    {
      _fillSize = value;
    }

    /// @brief Returns the offset from the border area to the fill area of the box.
    NO_DISCARD Maths::Vec2 GetFillOffset() const noexcept
    {
      return {_borderWidths[3], _borderWidths[0]};
    }

    /// @brief Returns the offset to the border area of the box.
    NO_DISCARD Maths::Vec2 GetBorderOffset() const noexcept
    {
      return _borderOffset;
    }

    /// @brief Sets the border offset.
    void SetBorderOffset(Maths::Vec2 value) noexcept
    {
      _borderOffset = value;
    }

    /// @brief Returns the border widths of the box.
    NO_DISCARD const EdgeSizes &GetBorderWidths() const noexcept
    {
      return _borderWidths;
    }

    /// @brief Sets the border widths of the box.
    void SetBorderWidths(const EdgeSizes &value) noexcept
    {
      _borderWidths = value;
    }

    /// @brief Returns the border radius of the box.
    NO_DISCARD const CornerSizes &GetBorderRadius() const noexcept
    {
      return _borderRadii;
    }

    /// Sets the border radius of the box.
    void SetBorderRadius(const CornerSizes &value) noexcept
    {
      _borderRadii = value;
    }
  };

  inline bool operator==(const RenderBox &a, const RenderBox &b)
  {
    return a.GetFillSize() == b.GetFillSize() && a.GetBorderOffset() == b.GetBorderOffset()
           && a.GetBorderWidths() == b.GetBorderWidths() && a.GetBorderRadius() == b.GetBorderRadius();
  }

  inline bool operator!=(const RenderBox &a, const RenderBox &b)
  {
    return !(a == b);
  }
}