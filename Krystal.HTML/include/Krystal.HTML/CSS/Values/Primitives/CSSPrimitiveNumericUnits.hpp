#pragma once

#include "Krystal.HTML/CSS/Values/CSSUnits.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSUnitType.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericRange.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Core/Visitor.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include <cassert>
#include <numbers>

namespace Krys::HTML
{
  // https://www.w3.org/TR/css-values-4/#numeric-types
  enum class NumericTypeCategory : uint8_t
  {
    Integer,
    Number,
    Percentage,
    Length,
    Angle,
    Time,
    Frequency,
    Resolution,
    Flex,
    LengthPercentage,
    AnglePercentage,
  };

  template <typename>
  struct UnitTraits;

  template <typename T>
  concept UnitEnum = IsEnum<T> && requires {
    requires Integral<decltype(UnitTraits<T>::Count)>;
    requires SameType<decltype(UnitTraits<T>::Canonical), const T>;
    requires SameType<decltype(UnitTraits<T>::Category), const NumericTypeCategory>;
    { UnitTraits<T>::Validate(std::declval<CSSUnitType>()) } -> SameType<Maybe<T>>;
  };

  /// @brief Unit types where the type is a single value unit type. e.g. LengthUnit, AngleUnit
  template <typename T>
  concept SingleValueUnitEnum = UnitEnum<T> && requires { requires(UnitTraits<T>::Count == 1); };

  /// @brief Unit types where the type is a composite of multiple other unit types. e.g. LengthPercentageUnit
  template <typename T>
  concept CompositeUnitEnum = UnitEnum<T> && requires { typename UnitTraits<T>::Composite; };

  // Base template: handles cases where the second argument is not a tuple
  template <typename T, typename Tuple>
  struct ContainsInTuple : std::false_type
  {
  };

  // Partial specialization: unpacks the tuple types and uses a fold expression
  template <typename T, typename... Ts>
  struct ContainsInTuple<T, std::tuple<Ts...>> : std::bool_constant<(std::is_same_v<T, Ts> || ...)>
  {
  };

  template <typename T, typename CompositeParent>
  concept NestedUnitEnumOf = UnitEnum<T> && CompositeUnitEnum<CompositeParent>
                             && ContainsInTuple<T, typename UnitTraits<CompositeParent>::Composite>::value;

  template <auto unitValue>
  requires UnitEnum<decltype(unitValue)>
  struct ValueLiteral
  {
    using UnitType = decltype(unitValue);

    constexpr static UnitType Unit = unitValue;
    double Value;

    constexpr explicit ValueLiteral(double initialValue) : Value {initialValue}
    {
    }

    // Synthesize all comparison and equality operators.

    constexpr auto operator<=>(const ValueLiteral &) const = default;

    // Support unary operators.

    constexpr ValueLiteral operator+()
    {
      return ValueLiteral {Value};
    }

    constexpr ValueLiteral operator-()
    {
      return ValueLiteral {-Value};
    }

    // Support addition between `ValueLiteral` and machine numeric types.

    constexpr ValueLiteral &operator+=(const ValueLiteral &rhs)
    {
      Value += rhs.Value;
      return *this;
    }

    constexpr ValueLiteral &operator+=(ConvertibleTo<double> auto const &rhs)
    {
      Value += static_cast<double>(rhs);
      return *this;
    }

    friend constexpr ValueLiteral operator+(const ValueLiteral &lhs, const ValueLiteral &rhs)
    {
      return ValueLiteral {lhs.Value + rhs.Value};
    }

    friend constexpr ValueLiteral operator+(const ValueLiteral &lhs, ConvertibleTo<double> auto const &rhs)
    {
      return ValueLiteral {lhs.Value + static_cast<double>(rhs)};
    }

    friend constexpr ValueLiteral operator+(ConvertibleTo<double> auto const &lhs, const ValueLiteral &rhs)
    {
      return ValueLiteral {static_cast<double>(lhs) + rhs.Value};
    }

    // Support subtraction between `ValueLiteral` and machine numeric types.

    constexpr ValueLiteral &operator-=(const ValueLiteral &rhs)
    {
      Value -= rhs.Value;
      return *this;
    }

    constexpr ValueLiteral &operator-=(std::convertible_to<double> auto const &rhs)
    {
      Value -= static_cast<double>(rhs);
      return *this;
    }

    friend constexpr ValueLiteral operator-(const ValueLiteral &lhs, const ValueLiteral &rhs)
    {
      return ValueLiteral {lhs.Value - rhs.Value};
    }

    friend constexpr ValueLiteral operator-(const ValueLiteral &lhs, ConvertibleTo<double> auto const &rhs)
    {
      return ValueLiteral {lhs.Value - static_cast<double>(rhs)};
    }

    friend constexpr ValueLiteral operator-(ConvertibleTo<double> auto const &lhs, const ValueLiteral &rhs)
    {
      return ValueLiteral {static_cast<double>(lhs) - rhs.Value};
    }

    // Support multiplication between `ValueLiteral` and machine numeric types.

    constexpr ValueLiteral &operator*=(ConvertibleTo<double> auto const &rhs)
    {
      Value *= static_cast<double>(rhs);
      return *this;
    }

    friend constexpr ValueLiteral operator*(const ValueLiteral &lhs, ConvertibleTo<double> auto const &rhs)
    {
      return ValueLiteral {lhs.Value * static_cast<double>(rhs)};
    }

    friend constexpr ValueLiteral operator*(ConvertibleTo<double> auto const &lhs, const ValueLiteral &rhs)
    {
      return ValueLiteral {static_cast<double>(lhs) * rhs.Value};
    }

    // Support division between `ValueLiteral` and machine numeric types.

    constexpr ValueLiteral &operator/=(ConvertibleTo<double> auto const &rhs)
    {
      Value /= static_cast<double>(rhs);
      return *this;
    }

    friend constexpr ValueLiteral operator/(const ValueLiteral &lhs, ConvertibleTo<double> auto const &rhs)
    {
      return ValueLiteral {lhs.Value / static_cast<double>(rhs)};
    }
  };

  /// @brief Checks if casting `other` to unit type `T` is a valid cast.
  template <UnitEnum T, UnitEnum U>
  KRYS_NODISCARD constexpr bool IsUnit(U other) noexcept
  {
    if constexpr (SameType<T, U>)
    {
      return true;
    }
    else if constexpr (NestedUnitEnumOf<U, T>)
    {
      return true;
    }
    else if constexpr (NestedUnitEnumOf<T, U>)
    {
      return UnitTraits<U>::template Is<T>(other);
    }
  }

  /// @brief Allows identity casts, upcasts or downcasts of a unit type.
  template <UnitEnum T, UnitEnum U>
  KRYS_NODISCARD constexpr T UnitCast(U other) noexcept
  {
    if constexpr (SameType<T, U>)
    {
      return other;
    }
    else if constexpr (NestedUnitEnumOf<U, T>)
    {
      return UnitTraits<T>::Upcast(other);
    }
    else if constexpr (NestedUnitEnumOf<T, U>)
    {
      assert(IsUnit<T>(other));
      return UnitTraits<U>::template Downcast<T>(other);
    }
  }

  /// @brief Allows casting UP from a unit type to a composite unit type that includes it.
  /// `AnglePercentageUnit::Deg` == UnitUpcast<AnglePercentageUnit>(AngleUnit::Deg)
  template <UnitEnum T, UnitEnum U>
  KRYS_NODISCARD constexpr Maybe<T> DynamicUnitCast(U other) noexcept
  {
    if constexpr (SameType<T, U>)
    {
      return other;
    }
    else if constexpr (NestedUnitEnumOf<U, T>)
    {
      return UnitTraits<T>::Upcast(other);
    }
    else if constexpr (NestedUnitEnumOf<T, U>)
    {
      if (!IsUnit<T>(other))
      {
        return {};
      }
      return UnitTraits<U>::template Downcast<T>(other);
    }
  }

  /// @brief Allows casting UP from a unit type to a composite unit type that includes it.
  /// `AnglePercentageUnit::Deg` == UnitUpcast<AnglePercentageUnit>(AngleUnit::Deg)
  template <UnitEnum T, UnitEnum U>
  requires NestedUnitEnumOf<U, T>
  KRYS_NODISCARD constexpr T UnitUpcast(U other) noexcept
  {
    return UnitTraits<T>::Upcast(other);
  }

  /// @brief Allows casting DOWN from a composite unit type to one of the unit types it includes.
  /// `AngleUnit::Deg` == UnitDowncast<AngleUnit>(AnglePercentageUnit::Deg)
  /// assert   == UnitDowncast<AngleUnit>(AnglePercentageUnit::Percentage)
  template <UnitEnum T, UnitEnum U>
  requires NestedUnitEnumOf<T, U>
  KRYS_NODISCARD constexpr T UnitDowncast(U other) noexcept
  {
    assert(IsUnit<T>(other));
    return UnitTraits<U>::template Downcast<T>(other);
  }

