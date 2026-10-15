#pragma once

#include "Krystal.Booey/CSS/Values/Shapes/EllipseFunction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Position.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PathComputation.hpp"

namespace krys::boo::style
{
  struct Ellipse
  {
    using Extent = css::Ellipse::Extent;
    using Length = style::LengthPercentage<css::NonNegative>;
    using RadialSize = Variant<Length, Extent>;

    css::SpaceSeparatedPair<RadialSize> Radii;
    Maybe<style::Position> Position;

    KRYS_NODISCARD bool operator==(const Ellipse &) const noexcept = default;
  };
  using EllipseFunction = css::FunctionNotation<css::ValueId::Ellipse, Ellipse>;

  template <size_t I>
  const auto &get(const Ellipse &value)
  {
    if constexpr (!I)
    {
      return value.Radii;
    }
    else if constexpr (I == 1)
    {
      return value.Position;
    }
  }

  DEFINE_TYPE_MAPPING(css::Ellipse, Ellipse)

  KRYS_NODISCARD FloatPoint ResolvePosition(const Ellipse &value, FloatSize boxSize) noexcept;
  KRYS_NODISCARD FloatSize ResolveRadii(const Ellipse &, FloatSize boxSize, FloatPoint center) noexcept;
  KRYS_NODISCARD boo::Path PathForCenterCoordinate(const Ellipse &, const FloatRect &, FloatPoint) noexcept;

  template <>
  struct PathComputation<Ellipse>
  {
    KRYS_NODISCARD boo::Path operator()(const Ellipse &, const FloatRect &) noexcept;
  };

  template <>
  struct Blending<Ellipse>
  {
    KRYS_NODISCARD bool CanBlend(const Ellipse &, const Ellipse &) noexcept;
    KRYS_NODISCARD Ellipse Blend(const Ellipse &, const Ellipse &, const BlendingContext &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::Ellipse, 2)
