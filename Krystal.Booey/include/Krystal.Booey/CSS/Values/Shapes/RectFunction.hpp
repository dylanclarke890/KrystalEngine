#pragma once

#include "Krystal.Booey/Core/RectEdges.hpp"
#include "Krystal.Booey/CSS/Values/Borders/BorderRadius.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  /// @brief <rect()> = rect( [ <length-percentage> | auto ]{4} [ round <'border-radius'> ]? )
  /// @see https://drafts.csswg.org/css-shapes-1/#funcdef-basic-shape-rect
  struct Rect
  {
    using Edge = Variant<LengthPercentage<>, keywords::Auto>;

    SpaceSeparatedRectEdges<Edge> Edges;
    BorderRadius Radii;

    bool operator==(const Rect &) const = default;
  };

  using RectFunction = FunctionNotation<ValueId::Rect, Rect>;

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Rect &value) noexcept
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
  struct Serialiser<Rect>
  {
    void operator()(CSSOMString &builder, const SerialisationContext &context, const Rect &value) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Rect, 2)