  /// @brief Allows conditional casting DOWN from a composite unit type to one of the unit types it includes.
  /// `AngleUnit::Deg` == DynamicUnitDowncast<AngleUnit>(AnglePercentageUnit::Deg)
  /// `Maybe`   == DynamicUnitDowncast<AngleUnit>(AnglePercentageUnit::Percentage)
  template <UnitEnum T, UnitEnum U>
  requires NestedUnitEnumOf<T, U>
  KRYS_NODISCARD constexpr Maybe<T> DynamicUnitDowncast(U other) noexcept
  {
    if (!IsUnit<T>(other))
    {
      return {};
    }
    return UnitTraits<U>::template Downcast<T>(other);
  }

  template <CompositeUnitEnum U, typename... F>
  KRYS_NODISCARD constexpr decltype(auto) SwitchOnUnitType(U unit, F &&...f) noexcept
  {
    auto visitor = CreateVisitor(std::forward<F>(f)...);
    return UnitTraits<U>::SwitchOnUnitType(unit, visitor);
  }

#define KRYS_DEFINE_CSS_UNIT_LITERAL(type, name)                                                             \
  inline namespace Literals                                                                                  \
  {                                                                                                          \
    consteval ValueLiteral<type> operator""_css_##name(long double value)                                    \
    {                                                                                                        \
      return ValueLiteral<type> {static_cast<double>(value)};                                                \
    }                                                                                                        \
                                                                                                             \
    consteval ValueLiteral<type> operator""_css_##name(unsigned long long value)                             \
    {                                                                                                        \
      return ValueLiteral<type> {static_cast<double>(value)};                                                \
    }                                                                                                        \
  }

#pragma region <integer>

  enum class IntegerUnit : uint8
  {
    Integer
  };

  KRYS_NODISCARD constexpr CSSUnitType ToCSSUnitType(IntegerUnit) noexcept
  {
    return CSSUnitType::Integer;
  }

  KRYS_NODISCARD constexpr Maybe<IntegerUnit> ToIntegerUnit(CSSUnitType cssUnit) noexcept
  {
    if (cssUnit == CSSUnitType::Integer)
    {
      return IntegerUnit::Integer;
    }

    return Null;
  }

  KRYS_NODISCARD constexpr bool ConversionToCanonicalUnitRequiresConversionData(IntegerUnit) noexcept
  {
    return false;
  }

  constexpr utf8_stringview UnitString(IntegerUnit)
  {
    return u8"";
  }

  template <>
  struct UnitTraits<IntegerUnit>
  {
    constexpr static auto Count = 1uz;
    constexpr static auto Canonical = IntegerUnit::Integer;
    constexpr static auto Category = NumericTypeCategory::Integer;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(CSSRange) noexcept
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<IntegerUnit> Validate(CSSUnitType cssUnit) noexcept
    {
      return ToIntegerUnit(cssUnit);
    }
  };

  static_assert(UnitTraits<IntegerUnit>::Count == ToUnderlying(IntegerUnit::Integer) + 1uz);

  KRYS_DEFINE_CSS_UNIT_LITERAL(IntegerUnit::Integer, integer)

#pragma endregion

#pragma region <number>

  enum class NumberUnit : uint8
  {
    Number
  };

  KRYS_NODISCARD constexpr CSSUnitType ToCSSUnitType(NumberUnit) noexcept
  {
    return CSSUnitType::Number;
  }

  KRYS_NODISCARD constexpr Maybe<NumberUnit> ToNumberUnit(CSSUnitType cssUnit) noexcept
  {
    if (cssUnit == CSSUnitType::Number)
    {
      return NumberUnit::Number;
    }
    return Null;
  }

  KRYS_NODISCARD constexpr bool ConversionToCanonicalUnitRequiresConversionData(NumberUnit) noexcept
  {
    return false;
  }

  KRYS_NODISCARD constexpr utf8_stringview UnitString(NumberUnit) noexcept
  {
    return u8"";
  }

  template <>
  struct UnitTraits<NumberUnit>
  {
    constexpr static auto Count = 1uz;
    constexpr static auto Canonical = NumberUnit::Number;
    constexpr static auto Category = NumericTypeCategory::Number;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(CSSRange) noexcept
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<NumberUnit> Validate(CSSUnitType cssUnit) noexcept
    {
      return ToNumberUnit(cssUnit);
    }
  };

  static_assert(UnitTraits<NumberUnit>::Count == ToUnderlying(NumberUnit::Number) + 1uz);

  KRYS_DEFINE_CSS_UNIT_LITERAL(NumberUnit::Number, number)

#pragma endregion

#pragma region <percentage>

  enum class PercentageUnit : uint8
  {
    Percentage
  };

  KRYS_NODISCARD constexpr CSSUnitType ToCSSUnitType(PercentageUnit) noexcept
  {
    return CSSUnitType::Percentage;
  }

  KRYS_NODISCARD constexpr Maybe<PercentageUnit> ToPercentageUnit(CSSUnitType cssUnit) noexcept
  {
    if (cssUnit == CSSUnitType::Percentage)
    {
      return PercentageUnit::Percentage;
    }

    return Null;
  }

  KRYS_NODISCARD constexpr bool ConversionToCanonicalUnitRequiresConversionData(PercentageUnit) noexcept
  {
    return false;
  }

  KRYS_NODISCARD constexpr utf8_stringview UnitString(PercentageUnit) noexcept
  {
    return u8"%";
  }

  template <>
  struct UnitTraits<PercentageUnit>
  {
    constexpr static auto Count = 1uz;
    constexpr static auto Canonical = PercentageUnit::Percentage;
    constexpr static auto Category = NumericTypeCategory::Percentage;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(CSSRange) noexcept
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<PercentageUnit> Validate(CSSUnitType cssUnit) noexcept
    {
      return ToPercentageUnit(cssUnit);
    }

    KRYS_NODISCARD consteval static bool IsValidRangeForUnitType(CSSRange) noexcept
    {
      return true;
    }
  };

  static_assert(UnitTraits<PercentageUnit>::Count == ToUnderlying(PercentageUnit::Percentage) + 1uz);

  KRYS_DEFINE_CSS_UNIT_LITERAL(PercentageUnit::Percentage, percentage)

#pragma endregion

#pragma region <angle>

#pragma region Angle Utils

  constexpr double radiansPerDegreeDouble = std::numbers::pi / 180.0;
  constexpr double degreesPerRadianDouble = 180.0 / std::numbers::pi;
  constexpr double gradientsPerDegreeDouble = 400.0 / 360.0;
  constexpr double degreesPerGradientDouble = 360.0 / 400.0;
  constexpr double turnsPerDegreeDouble = 1.0 / 360.0;
  constexpr double degreesPerTurnDouble = 360.0;
  constexpr double radiansPerTurnDouble = 2.0 * std::numbers::pi;

  KRYS_NODISCARD constexpr double deg2rad(double d) noexcept
  {
    return d * radiansPerDegreeDouble;
  }

  KRYS_NODISCARD constexpr double rad2deg(double r) noexcept
  {
    return r * degreesPerRadianDouble;
  }

  KRYS_NODISCARD constexpr double deg2grad(double d) noexcept
  {
    return d * gradientsPerDegreeDouble;
  }

  KRYS_NODISCARD constexpr double grad2deg(double g) noexcept
  {
    return g * degreesPerGradientDouble;
  }

  KRYS_NODISCARD constexpr double deg2turn(double d) noexcept
  {
    return d * turnsPerDegreeDouble;
  }

  KRYS_NODISCARD constexpr double turn2deg(double t) noexcept
  {
    return t * degreesPerTurnDouble;
  }

  // Note that these differ from the casting the double values above in their rounding errors.
  constexpr float radiansPerDegreeFloat = std::numbers::pi_v<float> / 180.0f;
  constexpr float degreesPerRadianFloat = 180.0f / std::numbers::pi_v<float>;
  constexpr float gradientsPerDegreeFloat = 400.0f / 360.0f;
  constexpr float degreesPerGradientFloat = 360.0f / 400.0f;
  constexpr float turnsPerDegreeFloat = 1.0f / 360.0f;
  constexpr float degreesPerTurnFloat = 360.0f;
  constexpr float radiansPerTurnFloat = 2.0f * std::numbers::pi_v<float>;

  KRYS_NODISCARD constexpr float deg2rad(float d) noexcept
  {
    return d * radiansPerDegreeFloat;
  }

  KRYS_NODISCARD constexpr float rad2deg(float r) noexcept
  {
    return r * degreesPerRadianFloat;
  }

