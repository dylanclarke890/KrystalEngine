#pragma once

#include "Krystal.Booey/CSS/Values/Motion/RayFunction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Position.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  using RaySize = css::RaySize;

  // ray() = ray( <angle> && <ray-size>? && contain? && [at <position>]? )
  // <ray-size> = closest-side | closest-corner | farthest-side | farthest-corner | sides
  // https://drafts.fxtf.org/motion-1/#ray-function
  struct Ray
  {
    style::Angle<> Angle;
    RaySize Size;
    Maybe<css::keywords::Contain> Contain;
    Maybe<style::Position> Position;

    KRYS_NODISCARD bool operator==(const Ray &) const noexcept = default;
  };
  using RayFunction = css::FunctionNotation<css::ValueId::Ray, Ray>;

  template <size_t I>
  const auto &get(const Ray &value)
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

  DEFINE_TYPE_MAPPING(css::Ray, Ray)

  // MARK: Serialization

  template <>
  struct Serialiser<Ray>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const Ray &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::Ray, 4)
