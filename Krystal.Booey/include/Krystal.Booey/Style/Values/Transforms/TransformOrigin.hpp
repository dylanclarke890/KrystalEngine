#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Position.hpp"

namespace krys::boo::style
{
  using TransformOriginX = PositionX;
  using TransformOriginY = PositionY;
  using TransformOriginZ = Length<>;
  using TransformOriginXY = Position;

  // https://www.w3.org/TR/css-transforms-1/#propdef-transform-origin
  struct TransformOrigin
  {
    TransformOriginX X;
    TransformOriginY Y;
    TransformOriginZ Z;

    KRYS_NODISCARD TransformOriginXY XY() const noexcept
    {
      return {X, Y};
    }

    KRYS_NODISCARD bool operator==(const TransformOrigin &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const TransformOrigin &value) noexcept
  {
    if constexpr (!I)
    {
      return value.X;
    }
    else if constexpr (I == 1)
    {
      return value.Y;
    }
    else if constexpr (I == 2)
    {
      return value.Z;
    }
  }
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::TransformOrigin, 3)