  KRYS_NODISCARD constexpr float deg2grad(float d) noexcept
  {
    return d * gradientsPerDegreeFloat;
  }

  KRYS_NODISCARD constexpr float grad2deg(float g) noexcept
  {
    return g * degreesPerGradientFloat;
  }

  KRYS_NODISCARD constexpr float deg2turn(float d) noexcept
  {
    return d * turnsPerDegreeFloat;
  }

  KRYS_NODISCARD constexpr float turn2deg(float t) noexcept
  {
    return t * degreesPerTurnFloat;
  }

  // Treat these as conversions through the canonical unit for angles, which is degrees.
  KRYS_NODISCARD constexpr double rad2grad(double r) noexcept
  {
    return deg2grad(rad2deg(r));
  }

  KRYS_NODISCARD constexpr double grad2rad(double g) noexcept
  {
    return deg2rad(grad2deg(g));
  }

  KRYS_NODISCARD constexpr double turn2grad(double t) noexcept
  {
    return deg2grad(turn2deg(t));
  }

  KRYS_NODISCARD constexpr double grad2turn(double g) noexcept
  {
    return deg2turn(grad2deg(g));
  }

  KRYS_NODISCARD constexpr double turn2rad(double t) noexcept
  {
    return deg2rad(turn2deg(t));
  }

  KRYS_NODISCARD constexpr double rad2turn(double r) noexcept
  {
    return deg2turn(rad2deg(r));
  }

  KRYS_NODISCARD constexpr float rad2grad(float r) noexcept
  {
    return deg2grad(rad2deg(r));
  }

  KRYS_NODISCARD constexpr float grad2rad(float g) noexcept
  {
    return deg2rad(grad2deg(g));
  }

  KRYS_NODISCARD constexpr float turn2grad(float t) noexcept
  {
    return deg2grad(turn2deg(t));
  }

  KRYS_NODISCARD constexpr float grad2turn(float g) noexcept
  {
    return deg2turn(grad2deg(g));
  }

  KRYS_NODISCARD constexpr float turn2rad(float t) noexcept
  {
    return deg2rad(turn2deg(t));
  }

  KRYS_NODISCARD constexpr float rad2turn(float r) noexcept
  {
    return deg2turn(rad2deg(r));
  }

#pragma endregion

  enum class AngleUnit : uint8
  {
    Deg,
    Rad,
    Grad,
    Turn
  };

  KRYS_NODISCARD constexpr CSSUnitType ToCSSUnitType(AngleUnit angleUnit) noexcept
  {
    switch (angleUnit)
    {
      case AngleUnit::Deg:  return CSSUnitType::deg;
      case AngleUnit::Rad:  return CSSUnitType::rad;
      case AngleUnit::Grad: return CSSUnitType::grad;
      case AngleUnit::Turn: return CSSUnitType::turn;
    }

    std::unreachable();
  }

  KRYS_NODISCARD constexpr Maybe<AngleUnit> ToAngleUnit(CSSUnitType cssUnit) noexcept
  {
    switch (cssUnit)
    {
      case CSSUnitType::deg:  return AngleUnit::Deg;
      case CSSUnitType::rad:  return AngleUnit::Rad;
      case CSSUnitType::grad: return AngleUnit::Grad;
      case CSSUnitType::turn: return AngleUnit::Turn;
      default:                break;
    }

    return Null;
  }

  KRYS_NODISCARD constexpr bool ConversionToCanonicalUnitRequiresConversionData(AngleUnit) noexcept
  {
    return false;
  }

  template <AngleUnit To, typename T>
  KRYS_NODISCARD constexpr T ConvertAngle(T value, AngleUnit unit) noexcept
  {
    if constexpr (To == AngleUnit::Deg)
    {
      switch (unit)
      {
        case AngleUnit::Deg:  return value;
        case AngleUnit::Rad:  return rad2deg(value);
        case AngleUnit::Grad: return grad2deg(value);
        case AngleUnit::Turn: return turn2deg(value);
      }

      std::unreachable();
    }
    else if constexpr (To == AngleUnit::Rad)
    {
      switch (unit)
      {
        case AngleUnit::Deg:  return deg2rad(value);
        case AngleUnit::Rad:  return value;
        case AngleUnit::Grad: return grad2rad(value);
        case AngleUnit::Turn: return turn2rad(value);
      }

      std::unreachable();
    }
    else if constexpr (To == AngleUnit::Grad)
    {
      switch (unit)
      {
        case AngleUnit::Deg:  return deg2grad(value);
        case AngleUnit::Rad:  return rad2grad(value);
        case AngleUnit::Grad: return value;
        case AngleUnit::Turn: return turn2grad(value);
      }

      std::unreachable();
    }
    else if constexpr (To == AngleUnit::Turn)
    {
      switch (unit)
      {
        case AngleUnit::Deg:  return deg2turn(value);
        case AngleUnit::Rad:  return rad2turn(value);
        case AngleUnit::Grad: return grad2turn(value);
        case AngleUnit::Turn: return value;
      }

      std::unreachable();
    }
  }

  KRYS_NODISCARD utf8_stringview UnitString(AngleUnit) noexcept;

  template <>
  struct UnitTraits<AngleUnit>
  {
    constexpr static auto Count = 4uz;
    constexpr static auto Canonical = AngleUnit::Deg;
    constexpr static auto Category = NumericTypeCategory::Angle;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(CSSRange)
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<AngleUnit> Validate(CSSUnitType cssUnit) noexcept
    {
      return ToAngleUnit(cssUnit);
    }

    template <AngleUnit To, typename T>
    KRYS_NODISCARD constexpr static T Convert(T value, AngleUnit unit) noexcept
    {
      return ConvertAngle<To, T>(value, unit);
    }
  };
  static_assert(UnitTraits<AngleUnit>::Count == ToUnderlying(AngleUnit::Turn) + 1uz);

  KRYS_DEFINE_CSS_UNIT_LITERAL(AngleUnit::Deg, deg)
  KRYS_DEFINE_CSS_UNIT_LITERAL(AngleUnit::Rad, rad)
  KRYS_DEFINE_CSS_UNIT_LITERAL(AngleUnit::Grad, grad)
  KRYS_DEFINE_CSS_UNIT_LITERAL(AngleUnit::Turn, turn)

#pragma endregion

#pragma region <length>

  enum class LengthUnit : uint8
  {
    Px,
    Cm,
    Mm,
    Q,
    In,
    Pt,
    Pc,

    // "font dependent" length units
    Em,
    Ex,
    Lh,
    Cap,
    Ch,
    Ic,

    // "root font dependent" length units
    Rcap,
    Rch,
    Rem,
    Rex,
    Ric,
    Rlh,

    // "viewport-percentage" length units
    Vw,
    Vh,
    Vmin,
    Vmax,
    Vb,
    Vi,
    Svw,
    Svh,
    Svmin,
    Svmax,
    Svb,
    Svi,
    Lvw,
    Lvh,
    Lvmin,
    Lvmax,
    Lvb,
    Lvi,
    Dvw,
    Dvh,
    Dvmin,
    Dvmax,
    Dvb,
    Dvi,

    // "container-percentage" length units
    Cqw,
    Cqh,
    Cqi,
    Cqb,
    Cqmin,
    Cqmax
  };

