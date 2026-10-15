#pragma once

#include "Krystal.Booey/CSS/Values/Shapes/PolygonFunction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Shapes/FillRule.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PathComputation.hpp"
#include "Krystal.Booey/Style/Values/Shapes/WindRuleComputation.hpp"

namespace krys::boo::style
{
  struct Polygon
  {
    using Vertex = css::SpaceSeparatedPoint<LengthPercentage<>>;
    using VerticesType = css::CommaSeparatedList<Vertex>;

    // FIXME: Add support the "round" clause.

    Maybe<style::FillRule> FillRule;
    VerticesType Vertices;

    KRYS_NODISCARD bool operator==(const Polygon &) const noexcept = default;
  };
  using PolygonFunction = css::FunctionNotation<css::ValueId::Polygon, Polygon>;

  template <size_t I>
  const auto &get(const Polygon &value)
  {
    if constexpr (!I)
    {
      return value.FillRule;
    }
    else if constexpr (I == 1)
    {
      return value.Vertices;
    }
  }

  DEFINE_TYPE_MAPPING(css::Polygon, Polygon)

  template <>
  struct PathComputation<Polygon>
  {
    KRYS_NODISCARD boo::Path operator()(const Polygon &, const FloatRect &) noexcept;
  };
  template <>
  struct WindRuleComputation<Polygon>
  {
    KRYS_NODISCARD boo::WindRule operator()(const Polygon &) noexcept;
  };

  template <>
  struct Blending<Polygon>
  {
    KRYS_NODISCARD bool CanBlend(const Polygon &, const Polygon &) noexcept;
    KRYS_NODISCARD Polygon Blend(const Polygon &, const Polygon &, const BlendingContext &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::Polygon, 2)
