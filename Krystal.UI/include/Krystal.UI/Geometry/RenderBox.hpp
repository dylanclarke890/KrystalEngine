#pragma once

#include "Krystal.Core/Maths/Vector.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.UI/Geometry/Common.hpp"

namespace krys::UI
{
  /// @brief Provides the data needed to generate a mesh for a given element's box.
  class RenderBox
  {
  private:
    Vec2 _fillSize;
    Vec2 _borderOffset;
    EdgeSizes _borderWidths;
    CornerSizes _borderRadii;

  public:
    RenderBox(Vec2 fillSize, Vec2 borderOffset, EdgeSizes borderWidths, CornerSizes borderRadii)
        : _fillSize(fillSize), _borderOffset(borderOffset), _borderWidths(borderWidths),
          _borderRadii(borderRadii)
    {
    }

    /// @brief Returns the size of the fill area of the box.
    KRYS_NODISCARD Vec2 GetFillSize() const noexcept
    {
      return _fillSize;
    }

    /// @brief Sets the size of the fill area of the box.
    void SetFillSize(Vec2 value) noexcept
    {
      _fillSize = value;
    }

    /// @brief Returns the offset from the border area to the fill area of the box.
    KRYS_NODISCARD Vec2 GetFillOffset() const noexcept
    {
      return {_borderWidths[3], _borderWidths[0]};
    }

    /// @brief Returns the offset to the border area of the box.
    KRYS_NODISCARD Vec2 GetBorderOffset() const noexcept
    {
      return _borderOffset;
    }

    /// @brief Sets the border offset.
    void SetBorderOffset(Vec2 value) noexcept
    {
      _borderOffset = value;
    }

    /// @brief Returns the border widths of the box.
    KRYS_NODISCARD const EdgeSizes &GetBorderWidths() const noexcept
    {
      return _borderWidths;
    }

    /// @brief Sets the border widths of the box.
    void SetBorderWidths(const EdgeSizes &value) noexcept
    {
      _borderWidths = value;
    }

    /// @brief Returns the border radius of the box.
    KRYS_NODISCARD const CornerSizes &GetBorderRadius() const noexcept
    {
      return _borderRadii;
    }

    /// Sets the border radius of the box.
    void SetBorderRadius(const CornerSizes &value) noexcept
    {
      _borderRadii = value;
    }

    bool operator==(const RenderBox &b) noexcept
    {
      return GetFillSize() == b.GetFillSize() && GetBorderOffset() == b.GetBorderOffset()
             && GetBorderWidths() == b.GetBorderWidths() && GetBorderRadius() == b.GetBorderRadius();
    }

    bool operator!=(const RenderBox &b) noexcept
    {
      return !(*this == b);
    }
  };
}