  KRYS_NODISCARD constexpr CSSUnitType ToCSSUnitType(LengthUnit lengthUnit) noexcept
  {
    switch (lengthUnit)
    {
      case LengthUnit::Px:    return CSSUnitType::px;
      case LengthUnit::Cm:    return CSSUnitType::cm;
      case LengthUnit::Mm:    return CSSUnitType::mm;
      case LengthUnit::Q:     return CSSUnitType::Q;
      case LengthUnit::In:    return CSSUnitType::in;
      case LengthUnit::Pt:    return CSSUnitType::pt;
      case LengthUnit::Pc:    return CSSUnitType::pc;
      case LengthUnit::Em:    return CSSUnitType::em;
      case LengthUnit::Ex:    return CSSUnitType::ex;
      case LengthUnit::Lh:    return CSSUnitType::lh;
      case LengthUnit::Cap:   return CSSUnitType::cap;
      case LengthUnit::Ch:    return CSSUnitType::ch;
      case LengthUnit::Ic:    return CSSUnitType::ic;
      case LengthUnit::Rcap:  return CSSUnitType::rcap;
      case LengthUnit::Rch:   return CSSUnitType::rch;
      case LengthUnit::Rem:   return CSSUnitType::rem;
      case LengthUnit::Rex:   return CSSUnitType::rex;
      case LengthUnit::Ric:   return CSSUnitType::ric;
      case LengthUnit::Rlh:   return CSSUnitType::rlh;
      case LengthUnit::Vw:    return CSSUnitType::vw;
      case LengthUnit::Vh:    return CSSUnitType::vh;
      case LengthUnit::Vmin:  return CSSUnitType::vmin;
      case LengthUnit::Vmax:  return CSSUnitType::vmax;
      case LengthUnit::Vb:    return CSSUnitType::vb;
      case LengthUnit::Vi:    return CSSUnitType::vi;
      case LengthUnit::Svw:   return CSSUnitType::svw;
      case LengthUnit::Svh:   return CSSUnitType::svh;
      case LengthUnit::Svmin: return CSSUnitType::svmin;
      case LengthUnit::Svmax: return CSSUnitType::svmax;
      case LengthUnit::Svb:   return CSSUnitType::svb;
      case LengthUnit::Svi:   return CSSUnitType::svi;
      case LengthUnit::Lvw:   return CSSUnitType::lvw;
      case LengthUnit::Lvh:   return CSSUnitType::lvh;
      case LengthUnit::Lvmin: return CSSUnitType::lvmin;
      case LengthUnit::Lvmax: return CSSUnitType::lvmax;
      case LengthUnit::Lvb:   return CSSUnitType::lvb;
      case LengthUnit::Lvi:   return CSSUnitType::lvi;
      case LengthUnit::Dvw:   return CSSUnitType::dvw;
      case LengthUnit::Dvh:   return CSSUnitType::dvh;
      case LengthUnit::Dvmin: return CSSUnitType::dvmin;
      case LengthUnit::Dvmax: return CSSUnitType::dvmax;
      case LengthUnit::Dvb:   return CSSUnitType::dvb;
      case LengthUnit::Dvi:   return CSSUnitType::dvi;
      case LengthUnit::Cqw:   return CSSUnitType::cqw;
      case LengthUnit::Cqh:   return CSSUnitType::cqh;
      case LengthUnit::Cqi:   return CSSUnitType::cqi;
      case LengthUnit::Cqb:   return CSSUnitType::cqb;
      case LengthUnit::Cqmin: return CSSUnitType::cqmin;
      case LengthUnit::Cqmax: return CSSUnitType::cqmax;
    }

    std::unreachable();
  }

  KRYS_NODISCARD constexpr Maybe<LengthUnit> ToLengthUnit(CSSUnitType cssUnit) noexcept
  {
    switch (cssUnit)
    {
      case CSSUnitType::px:    return LengthUnit::Px;
      case CSSUnitType::cm:    return LengthUnit::Cm;
      case CSSUnitType::mm:    return LengthUnit::Mm;
      case CSSUnitType::Q:     return LengthUnit::Q;
      case CSSUnitType::in:    return LengthUnit::In;
      case CSSUnitType::pt:    return LengthUnit::Pt;
      case CSSUnitType::pc:    return LengthUnit::Pc;
      case CSSUnitType::em:    return LengthUnit::Em;
      case CSSUnitType::ex:    return LengthUnit::Ex;
      case CSSUnitType::lh:    return LengthUnit::Lh;
      case CSSUnitType::cap:   return LengthUnit::Cap;
      case CSSUnitType::ch:    return LengthUnit::Ch;
      case CSSUnitType::ic:    return LengthUnit::Ic;
      case CSSUnitType::rcap:  return LengthUnit::Rcap;
      case CSSUnitType::rch:   return LengthUnit::Rch;
      case CSSUnitType::rem:   return LengthUnit::Rem;
      case CSSUnitType::rex:   return LengthUnit::Rex;
      case CSSUnitType::ric:   return LengthUnit::Ric;
      case CSSUnitType::rlh:   return LengthUnit::Rlh;
      case CSSUnitType::vw:    return LengthUnit::Vw;
      case CSSUnitType::vh:    return LengthUnit::Vh;
      case CSSUnitType::vmin:  return LengthUnit::Vmin;
      case CSSUnitType::vmax:  return LengthUnit::Vmax;
      case CSSUnitType::vb:    return LengthUnit::Vb;
      case CSSUnitType::vi:    return LengthUnit::Vi;
      case CSSUnitType::svw:   return LengthUnit::Svw;
      case CSSUnitType::svh:   return LengthUnit::Svh;
      case CSSUnitType::svmin: return LengthUnit::Svmin;
      case CSSUnitType::svmax: return LengthUnit::Svmax;
      case CSSUnitType::svb:   return LengthUnit::Svb;
      case CSSUnitType::svi:   return LengthUnit::Svi;
      case CSSUnitType::lvw:   return LengthUnit::Lvw;
      case CSSUnitType::lvh:   return LengthUnit::Lvh;
      case CSSUnitType::lvmin: return LengthUnit::Lvmin;
      case CSSUnitType::lvmax: return LengthUnit::Lvmax;
      case CSSUnitType::lvb:   return LengthUnit::Lvb;
      case CSSUnitType::lvi:   return LengthUnit::Lvi;
      case CSSUnitType::dvw:   return LengthUnit::Dvw;
      case CSSUnitType::dvh:   return LengthUnit::Dvh;
      case CSSUnitType::dvmin: return LengthUnit::Dvmin;
      case CSSUnitType::dvmax: return LengthUnit::Dvmax;
      case CSSUnitType::dvb:   return LengthUnit::Dvb;
      case CSSUnitType::dvi:   return LengthUnit::Dvi;
      case CSSUnitType::cqw:   return LengthUnit::Cqw;
      case CSSUnitType::cqh:   return LengthUnit::Cqh;
      case CSSUnitType::cqi:   return LengthUnit::Cqi;
      case CSSUnitType::cqb:   return LengthUnit::Cqb;
      case CSSUnitType::cqmin: return LengthUnit::Cqmin;
      case CSSUnitType::cqmax: return LengthUnit::Cqmax;
      default:                 break;
    }

    return Null;
  }

  KRYS_NODISCARD constexpr bool ConversionToCanonicalUnitRequiresConversionData(LengthUnit unit) noexcept
  {
    using enum LengthUnit;

    switch (unit)
    {
      case LengthUnit::Px:
      case LengthUnit::Cm:
      case LengthUnit::Mm:
      case LengthUnit::Q:
      case LengthUnit::In:
      case LengthUnit::Pt:
      case LengthUnit::Pc:
      {
        return false;
      }
      case LengthUnit::Em:
      case LengthUnit::Ex:
      case LengthUnit::Lh:
      case LengthUnit::Cap:
      case LengthUnit::Ch:
      case LengthUnit::Ic:
      case LengthUnit::Rcap:
      case LengthUnit::Rch:
      case LengthUnit::Rem:
      case LengthUnit::Rex:
      case LengthUnit::Ric:
      case LengthUnit::Rlh:
      case LengthUnit::Vw:
      case LengthUnit::Vh:
      case LengthUnit::Vmin:
      case LengthUnit::Vmax:
      case LengthUnit::Vb:
      case LengthUnit::Vi:
      case LengthUnit::Svw:
      case LengthUnit::Svh:
      case LengthUnit::Svmin:
      case LengthUnit::Svmax:
      case LengthUnit::Svb:
      case LengthUnit::Svi:
      case LengthUnit::Lvw:
      case LengthUnit::Lvh:
      case LengthUnit::Lvmin:
      case LengthUnit::Lvmax:
      case LengthUnit::Lvb:
      case LengthUnit::Lvi:
      case LengthUnit::Dvw:
      case LengthUnit::Dvh:
      case LengthUnit::Dvmin:
      case LengthUnit::Dvmax:
      case LengthUnit::Dvb:
      case LengthUnit::Dvi:
      case LengthUnit::Cqw:
      case LengthUnit::Cqh:
      case LengthUnit::Cqi:
      case LengthUnit::Cqb:
      case LengthUnit::Cqmin:
      case LengthUnit::Cqmax:
      {
        return true;
      }
    }

    std::unreachable();
  }

  KRYS_NODISCARD constexpr bool IsFontRelativeLength(LengthUnit lengthUnit) noexcept
  {
    switch (lengthUnit)
    {
      case LengthUnit::Em:
      case LengthUnit::Ex:
      case LengthUnit::Lh:
      case LengthUnit::Cap:
      case LengthUnit::Ch:
      case LengthUnit::Ic:
      {
        return true;
      }
      default:
      {
        return false;
      }
    }
  }

  KRYS_NODISCARD constexpr bool IsRootFontRelativeLength(LengthUnit lengthUnit) noexcept
  {
    switch (lengthUnit)
    {
      case LengthUnit::Rcap:
      case LengthUnit::Rch:
      case LengthUnit::Rem:
      case LengthUnit::Rex:
      case LengthUnit::Ric:
      case LengthUnit::Rlh:
      {
        return true;
      }
      default:
      {
        return false;
      }
    }
  }

  KRYS_NODISCARD constexpr bool IsFontOrRootFontRelativeLength(LengthUnit lengthUnit) noexcept
  {
    return IsFontRelativeLength(lengthUnit) || IsRootFontRelativeLength(lengthUnit);
  }

