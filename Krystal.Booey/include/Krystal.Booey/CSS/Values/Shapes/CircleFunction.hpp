#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Position.hpp"

namespace krys::boo::css
{
  // <circle()> = circle( <radial-size>? [ at <position> ]? )
  // https://drafts.csswg.org/css-shapes-1/#funcdef-basic-shape-circle
  struct Circle
  {
    using Extent = Variant<keywords::ClosestCorner, keywords::ClosestSide, keywords::FarthestCorner,
                           keywords::FarthestSide>;
    // FIXME: The spec says that this should take Length, not a LengthPercentage, but this does not match
    // the tests.
    using Length = css::LengthPercentage<NonNegative>;
    using RadialSize = Variant<Length, Extent>;

    RadialSize Radius;
    Maybe<boo::css::Position> Position;

    KRYS_NODISCARD bool operator==(const Circle &) const noexcept = default;
  };

  using CircleFunction = FunctionNotation<ValueId::Circle, Circle>;

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Circle &value) noexcept
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

  template <>
  struct Serialiser<Circle>
  {
    void operator()(CSSOMString &builder, const SerialisationContext &context, const Circle &value) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Circle, 2)
