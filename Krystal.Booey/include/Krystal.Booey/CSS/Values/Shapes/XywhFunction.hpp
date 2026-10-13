#pragma once

#include "Krystal.Booey/CSS/Values/Borders/BorderRadius.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::css
{
  // <xywh()> = xywh( <length-percentage>{2} <length-percentage [0,∞]>{2} [ round <'border-radius'> ]? )
  // https://drafts.csswg.org/css-shapes-1/#funcdef-basic-shape-xywh
  struct Xywh
  {
    using LocationType = SpaceSeparatedPoint<LengthPercentage<>>;
    using SizeType = SpaceSeparatedSize<LengthPercentage<NonNegative>>;

    LocationType Location;
    SizeType Size;
    BorderRadius Radii;

    KRYS_NODISCARD bool operator==(const Xywh &) const noexcept = default;
  };

  using XywhFunction = FunctionNotation<ValueId::Xywh, Xywh>;

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Xywh &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Location;
    }
    else if constexpr (I == 1)
    {
      return value.Size;
    }
    else if constexpr (I == 2)
    {
      return value.Radii;
    }
  }

  template <>
  struct Serialiser<Xywh>
  {
    void operator()(CSSOMString &builder, const SerialisationContext &context, const Xywh &value) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Xywh, 3)