  KRYS_NODISCARD constexpr bool IsViewportPercentageLength(LengthUnit lengthUnit) noexcept
  {
    switch (lengthUnit)
    {
      case LengthUnit::Vw:
      case LengthUnit::Vh:
      case LengthUnit::Vmin:
      case LengthUnit::Vmax:
      case LengthUnit::Vb:
      case LengthUnit::Vi:
      case LengthUnit::Svw:
      case LengthUnit::Svh:
      case LengthUnit::Svmin:
      case LengthUnit::Svmax:
      case LengthUnit::Svb:
      case LengthUnit::Svi:
      case LengthUnit::Lvw:
      case LengthUnit::Lvh:
      case LengthUnit::Lvmin:
      case LengthUnit::Lvmax:
      case LengthUnit::Lvb:
      case LengthUnit::Lvi:
      case LengthUnit::Dvw:
      case LengthUnit::Dvh:
      case LengthUnit::Dvmin:
      case LengthUnit::Dvmax:
      case LengthUnit::Dvb:
      case LengthUnit::Dvi:
      {
        return true;
      }
      default:
      {
        return false;
      }
    }
  }

  KRYS_NODISCARD constexpr bool IsContainerPercentageLength(LengthUnit lengthUnit) noexcept
  {
    switch (lengthUnit)
    {
      case LengthUnit::Cqw:
      case LengthUnit::Cqh:
      case LengthUnit::Cqi:
      case LengthUnit::Cqb:
      case LengthUnit::Cqmin:
      case LengthUnit::Cqmax:
      {
        return true;
      }
      default:
      {
        return false;
      }
    }
  }

  KRYS_NODISCARD utf8_stringview UnitString(LengthUnit) noexcept;

  template <>
  struct UnitTraits<LengthUnit>
  {
    constexpr static auto Count = 49uz;
    constexpr static auto Canonical = LengthUnit::Px;
    constexpr static auto Category = NumericTypeCategory::Length;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(CSSRange) noexcept
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<LengthUnit> Validate(CSSUnitType cssUnit) noexcept
    {
      return ToLengthUnit(cssUnit);
    }
  };

  static_assert(UnitTraits<LengthUnit>::Count == ToUnderlying(LengthUnit::Cqmax) + 1uz);

  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Px, px)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Cm, cm)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Mm, mm)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Q, q)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::In, in)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Pt, pt)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Pc, pc)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Em, em)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Ex, ex)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Lh, lh)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Cap, cap)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Ch, ch)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Ic, ic)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Rcap, rcap)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Rch, rch)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Rem, rem)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Rex, rex)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Ric, ric)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Rlh, rlh)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Vw, vw)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Vh, vh)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Vmin, vmin)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Vmax, vmax)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Vb, vb)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Vi, vi)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Svw, svw)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Svh, svh)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Svmin, svmin)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Svmax, svmax)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Svb, svb)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Svi, svi)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Lvw, lvw)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Lvh, lvh)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Lvmin, lvmin)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Lvmax, lvmax)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Lvb, lvb)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Lvi, lvi)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Dvw, dvw)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Dvh, dvh)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Dvmin, dvmin)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Dvmax, dvmax)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Dvb, dvb)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Dvi, dvi)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Cqw, cqw)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Cqh, cqh)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Cqi, cqi)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Cqb, cqb)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Cqmin, cqmin)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Cqmax, cqmax)

#pragma endregion

#pragma region <time>

  enum class TimeUnit : uint8
  {
    S,
    Ms
  };

  KRYS_NODISCARD constexpr CSSUnitType ToCSSUnitType(TimeUnit timeUnit) noexcept
  {
    switch (timeUnit)
    {
      case TimeUnit::S:  return CSSUnitType::s;
      case TimeUnit::Ms: return CSSUnitType::ms;
    }

    std::unreachable();
  }

  KRYS_NODISCARD constexpr Maybe<TimeUnit> ToTimeUnit(CSSUnitType cssUnit) noexcept
  {
    switch (cssUnit)
    {
      case CSSUnitType::s:  return TimeUnit::S;
      case CSSUnitType::ms: return TimeUnit::Ms;
      default:              return Null;
    }
  }

  KRYS_NODISCARD constexpr bool ConversionToCanonicalUnitRequiresConversionData(TimeUnit) noexcept
  {
    return false;
  }

  template <TimeUnit To, typename T>
  KRYS_NODISCARD constexpr T ConvertTime(T value, TimeUnit unit) noexcept
  {
    if constexpr (To == TimeUnit::S)
    {
      switch (unit)
      {
        case TimeUnit::S:  return value;
        case TimeUnit::Ms: return value * secsPerMs;
      }

      std::unreachable();
    }
    else if constexpr (To == TimeUnit::Ms)
    {
      switch (unit)
      {
        case TimeUnit::S:  return value / secsPerMs;
        case TimeUnit::Ms: return value;
      }

      std::unreachable();
    }
  }

  KRYS_NODISCARD utf8_stringview UnitString(TimeUnit) noexcept;

  template <>
  struct UnitTraits<TimeUnit>
  {
    constexpr static auto Count = 2uz;
    constexpr static auto Canonical = TimeUnit::S;
    constexpr static auto Category = NumericTypeCategory::Time;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(CSSRange) noexcept
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<TimeUnit> Validate(CSSUnitType cssUnit) noexcept
    {
      return ToTimeUnit(cssUnit);
    }

    template <TimeUnit To, typename T>
    KRYS_NODISCARD constexpr static T Convert(T value, TimeUnit unit) noexcept
    {
      return ConvertTime<To, T>(value, unit);
    }
  };

  static_assert(UnitTraits<TimeUnit>::Count == ToUnderlying(TimeUnit::Ms) + 1uz);

  KRYS_DEFINE_CSS_UNIT_LITERAL(TimeUnit::S, s)
  KRYS_DEFINE_CSS_UNIT_LITERAL(TimeUnit::Ms, ms)

#pragma endregion

#pragma region <frequency>

  enum class FrequencyUnit : uint8
  {
    Hz,
    Khz
  };

  KRYS_NODISCARD constexpr CSSUnitType ToCSSUnitType(FrequencyUnit frequencyUnit) noexcept
  {
    switch (frequencyUnit)
    {
      case FrequencyUnit::Hz:  return CSSUnitType::hz;
      case FrequencyUnit::Khz: return CSSUnitType::khz;
    }

    std::unreachable();
  }

  KRYS_NODISCARD constexpr Maybe<FrequencyUnit> ToFrequencyUnit(CSSUnitType cssUnit) noexcept
  {
    switch (cssUnit)
    {
      case CSSUnitType::hz:  return FrequencyUnit::Hz;
      case CSSUnitType::khz: return FrequencyUnit::Khz;
      default:               return Null;
    }
  }

  KRYS_NODISCARD constexpr bool ConversionToCanonicalUnitRequiresConversionData(FrequencyUnit) noexcept
  {
    return false;
  }

  template <FrequencyUnit To, typename T>
  KRYS_NODISCARD constexpr T ConvertFrequency(T value, FrequencyUnit unit) noexcept
  {
    if constexpr (To == FrequencyUnit::Hz)
    {
      switch (unit)
      {
        case FrequencyUnit::Hz:  return value;
        case FrequencyUnit::Khz: return value * hertzPerKilohertz;
      }

      std::unreachable();
    }
    else if constexpr (To == FrequencyUnit::Khz)
    {
      switch (unit)
      {
        case FrequencyUnit::Hz:  return value / hertzPerKilohertz;
        case FrequencyUnit::Khz: return value;
      }

      std::unreachable();
    }
  }

  KRYS_NODISCARD utf8_stringview UnitString(FrequencyUnit) noexcept;

  template <>
  struct UnitTraits<FrequencyUnit>
  {
    constexpr static auto Count = 2uz;
    constexpr static auto Canonical = FrequencyUnit::Hz;
    constexpr static auto Category = NumericTypeCategory::Frequency;

    static consteval bool IsValidRangeForCategory(CSSRange) noexcept
    {
      return true;
    }

    static constexpr Maybe<FrequencyUnit> Validate(CSSUnitType cssUnit) noexcept
    {
      return ToFrequencyUnit(cssUnit);
    }

    template <FrequencyUnit To, typename T>
    static constexpr T Convert(T value, FrequencyUnit unit) noexcept
    {
      return ConvertFrequency<To, T>(value, unit);
    }
  };

  static_assert(UnitTraits<FrequencyUnit>::Count == ToUnderlying(FrequencyUnit::Khz) + 1);

  KRYS_DEFINE_CSS_UNIT_LITERAL(FrequencyUnit::Hz, hz)
  KRYS_DEFINE_CSS_UNIT_LITERAL(FrequencyUnit::Khz, khz)

