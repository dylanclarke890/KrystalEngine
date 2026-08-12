#pragma once

#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericRange.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericUnits.hpp"

namespace Krys::HTML
{
  // NOTE: `ResolvedValueType` only affects the type the CSS value gets resolved to. Unresolved CSS
  // primitive numeric types always use a `double` as its internal representation.

  /// @brief Default implementation of `PrimitiveNumericRaw` for numeric types with a unit specifier.
  template <CSSRange R, UnitEnum U, typename V>
  struct PrimitiveNumericRaw
  {
    using UnitType = U;
    using UnitTraits = UnitTraits<UnitType>;
    using ResolvedValueType = V;

    constexpr static auto Range = R;
    constexpr static auto Category = UnitTraits::Category;

    static_assert(UnitTraits::IsValidRangeForCategory(Range));

    UnitType Unit;
    double Value;

    /// @brief Allows initialization from unit value of the same unit type and a value.
    /// @example
    ///    AngleRaw<R> foo { AngleUnit::Deg, 0 };
    constexpr PrimitiveNumericRaw(UnitType unit, double value) : Unit {unit}, Value {value}
    {
    }

    /// @brief Allows initialization from a literal with the same unit type.
    /// @example
    ///    AngleRaw<R> foo { 0_css_deg };
    template <UnitType UnitValue>
    constexpr PrimitiveNumericRaw(ValueLiteral<UnitValue> literal)
        : Unit {literal.Unit}, Value {literal.Value}
    {
    }

    /// @brief Allows initialization from nested unit types if the type is composite.
    /// @example
    ///    AnglePercentageRaw<R> foo { AngleUnit::Deg, 0 };
    ///    AnglePercentageRaw<R> foo { PercentageUnit::Percentage, 0 };
    constexpr PrimitiveNumericRaw(NestedUnitEnumOf<UnitType> auto unit, double value)
        : Unit {UnitUpcast<UnitType>(unit)}, Value {value}
    {
    }

    /// @brief Allows initialization from raw value of one of the nested unit types if the type is composite.
    /// @example
    ///    AnglePercentageRaw<R> foo { AngleRaw<R> { ... } };
    ///    AnglePercentageRaw<R> foo { PercentageRaw<R> { ... } };
    template <typename T>
    requires NumericRaw<T> && NestedUnitEnumOf<typename T::UnitType, UnitType>
    constexpr PrimitiveNumericRaw(T other) : Unit {UnitUpcast<UnitType>(other.Unit)}, Value {other.Value}
    {
    }

    /// @brief Allows initialization from literal value of one of the nested unit types if the type is
    /// composite.
    /// @example
    ///    AnglePercentageRaw<R> foo { 0_css_deg };
    ///    AnglePercentageRaw<R> foo { 0_css_percentage };
    template <NestedUnitEnumOf<UnitType> E, E UnitValue>
    constexpr PrimitiveNumericRaw(ValueLiteral<UnitValue> literal)
        : Unit {UnitUpcast<UnitType>(literal.Unit)}, Value {literal.Value}
    {
    }

    constexpr bool operator==(const PrimitiveNumericRaw &) const = default;

    template <typename T>
    requires NumericRaw<T> && NestedUnitEnumOf<typename T::UnitType, UnitType>
    constexpr bool operator==(const T &other) const
    {
      return Unit == other.Unit && Value == other.Value;
    }

    template <UnitType UnitValue>
    constexpr bool operator==(const ValueLiteral<UnitValue> &other) const
    {
      return Unit == other.Unit && Value == other.Value;
    }

    template <NestedUnitEnumOf<UnitType> E, E UnitValue>
    constexpr bool operator==(const ValueLiteral<UnitValue> &other) const
    {
      return Unit == UnitUpcast<UnitType>(other.Unit) && Value == other.Value;
    }
  };

  /// @brief Specialization of `PrimitiveNumericRaw` for numeric types with only a single possible
  /// unit type (e.g. IntegerUnit, NumberUnit, PercentageUnit, FlexUnit).
  template <CSSRange R, SingleValueUnitEnum U, typename V>
  struct PrimitiveNumericRaw<R, U, V>
  {
    using ResolvedValueType = V;
    using UnitType = U;
    using UnitTraits = UnitTraits<UnitType>;

    constexpr static auto Range = R;
    constexpr static auto Category = UnitTraits::Category;

