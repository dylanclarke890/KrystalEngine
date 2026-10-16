#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Position.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  using RaySize = Variant<keywords::ClosestCorner, keywords::ClosestSide, keywords::FarthestCorner,
                          keywords::FarthestSide, keywords::Sides>;

  // ray() = ray( <angle> && <ray-size>? && contain? && [at <position>]? )
  // <ray-size> = closest-side | closest-corner | farthest-side | farthest-corner | sides
  // https://drafts.fxtf.org/motion-1/#ray-function
  struct Ray
  {
    boo::css::Angle<> Angle;
    RaySize Size;
    Maybe<keywords::Contain> Contain;
    Maybe<boo::css::Position> Position;

    KRYS_NODISCARD bool operator==(const Ray &) const noexcept = default;
  };

  using RayFunction = FunctionNotation<ValueId::Ray, Ray>;

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Ray &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Angle;
    }
    else if constexpr (I == 1)
    {
      return value.Size;
    }
    else if constexpr (I == 2)
    {
      return value.Contain;
    }
    else if constexpr (I == 3)
    {
      return value.Position;
    }
  }

  template <>
  struct Serialiser<Ray>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context, const Ray &value) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Ray, 4)
