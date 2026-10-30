#pragma once

#include "Krystal.Booey/CSS/Values/Shapes/CircleFunction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Position.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PathComputation.hpp"

namespace krys::boo::style
{
  struct Circle
  {
    using Extent = css::Circle::Extent;
    using Length = style::LengthPercentage<css::NonNegative>;
    using RadialSize = Variant<Length, Extent>;

    RadialSize Radius;
    Maybe<style::Position> Position;

    KRYS_NODISCARD bool operator==(const Circle &) const noexcept = default;
  };
  using CircleFunction = css::FunctionNotation<css::ValueId::Circle, Circle>;

  template <size_t I>
  const auto &get(const Circle &value)
  {
    if constexpr (!I)
    {
      return value.Radius;
    }
    else if constexpr (I == 1)
    {
      return value.Position;
    }
  }

  DEFINE_TYPE_MAPPING(css::Circle, Circle)

  KRYS_NODISCARD FloatPoint ResolvePosition(const Circle &value, FloatSize boxSize) noexcept;
  KRYS_NODISCARD float ResolveRadius(const Circle &value, FloatSize boxSize, FloatPoint center) noexcept;
  KRYS_NODISCARD boo::Path PathForCenterCoordinate(const Circle &, const FloatRect &, FloatPoint) noexcept;

  template <>
  struct PathComputation<Circle>
  {
    KRYS_NODISCARD boo::Path operator()(const Circle &, const FloatRect &) noexcept;
  };

  template <>
  struct Blending<Circle>
  {
    KRYS_NODISCARD bool CanBlend(const Circle &, const Circle &) noexcept;
    KRYS_NODISCARD Circle Blend(const Circle &, const Circle &, const BlendingContext &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::Circle, 2)