    static_assert(UnitTraits::IsValidRangeForCategory(Range));

    constexpr static auto Unit = UnitTraits::Canonical;
    double Value;

    template <typename T>
    requires Integral<T> || FloatingPoint<T>
    constexpr PrimitiveNumericRaw(T value) : Value {static_cast<double>(value)}
    {
    }

    /// @brief Constructor is required to allow generic code to uniformly initialize primitives.
    template <typename T>
    requires Integral<T> || FloatingPoint<T>
    constexpr PrimitiveNumericRaw(UnitType, T value) : Value {static_cast<double>(value)}
    {
    }

    template <auto UnitValue>
    requires SameType<decltype(UnitValue), UnitType>
    constexpr PrimitiveNumericRaw(ValueLiteral<UnitValue> literal) : Value {literal.Value}
    {
    }

    /// @brief Compares two `PrimitiveNumericRaw` objects for equality.
    constexpr bool operator==(const PrimitiveNumericRaw &) const = default;

    template <auto UnitValue>
    requires SameType<decltype(UnitValue), UnitType>
    constexpr bool operator==(const ValueLiteral<UnitValue> &literal) const
    {
      return Value == literal.Value;
    }
  };

#pragma region Integer Primitive Raw

  template <CSSRange R = All, typename V = int>
  struct IntegerRaw : PrimitiveNumericRaw<R, IntegerUnit, V>
  {
    using Base = PrimitiveNumericRaw<R, IntegerUnit, V>;
    using Base::Base;
  };

#pragma endregion

#pragma region Number Primitive Raw

  template <CSSRange R = All, typename V = double>
  struct NumberRaw : PrimitiveNumericRaw<R, NumberUnit, V>
  {
    using Base = PrimitiveNumericRaw<R, NumberUnit, V>;
    using Base::Base;
  };

#pragma endregion

#pragma region Percentage Primitive Raw

  template <CSSRange R = All, typename V = double>
  struct PercentageRaw : PrimitiveNumericRaw<R, PercentageUnit, V>
  {
    using Base = PrimitiveNumericRaw<R, PercentageUnit, V>;
    using Base::Base;
  };

#pragma endregion

#pragma region Dimension Primitives Raw

  template <CSSRange R = All, typename V = double>
  struct AngleRaw : PrimitiveNumericRaw<R, AngleUnit, V>
  {
    using Base = PrimitiveNumericRaw<R, AngleUnit, V>;
    using Base::Base;
  };

  template <CSSRange R = All, typename V = float>
  struct LengthRaw : PrimitiveNumericRaw<R, LengthUnit, V>
  {
    using Base = PrimitiveNumericRaw<R, LengthUnit, V>;
    using Base::Base;
  };

  template <CSSRange R = All, typename V = double>
  struct TimeRaw : PrimitiveNumericRaw<R, TimeUnit, V>
  {
    using Base = PrimitiveNumericRaw<R, TimeUnit, V>;
    using Base::Base;
  };

  template <CSSRange R = All, typename V = double>
  struct FrequencyRaw : PrimitiveNumericRaw<R, FrequencyUnit, V>
  {
    using Base = PrimitiveNumericRaw<R, FrequencyUnit, V>;
    using Base::Base;
  };

  template <CSSRange R = NonNegative, typename V = double>
  struct ResolutionRaw : PrimitiveNumericRaw<R, ResolutionUnit, V>
  {
    using Base = PrimitiveNumericRaw<R, ResolutionUnit, V>;
    using Base::Base;
  };

  template <CSSRange R = All, typename V = double>
  struct FlexRaw : PrimitiveNumericRaw<R, FlexUnit, V>
  {
    using Base = PrimitiveNumericRaw<R, FlexUnit, V>;
    using Base::Base;
  };

#pragma endregion

#pragma region Dimension + Percentage Primitives Raw

  template <CSSRange R = All, typename V = float>
  struct AnglePercentageRaw : PrimitiveNumericRaw<R, AnglePercentageUnit, V>
  {
    using Base = PrimitiveNumericRaw<R, AnglePercentageUnit, V>;
    using Base::Base;
  };

  template <CSSRange R = All, typename V = float>
  struct LengthPercentageRaw : PrimitiveNumericRaw<R, LengthPercentageUnit, V>
  {
    using Base = PrimitiveNumericRaw<R, LengthPercentageUnit, V>;
    using Base::Base;
  };

#pragma endregion
}