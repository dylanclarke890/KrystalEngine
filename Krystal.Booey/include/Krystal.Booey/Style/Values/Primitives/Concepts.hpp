#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Concepts.hpp"

namespace krys::boo::style
{
  // Forward declaration of PrimitiveNumeric to needed to create a hard constraint for the Numeric concept
  // below.
  template <css::Numeric>
  struct PrimitiveNumeric;

  // Concept for use in generic contexts to filter on all numeric Style types.
  template <typename T>
  concept Numeric = DerivedFrom<T, PrimitiveNumeric<typename T::CSS>>;

  // Concept for use in generic contexts to filter on non-composite numeric Style types.
  template <typename T>
  concept NonCompositeNumeric = Numeric<T> && css::NonCompositeNumeric<typename T::CSS>;

  // Concept for use in generic contexts to filter on dimension-percentage numeric Style types.
  template <typename T>
  concept DimensionPercentageNumeric =
    Numeric<T> && css::VariantLike<T> && css::DimensionPercentageNumeric<typename T::CSS>;

  // Forward declaration of UnevaluatedCalc to needed to create a hard constraint for the Calc
  // concept below.
  template <css::Numeric>
  struct UnevaluatedCalc;

  // Concept for use in generic contexts to filter on UnevaluatedCalc Style types.
  template <typename T>
  concept Calc = SameType<T, UnevaluatedCalc<typename T::CSS>>;
}