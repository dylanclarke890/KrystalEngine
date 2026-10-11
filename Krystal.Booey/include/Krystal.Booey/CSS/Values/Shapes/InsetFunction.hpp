#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Position.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/CSS/Values/Aggregates.hpp"

namespace krys::boo::css
{
    // <inset()> = inset( <length-percentage>{1,4} [ round <'border-radius'> ]? )
    // https://drafts.csswg.org/css-shapes-1/#funcdef-basic-shape-inset
    struct Inset
    {
      using Insets = MinimallySerializingSpaceSeparatedRectEdges<LengthPercentage<>>;

      Insets Insets;
      BorderRadius Radii;

      bool operator==(const Inset &) const noexcept = default;
    };

    using InsetFunction = FunctionNotation<ValueId::Inset, Inset>;

    template <size_t I>
    KRYS_NODISCARD const auto &get(const Inset &value) noexcept
    {
      if constexpr (!I)
      {
        return value.Insets;
      }
      else if constexpr (I == 1)
      {
        return value.Radii;
      }
    }

    template <>
    struct Serialiser<Inset>
    {
      void operator()(CSSOMString &builder, const SerialisationContext &context, const Inset &value) noexcept;
    };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Inset, 2)
