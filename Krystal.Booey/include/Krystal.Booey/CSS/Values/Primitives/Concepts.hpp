#pragma once

#include "Krystal.Booey/CSS/Values/Concepts.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <tuple>
#include <type_traits>

namespace krys::boo::css
{
  enum class UnitType : uint8;
  enum class Category : uint8;

  template <typename>
  struct UnitTraits;

  /// @brief Concept covering all unit types.
  template <typename T>
  concept UnitEnum = IsEnum<T> && requires {
    requires Integral<decltype(UnitTraits<T>::Count)>;
    requires SameType<decltype(UnitTraits<T>::Canonical), const T>;
    requires SameType<decltype(UnitTraits<T>::Category), const css::Category>;
    { UnitTraits<T>::Validate(std::declval<UnitType>()) } -> SameType<Maybe<T>>;
  };

  /// @brief Concept covering unit types where the enumeration contains only a single value.
  /// e.g. IntegerUnit, NumberUnit, PercentageUnit, FlexUnit
  template <typename T>
  concept SingleValueUnitEnum = UnitEnum<T> && requires { requires(UnitTraits<T>::Count == 1); };

  /// @brief Concept covering unit types where the type is a composite of multiple other unit types.
  /// e.g. AnglePercentageUnit, LengthPercentageUnit
  template <typename T>
  concept CompositeUnitEnum = UnitEnum<T> && requires { typename UnitTraits<T>::Composite; };

  namespace detail
  {
    template <typename T, typename Tuple>
    struct ContainsInTuple : std::false_type
    {
    };

    template <typename T, typename... Ts>
    struct ContainsInTuple<T, std::tuple<Ts...>> : std::bool_constant<(SameType<T, Ts> || ...)>
    {
    };
  }

  /// @brief Concept to check if a type, `T`, is one of the types `CompositeParent` composes over.
  /// e.g. `NestedUnitEnumOf<LengthUnit, LengthPercentageUnit> == true`
  template <typename T, typename CompositeParent>
  concept NestedUnitEnumOf =
    UnitEnum<T> && CompositeUnitEnum<CompositeParent>
    && detail::ContainsInTuple<T, typename UnitTraits<CompositeParent>::Composite>::value;

  // Forward declaration of PrimitiveNumericRaw needed to create a hard constraint for the NumericRaw
  // concept below.
  template <Range, UnitEnum, typename>
  struct PrimitiveNumericRaw;

  /// @brief Concept for use in generic contexts to filter on raw numeric CSS types.
  template <typename T>
  concept NumericRaw =
    DerivedFrom<T, PrimitiveNumericRaw<T::Range, typename T::UnitType, typename T::ResolvedValueType>>;

  // Forward declaration of PrimitiveNumeric needed to create a hard constraint for the Numeric concept
  // below.
  template <NumericRaw>
  struct PrimitiveNumeric;

  /// @brief Concept for use in generic contexts to filter on all numeric CSS types.
  template <typename T>
  concept Numeric = VariantLike<T> && DerivedFrom<T, PrimitiveNumeric<typename T::Raw>>;

  /// @brief Concept for use in generic contexts to filter on non-composite numeric CSS types.
  template <typename T>
  concept NonCompositeNumeric = Numeric<T> && (!CompositeUnitEnum<typename T::UnitType>);

  /// @brief Concept for use in generic contexts to filter on dimension-percentage numeric CSS types.
  template <typename T>
  concept DimensionPercentageNumeric = Numeric<T> && CompositeUnitEnum<typename T::UnitType>;

  // Forward declaration of UnevaluatedCalc needed to create a hard constraint for the Calc concept below.
  template <NumericRaw>
  struct UnevaluatedCalc;

  /// @brief Concept for use in generic contexts to filter on UnevaluatedCalc CSS types.
  template <typename T>
  concept Calc = SameType<T, UnevaluatedCalc<typename T::Raw>>;
}
