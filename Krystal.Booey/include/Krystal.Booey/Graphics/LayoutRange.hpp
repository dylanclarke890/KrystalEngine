#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Core/Attributes.hpp"
#include <algorithm>

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief A one-dimensional `LayoutRect`: a location and a size along one axis.
  class LayoutRange
  {
  private:
    LayoutUnit _location;
    LayoutUnit _size;

  public:
    constexpr LayoutRange() noexcept = default;
    constexpr LayoutRange(LayoutUnit location, LayoutUnit size) noexcept : _location(location), _size(size)
    {
    }

    KRYS_NODISCARD constexpr LayoutUnit Min() const noexcept
    {
      return _location;
    }
    KRYS_NODISCARD constexpr LayoutUnit Max() const noexcept
    {
      return _location + _size;
    }
    KRYS_NODISCARD constexpr LayoutUnit Size() const noexcept
    {
      return _size;
    }

    constexpr void Set(LayoutUnit location, LayoutUnit size) noexcept
    {
      _location = location;
      _size = size;
    }
    constexpr void Reset(LayoutUnit size = 0_lu) noexcept
    {
      _location = 0_lu;
      _size = size;
    }

    constexpr void MoveBy(LayoutUnit shift) noexcept
    {
      _location += shift;
    }
    constexpr void MoveTo(LayoutUnit target) noexcept
    {
      _location = target;
    }

    constexpr void ShiftMinEdgeBy(LayoutUnit shift) noexcept
    {
      _location += shift;
      _size -= shift;
    }
    constexpr void ShiftMaxEdgeBy(LayoutUnit shift) noexcept
    {
      _size += shift;
    }
    constexpr void ShiftMinEdgeTo(LayoutUnit target) noexcept
    {
      ShiftMinEdgeBy(target - Min());
    }
    constexpr void ShiftMaxEdgeTo(LayoutUnit target) noexcept
    {
      ShiftMaxEdgeBy(target - Max());
    }

    constexpr void SizeFromMinEdge(LayoutUnit size = 0_lu) noexcept
    {
      _size = size;
    }
    constexpr void SizeFromMaxEdge(LayoutUnit size = 0_lu) noexcept
    {
      _location -= size - _size;
      _size = size;
    }

    /// @brief Moves the min edge up to @p target, if it's below it.
    constexpr void FloorMinEdgeTo(LayoutUnit target) noexcept
    {
      if (target > Min())
      {
        ShiftMinEdgeTo(target);
      }
    }
    /// @brief Moves the max edge up to @p target, if it's below it.
    constexpr void FloorMaxEdgeTo(LayoutUnit target) noexcept
    {
      if (target > Max())
      {
        ShiftMaxEdgeTo(target);
      }
    }
    /// @brief Moves the min edge down to @p target, if it's above it.
    constexpr void CapMinEdgeTo(LayoutUnit target) noexcept
    {
      if (target < Min())
      {
        ShiftMinEdgeTo(target);
      }
    }
    /// @brief Moves the max edge down to @p target, if it's above it.
    constexpr void CapMaxEdgeTo(LayoutUnit target) noexcept
    {
      if (target < Max())
      {
        ShiftMaxEdgeTo(target);
      }
    }

    constexpr void FloorSizeFromMinEdge(LayoutUnit size = 0_lu) noexcept
    {
      _size = std::max(_size, size);
    }
    constexpr void FloorSizeFromMaxEdge(LayoutUnit size = 0_lu) noexcept
    {
      if (size > _size)
      {
        SizeFromMaxEdge(size);
      }
    }

    KRYS_NODISCARD constexpr bool operator==(const LayoutRange &) const noexcept = default;
  };

  TextStream &operator<<(TextStream &ts, const LayoutRange &range) noexcept;
}
