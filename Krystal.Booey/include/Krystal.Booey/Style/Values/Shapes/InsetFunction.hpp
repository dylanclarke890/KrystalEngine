#pragma once

#include "Krystal.Booey/CSS/Values/Shapes/InsetFunction.hpp"
#include "Krystal.Booey/Style/Values/Borders/BorderRadius.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PathComputation.hpp"

namespace krys::boo
{
  class Path;

  namespace style
  {
    struct Inset
    {
      using InsetsType = css::MinimallySerialisingSpaceSeparatedRectEdges<LengthPercentage<>>;

      InsetsType Insets;
      BorderRadius Radii;

      KRYS_NODISCARD bool operator==(const Inset &) const noexcept = default;
    };
    using InsetFunction = css::FunctionNotation<css::ValueId::Inset, Inset>;

    template <size_t I>
    const auto &get(const Inset &value)
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

    DEFINE_TYPE_MAPPING(css::Inset, Inset)

    template <>
    struct PathComputation<Inset>
    {
      KRYS_NODISCARD boo::Path operator()(const Inset &, const FloatRect &) noexcept;
    };

  }
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::Inset, 2)
