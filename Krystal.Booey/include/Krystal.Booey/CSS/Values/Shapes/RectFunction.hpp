#pragma once

#include "Krystal.Booey/Core/RectEdges.hpp"
#include "Krystal.Booey/CSS/Values/Borders/BorderRadius.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  // <rect()> = rect( [ <length-percentage> | auto ]{4} [ round <'border-radius'> ]? )
  // https://drafts.csswg.org/css-shapes-1/#funcdef-basic-shape-rect
  struct RectShape
  {
    using Edge = Variant<LengthPercentage<>, keywords::Auto>;

    SpaceSeparatedRectEdges<Edge> Edges;
    BorderRadius Radii;

    KRYS_NODISCARD bool operator==(const RectShape &) const noexcept = default;
  };

  using RectFunction = FunctionNotation<ValueId::Rect, RectShape>;

  template <size_t I>
  KRYS_NODISCARD const auto &get(const RectShape &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Edges;
    }
    else if constexpr (I == 1)
    {
      return value.Radii;
    }
  }

  template <>
  struct Serialiser<RectShape>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const RectShape &value) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::RectShape, 2)
