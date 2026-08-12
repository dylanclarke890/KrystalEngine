#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueConcepts.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericRange.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericUnits.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"

namespace Krys::HTML
{
  // Forward declaration of PrimitiveNumericRaw needed to create a hard constraint for the NumericRaw
  // concept below.
  template <CSSRange, UnitEnum, typename>
  struct PrimitiveNumericRaw;

  /// @brief Concept for use in generic contexts to filter on raw numeric CSS types.
  template <typename T>
  concept NumericRaw =
    DerivedFrom<T, PrimitiveNumericRaw<T::Range, typename T::UnitType, typename T::ResolvedValueType>>;

  // Forward declaration of PrimitiveNumeric to needed to create a hard constraint for the Numeric concept
  // below.
  template <NumericRaw>
  struct PrimitiveNumeric;

  /// @brief Concept for use in generic contexts to filter on all numeric CSS types.
  template <typename T>
  concept Numeric = VariantLike<T> && std::derived_from<T, PrimitiveNumeric<typename T::Raw>>;

  /// @brief Concept for use in generic contexts to filter on non-composite numeric CSS types.
  template <typename T>
  concept NonCompositeNumeric = Numeric<T> && (!CompositeUnitEnum<typename T::UnitType>);

  /// @brief Concept for use in generic contexts to filter on dimension-percentage numeric CSS types.
  template <typename T>
  concept DimensionPercentageNumeric = Numeric<T> && CompositeUnitEnum<typename T::UnitType>;

  // Forward declaration of UnevaluatedCalc to needed to create a hard constraint for the Calc concept below.
  template <NumericRaw>
  struct UnevaluatedCalc;

  /// @brief Concept for use in generic contexts to filter on UnevaluatedCalc CSS types.
  template <typename T>
  concept Calc = SameType<T, UnevaluatedCalc<typename T::Raw>>;
}