#pragma endregion

#pragma region <resolution>

  enum class ResolutionUnit : uint8
  {
    Dppx,
    X,
    Dpi,
    Dpcm
  };

  KRYS_NODISCARD constexpr CSSUnitType ToCSSUnitType(ResolutionUnit resolutionUnit) noexcept
  {
    switch (resolutionUnit)
    {
      case ResolutionUnit::Dppx: return CSSUnitType::dppx;
      case ResolutionUnit::X:    return CSSUnitType::x;
      case ResolutionUnit::Dpi:  return CSSUnitType::dpi;
      case ResolutionUnit::Dpcm: return CSSUnitType::dpcm;
    }

    std::unreachable();
  }

  KRYS_NODISCARD constexpr Maybe<ResolutionUnit> ToResolutionUnit(CSSUnitType cssUnit) noexcept
  {
    switch (cssUnit)
    {
      case CSSUnitType::dppx: return ResolutionUnit::Dppx;
      case CSSUnitType::x:    return ResolutionUnit::X;
      case CSSUnitType::dpi:  return ResolutionUnit::Dpi;
      case CSSUnitType::dpcm: return ResolutionUnit::Dpcm;
      default:                return Null;
    }
  }

  KRYS_NODISCARD constexpr bool ConversionToCanonicalUnitRequiresConversionData(ResolutionUnit) noexcept
  {
    return false;
  }

  template <ResolutionUnit To, typename T>
  KRYS_NODISCARD constexpr T ConvertResolution(T value, ResolutionUnit unit) noexcept
  {
    if constexpr (To == ResolutionUnit::Dppx)
    {
      switch (unit)
      {
        case ResolutionUnit::Dppx: return value;
        case ResolutionUnit::X:    return value * dppxPerX;
        case ResolutionUnit::Dpi:  return value * dppxPerDpi;
        case ResolutionUnit::Dpcm: return value * dppxPerDpcm;
      }

      std::unreachable();
    }
    else if constexpr (To == ResolutionUnit::X)
    {
      switch (unit)
      {
        case ResolutionUnit::Dppx: return value / dppxPerX;
        case ResolutionUnit::X:    return value;
        case ResolutionUnit::Dpi:  return value * dppxPerDpi / dppxPerX;
        case ResolutionUnit::Dpcm: return value * dppxPerDpcm / dppxPerX;
      }

      std::unreachable();
    }
    else if constexpr (To == ResolutionUnit::Dpi)
    {
      switch (unit)
      {
        case ResolutionUnit::Dppx: return value / dppxPerDpi;
        case ResolutionUnit::X:    return value * dppxPerX / dppxPerDpi;
        case ResolutionUnit::Dpi:  return value;
        case ResolutionUnit::Dpcm: return value * dppxPerDpcm / dppxPerDpi;
      }

      std::unreachable();
    }
    else if constexpr (To == ResolutionUnit::Dpcm)
    {
      switch (unit)
      {
        case ResolutionUnit::Dppx: return value / dppxPerDpcm;
        case ResolutionUnit::X:    return value * dppxPerX / dppxPerDpcm;
        case ResolutionUnit::Dpi:  return value * dppxPerDpi / dppxPerDpcm;
        case ResolutionUnit::Dpcm: return value;
      }

      std::unreachable();
    }
  }

  KRYS_NODISCARD utf8_stringview UnitString(ResolutionUnit) noexcept;

  template <>
  struct UnitTraits<ResolutionUnit>
  {
    constexpr static auto Count = 4uz;
    constexpr static auto Canonical = ResolutionUnit::Dppx;
    constexpr static auto Category = NumericTypeCategory::Resolution;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(CSSRange tokens) noexcept
    {
      return tokens.Min >= 0;
    }

    KRYS_NODISCARD constexpr static Maybe<ResolutionUnit> Validate(CSSUnitType cssUnit) noexcept
    {
      return ToResolutionUnit(cssUnit);
    }

    template <ResolutionUnit To, typename T>
    KRYS_NODISCARD constexpr static T Convert(T value, ResolutionUnit unit) noexcept
    {
      return ConvertResolution<To, T>(value, unit);
    }
  };

  static_assert(UnitTraits<ResolutionUnit>::Count == ToUnderlying(ResolutionUnit::Dpcm) + 1uz);

  KRYS_DEFINE_CSS_UNIT_LITERAL(ResolutionUnit::Dppx, dppx)
  KRYS_DEFINE_CSS_UNIT_LITERAL(ResolutionUnit::X, x)
  KRYS_DEFINE_CSS_UNIT_LITERAL(ResolutionUnit::Dpi, dpi)
  KRYS_DEFINE_CSS_UNIT_LITERAL(ResolutionUnit::Dpcm, dpcm)

#pragma endregion

#pragma region <flex>

  enum class FlexUnit : uint8
  {
    Fr
  };

  KRYS_NODISCARD constexpr CSSUnitType ToCSSUnitType(FlexUnit) noexcept
  {
    return CSSUnitType::fr;
  }

  KRYS_NODISCARD constexpr Maybe<FlexUnit> ToFlexUnit(CSSUnitType cssUnit) noexcept
  {
    if (cssUnit == CSSUnitType::fr)
    {
      return FlexUnit::Fr;
    }

    return Null;
  }

  KRYS_NODISCARD constexpr bool conversionToCanonicalUnitRequiresConversionData(FlexUnit) noexcept
  {
    return false;
  }

  KRYS_NODISCARD constexpr utf8_stringview unitString(FlexUnit) noexcept
  {
    return u8"fr";
  }

  template <>
  struct UnitTraits<FlexUnit>
  {
    constexpr static auto Count = 1uz;
    constexpr static auto Canonical = FlexUnit::Fr;
    constexpr static auto Category = NumericTypeCategory::Flex;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(CSSRange) noexcept
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<FlexUnit> Validate(CSSUnitType cssUnit) noexcept
    {
      return ToFlexUnit(cssUnit);
    }
  };

  static_assert(UnitTraits<FlexUnit>::Count == ToUnderlying(FlexUnit::Fr) + 1uz);

  KRYS_DEFINE_CSS_UNIT_LITERAL(FlexUnit::Fr, fr)

#pragma endregion

#pragma region <angle-percentage>

  // NOTE: The value of the <angle> units in `AnglePercentageUnit` must match their counterpart in
  // `AngleUnit`. This is statically asserted in CSSPrimitiveNumericsUnits.cpp, so if new <angle> units are
  // added, please ensure the counter parts and assertions are updated as well.

  enum class AnglePercentageUnit : uint8
  {
    Deg,
    Rad,
    Grad,
    Turn,
    Percentage
  };

  // Overload of `operator==` to allow comparing `AnglePercentageUnit` and `AngleUnit`.
  KRYS_NODISCARD constexpr bool operator==(AnglePercentageUnit a, AngleUnit b) noexcept
  {
    return ToUnderlying(a) == ToUnderlying(b);
  }

  // Overload of `operator==` to allow comparing `AnglePercentageUnit` and `PercentageUnit`.
  KRYS_NODISCARD constexpr bool operator==(AnglePercentageUnit a, PercentageUnit) noexcept
  {
    return a == AnglePercentageUnit::Percentage;
  }

  KRYS_NODISCARD constexpr CSSUnitType toCSSUnitType(AnglePercentageUnit anglePercentageUnit) noexcept
  {
    switch (anglePercentageUnit)
    {
      case AnglePercentageUnit::Deg:        return CSSUnitType::deg;
      case AnglePercentageUnit::Rad:        return CSSUnitType::rad;
      case AnglePercentageUnit::Grad:       return CSSUnitType::grad;
      case AnglePercentageUnit::Turn:       return CSSUnitType::turn;
      case AnglePercentageUnit::Percentage: return CSSUnitType::Percentage;
    }

    std::unreachable();
  }

  KRYS_NODISCARD constexpr Maybe<AnglePercentageUnit> ToAnglePercentageUnit(CSSUnitType cssUnit) noexcept
  {
    switch (cssUnit)
    {
      case CSSUnitType::deg:        return AnglePercentageUnit::Deg;
      case CSSUnitType::rad:        return AnglePercentageUnit::Rad;
      case CSSUnitType::grad:       return AnglePercentageUnit::Grad;
      case CSSUnitType::turn:       return AnglePercentageUnit::Turn;
      case CSSUnitType::Percentage: return AnglePercentageUnit::Percentage;
      default:                      return Null;
    }
  }

  constexpr bool ConversionToCanonicalUnitRequiresConversionData(AnglePercentageUnit) noexcept
  {
    return false;
  }

  template <>
  struct UnitTraits<AnglePercentageUnit>
  {
    using Composite = std::tuple<AngleUnit, PercentageUnit>;

    constexpr static auto Count = UnitTraits<AngleUnit>::Count + UnitTraits<PercentageUnit>::Count;
    constexpr static auto Canonical = AnglePercentageUnit::Deg;
    constexpr static auto Category = NumericTypeCategory::AnglePercentage;

    KRYS_NODISCARD consteval static bool isValidRangeForCategory(CSSRange) noexcept
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<AnglePercentageUnit> validate(CSSUnitType cssUnit) noexcept
    {
      return ToAnglePercentageUnit(cssUnit);
    }

    template <UnitEnum E>
    KRYS_NODISCARD constexpr static AnglePercentageUnit upcast(E unit) noexcept
    {
      if constexpr (SameType<E, AngleUnit>)
      {
        return static_cast<AnglePercentageUnit>(unit);
      }
      else if constexpr (SameType<E, PercentageUnit>)
      {
        return AnglePercentageUnit::Percentage;
      }
    }

    template <UnitEnum E>
    KRYS_NODISCARD constexpr static bool Is(AnglePercentageUnit unit) noexcept
    {
      if constexpr (SameType<E, AngleUnit>)
      {
        return unit != AnglePercentageUnit::Percentage;
      }
      else if constexpr (SameType<E, PercentageUnit>)
      {
        return unit == AnglePercentageUnit::Percentage;
      }
    }

    template <UnitEnum E>
    KRYS_NODISCARD constexpr static E Downcast(AnglePercentageUnit unit) noexcept
    {
      if constexpr (SameType<E, AngleUnit>)
      {
        return static_cast<AngleUnit>(unit);
      }
      else if constexpr (std::same_as<E, PercentageUnit>)
        return PercentageUnit::Percentage;
    }

    KRYS_NODISCARD constexpr static decltype(auto) SwitchOnUnitType(AnglePercentageUnit unit,
                                                                    auto &&f) noexcept
    {
      if (unit == AnglePercentageUnit::Percentage)
      {
        return f(Downcast<PercentageUnit>(unit));
      }
      return f(Downcast<AngleUnit>(unit));
    }
  };

  static_assert(UnitTraits<AnglePercentageUnit>::Count
                == ToUnderlying(AnglePercentageUnit::Percentage) + 1uz);

