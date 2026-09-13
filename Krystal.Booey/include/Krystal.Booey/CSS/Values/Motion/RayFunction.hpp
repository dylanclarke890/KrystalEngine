#pragma once

#include "Krystal.Booey/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/CSSPosition.hpp"

namespace krys::boo::css
{
  using RaySize = Variant<keywords::ClosestCorner, keywords::ClosestSide, keywords::FarthestCorner,
                          keywords::FarthestSide, keywords::Sides>;

  // ray() = ray( <angle> && <ray-size>? && contain? && [at <position>]? )
  // <ray-size> = closest-side | closest-corner | farthest-side | farthest-corner | sides
  // https://drafts.fxtf.org/motion-1/#ray-function
  struct Ray
  {
    Angle<> angle;
    RaySize size;
    Maybe<keywords::Contain> contain;
    Maybe<CSSPosition> position;

    KRYS_NODISCARD bool operator==(const Ray &) const noexcept = default;
  };

  using RayFunction = FunctionNotation<ValueId::Ray, Ray>;

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

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Ray, 4)
