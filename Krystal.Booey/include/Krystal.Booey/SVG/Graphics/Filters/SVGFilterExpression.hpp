#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterEffectGeometry.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief A step of a filter expression: an effect (by its index in the filter's effects), its depth in the
  /// filter graph, and its subregion.
  struct SVGFilterExpressionTerm
  {
    uint32 Index {0};
    uint32 Level {0};
    Maybe<FilterEffectGeometry> Geometry;
  };

  /// @brief A filter graph in postfix order: each effect follows its inputs.
  using SVGFilterExpression = List<SVGFilterExpressionTerm>;
}