#pragma endregion

#pragma region <length-percentage>

  // NOTE: The value of the <length> units in `LengthPercentageUnit` must match their counterpart in
  // `LengthUnit`. This is statically asserted in CSSPrimitiveNumericsUnits.cpp, so if new <length> units are
  // added, please ensure the counter parts and assertions are updated as well.

  enum class LengthPercentageUnit : uint8
  {
    Px,
    Cm,
    Mm,
    Q,
    In,
    Pt,
    Pc,
    Em,
    Ex,
    Lh,
    Cap,
    Ch,
    Ic,
    Rcap,
    Rch,
    Rem,
    Rex,
    Ric,
    Rlh,
    Vw,
    Vh,
    Vmin,
    Vmax,
    Vb,
    Vi,
    Svw,
    Svh,
    Svmin,
    Svmax,
    Svb,
    Svi,
    Lvw,
    Lvh,
    Lvmin,
    Lvmax,
    Lvb,
    Lvi,
    Dvw,
    Dvh,
    Dvmin,
    Dvmax,
    Dvb,
    Dvi,
    Cqw,
    Cqh,
    Cqi,
    Cqb,
    Cqmin,
    Cqmax,
    Percentage
  };

  // Overload of `operator==` to allow comparing `LengthPercentageUnit` and `LengthUnit`.
  KRYS_NODISCARD constexpr bool operator==(LengthPercentageUnit a, LengthUnit b) noexcept
  {
    return ToUnderlying(a) == ToUnderlying(b);
  }

  // Overload of `operator==` to allow comparing `LengthPercentageUnit` and `PercentageUnit`.
  KRYS_NODISCARD constexpr bool operator==(LengthPercentageUnit a, PercentageUnit) noexcept
  {
    return a == LengthPercentageUnit::Percentage;
  }

  KRYS_NODISCARD constexpr CSSUnitType ToCSSUnitType(LengthPercentageUnit lengthPercentageUnit) noexcept
  {
    using enum LengthPercentageUnit;

    switch (lengthPercentageUnit)
    {
      case LengthPercentageUnit::Px:         return CSSUnitType::px;
      case LengthPercentageUnit::Cm:         return CSSUnitType::cm;
      case LengthPercentageUnit::Mm:         return CSSUnitType::mm;
      case LengthPercentageUnit::Q:          return CSSUnitType::Q;
      case LengthPercentageUnit::In:         return CSSUnitType::in;
      case LengthPercentageUnit::Pt:         return CSSUnitType::pt;
      case LengthPercentageUnit::Pc:         return CSSUnitType::pc;
      case LengthPercentageUnit::Em:         return CSSUnitType::em;
      case LengthPercentageUnit::Ex:         return CSSUnitType::ex;
      case LengthPercentageUnit::Lh:         return CSSUnitType::lh;
      case LengthPercentageUnit::Cap:        return CSSUnitType::cap;
      case LengthPercentageUnit::Ch:         return CSSUnitType::ch;
      case LengthPercentageUnit::Ic:         return CSSUnitType::ic;
      case LengthPercentageUnit::Rcap:       return CSSUnitType::rcap;
      case LengthPercentageUnit::Rch:        return CSSUnitType::rch;
      case LengthPercentageUnit::Rem:        return CSSUnitType::rem;
      case LengthPercentageUnit::Rex:        return CSSUnitType::rex;
      case LengthPercentageUnit::Ric:        return CSSUnitType::ric;
      case LengthPercentageUnit::Rlh:        return CSSUnitType::rlh;
      case LengthPercentageUnit::Vw:         return CSSUnitType::vw;
      case LengthPercentageUnit::Vh:         return CSSUnitType::vh;
      case LengthPercentageUnit::Vmin:       return CSSUnitType::vmin;
      case LengthPercentageUnit::Vmax:       return CSSUnitType::vmax;
      case LengthPercentageUnit::Vb:         return CSSUnitType::vb;
      case LengthPercentageUnit::Vi:         return CSSUnitType::vi;
      case LengthPercentageUnit::Svw:        return CSSUnitType::svw;
      case LengthPercentageUnit::Svh:        return CSSUnitType::svh;
      case LengthPercentageUnit::Svmin:      return CSSUnitType::svmin;
      case LengthPercentageUnit::Svmax:      return CSSUnitType::svmax;
      case LengthPercentageUnit::Svb:        return CSSUnitType::svb;
      case LengthPercentageUnit::Svi:        return CSSUnitType::svi;
      case LengthPercentageUnit::Lvw:        return CSSUnitType::lvw;
      case LengthPercentageUnit::Lvh:        return CSSUnitType::lvh;
      case LengthPercentageUnit::Lvmin:      return CSSUnitType::lvmin;
      case LengthPercentageUnit::Lvmax:      return CSSUnitType::lvmax;
      case LengthPercentageUnit::Lvb:        return CSSUnitType::lvb;
      case LengthPercentageUnit::Lvi:        return CSSUnitType::lvi;
      case LengthPercentageUnit::Dvw:        return CSSUnitType::dvw;
      case LengthPercentageUnit::Dvh:        return CSSUnitType::dvh;
      case LengthPercentageUnit::Dvmin:      return CSSUnitType::dvmin;
      case LengthPercentageUnit::Dvmax:      return CSSUnitType::dvmax;
      case LengthPercentageUnit::Dvb:        return CSSUnitType::dvb;
      case LengthPercentageUnit::Dvi:        return CSSUnitType::dvi;
      case LengthPercentageUnit::Cqw:        return CSSUnitType::cqw;
      case LengthPercentageUnit::Cqh:        return CSSUnitType::cqh;
      case LengthPercentageUnit::Cqi:        return CSSUnitType::cqi;
      case LengthPercentageUnit::Cqb:        return CSSUnitType::cqb;
      case LengthPercentageUnit::Cqmin:      return CSSUnitType::cqmin;
      case LengthPercentageUnit::Cqmax:      return CSSUnitType::cqmax;
      case LengthPercentageUnit::Percentage: return CSSUnitType::Percentage;
    }

    std::unreachable();
  }

  KRYS_NODISCARD constexpr Maybe<LengthPercentageUnit> ToLengthPercentageUnit(CSSUnitType cssUnit) noexcept
  {
    switch (cssUnit)
    {
      case CSSUnitType::px:         return LengthPercentageUnit::Px;
      case CSSUnitType::cm:         return LengthPercentageUnit::Cm;
      case CSSUnitType::mm:         return LengthPercentageUnit::Mm;
      case CSSUnitType::Q:          return LengthPercentageUnit::Q;
      case CSSUnitType::in:         return LengthPercentageUnit::In;
      case CSSUnitType::pt:         return LengthPercentageUnit::Pt;
      case CSSUnitType::pc:         return LengthPercentageUnit::Pc;
      case CSSUnitType::em:         return LengthPercentageUnit::Em;
      case CSSUnitType::ex:         return LengthPercentageUnit::Ex;
      case CSSUnitType::lh:         return LengthPercentageUnit::Lh;
      case CSSUnitType::cap:        return LengthPercentageUnit::Cap;
      case CSSUnitType::ch:         return LengthPercentageUnit::Ch;
      case CSSUnitType::ic:         return LengthPercentageUnit::Ic;
      case CSSUnitType::rcap:       return LengthPercentageUnit::Rcap;
      case CSSUnitType::rch:        return LengthPercentageUnit::Rch;
      case CSSUnitType::rem:        return LengthPercentageUnit::Rem;
      case CSSUnitType::rex:        return LengthPercentageUnit::Rex;
      case CSSUnitType::ric:        return LengthPercentageUnit::Ric;
      case CSSUnitType::rlh:        return LengthPercentageUnit::Rlh;
      case CSSUnitType::vw:         return LengthPercentageUnit::Vw;
      case CSSUnitType::vh:         return LengthPercentageUnit::Vh;
      case CSSUnitType::vmin:       return LengthPercentageUnit::Vmin;
      case CSSUnitType::vmax:       return LengthPercentageUnit::Vmax;
      case CSSUnitType::vb:         return LengthPercentageUnit::Vb;
      case CSSUnitType::vi:         return LengthPercentageUnit::Vi;
      case CSSUnitType::svw:        return LengthPercentageUnit::Svw;
      case CSSUnitType::svh:        return LengthPercentageUnit::Svh;
      case CSSUnitType::svmin:      return LengthPercentageUnit::Svmin;
      case CSSUnitType::svmax:      return LengthPercentageUnit::Svmax;
      case CSSUnitType::svb:        return LengthPercentageUnit::Svb;
      case CSSUnitType::svi:        return LengthPercentageUnit::Svi;
      case CSSUnitType::lvw:        return LengthPercentageUnit::Lvw;
      case CSSUnitType::lvh:        return LengthPercentageUnit::Lvh;
      case CSSUnitType::lvmin:      return LengthPercentageUnit::Lvmin;
      case CSSUnitType::lvmax:      return LengthPercentageUnit::Lvmax;
      case CSSUnitType::lvb:        return LengthPercentageUnit::Lvb;
      case CSSUnitType::lvi:        return LengthPercentageUnit::Lvi;
      case CSSUnitType::dvw:        return LengthPercentageUnit::Dvw;
      case CSSUnitType::dvh:        return LengthPercentageUnit::Dvh;
      case CSSUnitType::dvmin:      return LengthPercentageUnit::Dvmin;
      case CSSUnitType::dvmax:      return LengthPercentageUnit::Dvmax;
      case CSSUnitType::dvb:        return LengthPercentageUnit::Dvb;
      case CSSUnitType::dvi:        return LengthPercentageUnit::Dvi;
      case CSSUnitType::cqw:        return LengthPercentageUnit::Cqw;
      case CSSUnitType::cqh:        return LengthPercentageUnit::Cqh;
      case CSSUnitType::cqi:        return LengthPercentageUnit::Cqi;
      case CSSUnitType::cqb:        return LengthPercentageUnit::Cqb;
      case CSSUnitType::cqmin:      return LengthPercentageUnit::Cqmin;
      case CSSUnitType::cqmax:      return LengthPercentageUnit::Cqmax;
      case CSSUnitType::Percentage: return LengthPercentageUnit::Percentage;
      default:                      return Null;
    }
  }

  KRYS_NODISCARD constexpr bool
    ConversionToCanonicalUnitRequiresConversionData(LengthPercentageUnit unit) noexcept
  {
    switch (unit)
    {
      case LengthPercentageUnit::Px:
      case LengthPercentageUnit::Cm:
      case LengthPercentageUnit::Mm:
      case LengthPercentageUnit::Q:
      case LengthPercentageUnit::In:
      case LengthPercentageUnit::Pt:
      case LengthPercentageUnit::Pc:
      case LengthPercentageUnit::Percentage:
      {
        return false;
      }
      case LengthPercentageUnit::Em:
      case LengthPercentageUnit::Ex:
      case LengthPercentageUnit::Lh:
      case LengthPercentageUnit::Cap:
      case LengthPercentageUnit::Ch:
      case LengthPercentageUnit::Ic:
      case LengthPercentageUnit::Rcap:
      case LengthPercentageUnit::Rch:
      case LengthPercentageUnit::Rem:
      case LengthPercentageUnit::Rex:
      case LengthPercentageUnit::Ric:
      case LengthPercentageUnit::Rlh:
      case LengthPercentageUnit::Vw:
      case LengthPercentageUnit::Vh:
      case LengthPercentageUnit::Vmin:
      case LengthPercentageUnit::Vmax:
      case LengthPercentageUnit::Vb:
      case LengthPercentageUnit::Vi:
      case LengthPercentageUnit::Svw:
      case LengthPercentageUnit::Svh:
      case LengthPercentageUnit::Svmin:
      case LengthPercentageUnit::Svmax:
      case LengthPercentageUnit::Svb:
      case LengthPercentageUnit::Svi:
      case LengthPercentageUnit::Lvw:
      case LengthPercentageUnit::Lvh:
      case LengthPercentageUnit::Lvmin:
      case LengthPercentageUnit::Lvmax:
      case LengthPercentageUnit::Lvb:
      case LengthPercentageUnit::Lvi:
      case LengthPercentageUnit::Dvw:
      case LengthPercentageUnit::Dvh:
      case LengthPercentageUnit::Dvmin:
      case LengthPercentageUnit::Dvmax:
      case LengthPercentageUnit::Dvb:
      case LengthPercentageUnit::Dvi:
      case LengthPercentageUnit::Cqw:
      case LengthPercentageUnit::Cqh:
      case LengthPercentageUnit::Cqi:
      case LengthPercentageUnit::Cqb:
      case LengthPercentageUnit::Cqmin:
      case LengthPercentageUnit::Cqmax:
      {
        return true;
      }
    }

    std::unreachable();
  }

  template <>
  struct UnitTraits<LengthPercentageUnit>
  {
    using Composite = std::tuple<LengthUnit, PercentageUnit>;

    constexpr static auto Count = UnitTraits<LengthUnit>::Count + UnitTraits<PercentageUnit>::Count;
    constexpr static auto Canonical = LengthPercentageUnit::Px;
    constexpr static auto Category = NumericTypeCategory::LengthPercentage;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(CSSRange) noexcept
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<LengthPercentageUnit> Validate(CSSUnitType cssUnit) noexcept
    {
      return ToLengthPercentageUnit(cssUnit);
    }

    template <UnitEnum E>
    KRYS_NODISCARD constexpr static LengthPercentageUnit Upcast(E unit) noexcept
    {
      if constexpr (SameType<E, LengthUnit>)
      {
        return static_cast<LengthPercentageUnit>(unit);
      }
      else if constexpr (SameType<E, PercentageUnit>)
      {
        return LengthPercentageUnit::Percentage;
      }
    }

    template <UnitEnum E>
    KRYS_NODISCARD constexpr static bool Is(LengthPercentageUnit unit) noexcept
    {
      if constexpr (SameType<E, LengthUnit>)
      {
        return unit != LengthPercentageUnit::Percentage;
      }
      else if constexpr (SameType<E, PercentageUnit>)
      {
        return unit == LengthPercentageUnit::Percentage;
      }
    }

    template <UnitEnum E>
    KRYS_NODISCARD constexpr static E Downcast(LengthPercentageUnit unit) noexcept
    {
      if constexpr (SameType<E, LengthUnit>)
      {
        return static_cast<LengthUnit>(unit);
      }
      else if constexpr (SameType<E, PercentageUnit>)
      {
        return PercentageUnit::Percentage;
      }
    }

    KRYS_NODISCARD constexpr static decltype(auto) SwitchOnUnitType(LengthPercentageUnit unit,
                                                                    auto &&f) noexcept
    {
      if (unit == LengthPercentageUnit::Percentage)
      {
        return f(Downcast<PercentageUnit>(unit));
      }
      return f(Downcast<LengthUnit>(unit));
    }
  };

  static_assert(UnitTraits<LengthPercentageUnit>::Count
                == ToUnderlying(LengthPercentageUnit::Percentage) + 1uz);

  KRYS_NODISCARD constexpr utf8_stringview UnitString(CompositeUnitEnum auto unit) noexcept
  {
    return SwitchOnUnitType(unit, [](auto alternative) { return UnitString(alternative); });
  }

#pragma endregion
}