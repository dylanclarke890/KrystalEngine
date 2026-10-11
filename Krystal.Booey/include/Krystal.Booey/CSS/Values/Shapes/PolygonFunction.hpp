#pragma once

#include "Krystal.Booey/CSS/Values/Shapes/FillRule.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  // <polygon()> = polygon( <'fill-rule'>? [ round <length> ]? , [<length-percentage> <length-percentage>]#
  // ) https://drafts.csswg.org/css-shapes-1/#funcdef-basic-shape-polygon
  // FIXME: Add support the "round" clause.
  struct Polygon
  {
    using Vertex = SpaceSeparatedPoint<LengthPercentage<>>;
    using Vertices = CommaSeparatedList<Vertex>;

    Maybe<FillRule> FillRule;
    Vertices Vertices;

    bool operator==(const Polygon &) const noexcept = default;
  };

  using PolygonFunction = FunctionNotation<ValueId::Polygon, Polygon>;

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Polygon &value) noexcept
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

  template <>
  struct Serialiser<Polygon>
  {
    void operator()(CSSOMString &builder, const SerialisationContext &context, const Polygon &value) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Polygon, 2)
