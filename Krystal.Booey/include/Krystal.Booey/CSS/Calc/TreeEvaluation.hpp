#pragma once

#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class SymbolTable;

  enum class Category : uint8;
}

namespace krys::boo::css::calc
{
  struct Anchor;
  struct Tree;

  struct EvaluationOptions
  {
    /// @brief The context in which the evaluation is taking place.
    css::Category Category;

    /// @brief The allowed numeric range for the calculated result.
    css::Range Range;

    /// @brief The information needed to convert units into their canonical forms.
    Maybe<ToLengthConversionData> ConversionData;

    /// @brief Contains information needed to convert unresolved symbols into numeric values.
    const css::SymbolTable &SymbolTable;
  };

  KRYS_NODISCARD Maybe<double> EvaluateDouble(const Tree &tree, const EvaluationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<double> EvaluateWithoutFallback(const Anchor &anchor,
                                                       const EvaluationOptions &options) noexcept;
}