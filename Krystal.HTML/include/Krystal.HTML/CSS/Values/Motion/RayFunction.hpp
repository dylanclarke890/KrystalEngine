#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPosition.hpp"

namespace Krys::HTML
{
  using RaySize = Variant<Keywords::ClosestCorner, Keywords::ClosestSide, Keywords::FarthestCorner,
                          Keywords::FarthestSide, Keywords::Sides>;

  // ray() = ray( <angle> && <ray-size>? && contain? && [at <position>]? )
  // <ray-size> = closest-side | closest-corner | farthest-side | farthest-corner | sides
  // https://drafts.fxtf.org/motion-1/#ray-function
  struct Ray
  {
    Angle<> angle;
    RaySize size;
    Maybe<Keywords::Contain> contain;
    Maybe<CSSPosition> position;

    KRYS_NODISCARD bool operator==(const Ray &) const noexcept = default;
  };

  using RayFunction = FunctionNotation<CSSValueId::Ray, Ray>;

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Ray &value) noexcept
  {
    if constexpr (!I)
    {
      return value.angle;
    }
    else if constexpr (I == 1)
    {
      return value.size;
    }
    else if constexpr (I == 2)
    {
      return value.contain;
    }
    else if constexpr (I == 3)
    {
      return value.position;
    }
  }
}

DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::Ray, 4)
