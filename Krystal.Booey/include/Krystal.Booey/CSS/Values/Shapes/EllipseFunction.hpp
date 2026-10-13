#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Position.hpp"

namespace krys::boo::css
{
  // <ellipse()> = ellipse( <radial-size>? [ at <position> ]? )
  // https://drafts.csswg.org/css-shapes-1/#funcdef-basic-shape-ellipse
  struct Ellipse
  {
    using Extent = Variant<keywords::ClosestCorner, keywords::ClosestSide, keywords::FarthestCorner,
                           keywords::FarthestSide>;
    using Length = css::LengthPercentage<NonNegative>;
    using RadialSize = Variant<Length, Extent>;

    // FIXME: The spec says that this should take only a single RadialSize, not a pair, but this does not
    // match the tests.
    SpaceSeparatedPair<RadialSize> Radii;
    Maybe<boo::css::Position> Position;

    KRYS_NODISCARD bool operator==(const Ellipse &) const noexcept = default;
  };

  using EllipseFunction = FunctionNotation<ValueId::Ellipse, Ellipse>;

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Ellipse &value) noexcept
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

  template <>
  struct Serialiser<Ellipse>
  {
    void operator()(CSSOMString &builder, const SerialisationContext &context, const Ellipse &value) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Ellipse, 2)
