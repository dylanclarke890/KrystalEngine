#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <cassert>

namespace krys::boo::css
{
  // https://www.w3.org/TR/css-values-4/#numeric-types
  enum class NumericTypeCategory : uint8
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
    { UnitTraits<T>::Validate(std::declval<UnitType>()) } -> SameType<Maybe<T>>;
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

  KRYS_NODISCARD constexpr UnitType ToUnitType(IntegerUnit) noexcept
  {
    return UnitType::Integer;
  }

  KRYS_NODISCARD constexpr Maybe<IntegerUnit> ToIntegerUnit(UnitType cssUnit) noexcept
  {
    if (cssUnit == UnitType::Integer)
    {
      return IntegerUnit::Integer;
    }

    return null;
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

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<IntegerUnit> Validate(UnitType cssUnit) noexcept
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

  KRYS_NODISCARD constexpr UnitType ToUnitType(NumberUnit) noexcept
  {
    return UnitType::Number;
  }

  KRYS_NODISCARD constexpr Maybe<NumberUnit> ToNumberUnit(UnitType cssUnit) noexcept
  {
    if (cssUnit == UnitType::Number)
    {
      return NumberUnit::Number;
    }
    return null;
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

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<NumberUnit> Validate(UnitType cssUnit) noexcept
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

  KRYS_NODISCARD constexpr UnitType ToUnitType(PercentageUnit) noexcept
  {
    return UnitType::Percentage;
  }

  KRYS_NODISCARD constexpr Maybe<PercentageUnit> ToPercentageUnit(UnitType cssUnit) noexcept
  {
    if (cssUnit == UnitType::Percentage)
    {
      return PercentageUnit::Percentage;
    }

    return null;
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

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<PercentageUnit> Validate(UnitType cssUnit) noexcept
    {
      return ToPercentageUnit(cssUnit);
    }

    KRYS_NODISCARD consteval static bool IsValidRangeForUnitType(Range) noexcept
    {
      return true;
    }
  };

  static_assert(UnitTraits<PercentageUnit>::Count == ToUnderlying(PercentageUnit::Percentage) + 1uz);

  KRYS_DEFINE_CSS_UNIT_LITERAL(PercentageUnit::Percentage, percentage)

#pragma endregion

#pragma region <angle>

  enum class AngleUnit : uint8
  {
    Deg,
    Rad,
    Grad,
    Turn
  };

  KRYS_NODISCARD constexpr UnitType ToUnitType(AngleUnit angleUnit) noexcept
  {
    switch (angleUnit)
    {
      case AngleUnit::Deg:  return UnitType::deg;
      case AngleUnit::Rad:  return UnitType::rad;
      case AngleUnit::Grad: return UnitType::grad;
      case AngleUnit::Turn: return UnitType::turn;
    }

    std::unreachable();
  }

  KRYS_NODISCARD constexpr Maybe<AngleUnit> ToAngleUnit(UnitType cssUnit) noexcept
  {
    switch (cssUnit)
    {
      case UnitType::deg:  return AngleUnit::Deg;
      case UnitType::rad:  return AngleUnit::Rad;
      case UnitType::grad: return AngleUnit::Grad;
      case UnitType::turn: return AngleUnit::Turn;
      default:             break;
    }

    return null;
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

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range)
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<AngleUnit> Validate(UnitType cssUnit) noexcept
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

  KRYS_NODISCARD constexpr UnitType ToUnitType(LengthUnit lengthUnit) noexcept
  {
    switch (lengthUnit)
    {
      case LengthUnit::Px:    return UnitType::px;
      case LengthUnit::Cm:    return UnitType::cm;
      case LengthUnit::Mm:    return UnitType::mm;
      case LengthUnit::Q:     return UnitType::Q;
      case LengthUnit::In:    return UnitType::in;
      case LengthUnit::Pt:    return UnitType::pt;
      case LengthUnit::Pc:    return UnitType::pc;
      case LengthUnit::Em:    return UnitType::em;
      case LengthUnit::Ex:    return UnitType::ex;
      case LengthUnit::Lh:    return UnitType::lh;
      case LengthUnit::Cap:   return UnitType::cap;
      case LengthUnit::Ch:    return UnitType::ch;
      case LengthUnit::Ic:    return UnitType::ic;
      case LengthUnit::Rcap:  return UnitType::rcap;
      case LengthUnit::Rch:   return UnitType::rch;
      case LengthUnit::Rem:   return UnitType::rem;
      case LengthUnit::Rex:   return UnitType::rex;
      case LengthUnit::Ric:   return UnitType::ric;
      case LengthUnit::Rlh:   return UnitType::rlh;
      case LengthUnit::Vw:    return UnitType::vw;
      case LengthUnit::Vh:    return UnitType::vh;
      case LengthUnit::Vmin:  return UnitType::vmin;
      case LengthUnit::Vmax:  return UnitType::vmax;
      case LengthUnit::Vb:    return UnitType::vb;
      case LengthUnit::Vi:    return UnitType::vi;
      case LengthUnit::Svw:   return UnitType::svw;
      case LengthUnit::Svh:   return UnitType::svh;
      case LengthUnit::Svmin: return UnitType::svmin;
      case LengthUnit::Svmax: return UnitType::svmax;
      case LengthUnit::Svb:   return UnitType::svb;
      case LengthUnit::Svi:   return UnitType::svi;
      case LengthUnit::Lvw:   return UnitType::lvw;
      case LengthUnit::Lvh:   return UnitType::lvh;
      case LengthUnit::Lvmin: return UnitType::lvmin;
      case LengthUnit::Lvmax: return UnitType::lvmax;
      case LengthUnit::Lvb:   return UnitType::lvb;
      case LengthUnit::Lvi:   return UnitType::lvi;
      case LengthUnit::Dvw:   return UnitType::dvw;
      case LengthUnit::Dvh:   return UnitType::dvh;
      case LengthUnit::Dvmin: return UnitType::dvmin;
      case LengthUnit::Dvmax: return UnitType::dvmax;
      case LengthUnit::Dvb:   return UnitType::dvb;
      case LengthUnit::Dvi:   return UnitType::dvi;
      case LengthUnit::Cqw:   return UnitType::cqw;
      case LengthUnit::Cqh:   return UnitType::cqh;
      case LengthUnit::Cqi:   return UnitType::cqi;
      case LengthUnit::Cqb:   return UnitType::cqb;
      case LengthUnit::Cqmin: return UnitType::cqmin;
      case LengthUnit::Cqmax: return UnitType::cqmax;
    }

    std::unreachable();
  }

  KRYS_NODISCARD constexpr Maybe<LengthUnit> ToLengthUnit(UnitType cssUnit) noexcept
  {
    switch (cssUnit)
    {
      case UnitType::px:    return LengthUnit::Px;
      case UnitType::cm:    return LengthUnit::Cm;
      case UnitType::mm:    return LengthUnit::Mm;
      case UnitType::Q:     return LengthUnit::Q;
      case UnitType::in:    return LengthUnit::In;
      case UnitType::pt:    return LengthUnit::Pt;
      case UnitType::pc:    return LengthUnit::Pc;
      case UnitType::em:    return LengthUnit::Em;
      case UnitType::ex:    return LengthUnit::Ex;
      case UnitType::lh:    return LengthUnit::Lh;
      case UnitType::cap:   return LengthUnit::Cap;
      case UnitType::ch:    return LengthUnit::Ch;
      case UnitType::ic:    return LengthUnit::Ic;
      case UnitType::rcap:  return LengthUnit::Rcap;
      case UnitType::rch:   return LengthUnit::Rch;
      case UnitType::rem:   return LengthUnit::Rem;
      case UnitType::rex:   return LengthUnit::Rex;
      case UnitType::ric:   return LengthUnit::Ric;
      case UnitType::rlh:   return LengthUnit::Rlh;
      case UnitType::vw:    return LengthUnit::Vw;
      case UnitType::vh:    return LengthUnit::Vh;
      case UnitType::vmin:  return LengthUnit::Vmin;
      case UnitType::vmax:  return LengthUnit::Vmax;
      case UnitType::vb:    return LengthUnit::Vb;
      case UnitType::vi:    return LengthUnit::Vi;
      case UnitType::svw:   return LengthUnit::Svw;
      case UnitType::svh:   return LengthUnit::Svh;
      case UnitType::svmin: return LengthUnit::Svmin;
      case UnitType::svmax: return LengthUnit::Svmax;
      case UnitType::svb:   return LengthUnit::Svb;
      case UnitType::svi:   return LengthUnit::Svi;
      case UnitType::lvw:   return LengthUnit::Lvw;
      case UnitType::lvh:   return LengthUnit::Lvh;
      case UnitType::lvmin: return LengthUnit::Lvmin;
      case UnitType::lvmax: return LengthUnit::Lvmax;
      case UnitType::lvb:   return LengthUnit::Lvb;
      case UnitType::lvi:   return LengthUnit::Lvi;
      case UnitType::dvw:   return LengthUnit::Dvw;
      case UnitType::dvh:   return LengthUnit::Dvh;
      case UnitType::dvmin: return LengthUnit::Dvmin;
      case UnitType::dvmax: return LengthUnit::Dvmax;
      case UnitType::dvb:   return LengthUnit::Dvb;
      case UnitType::dvi:   return LengthUnit::Dvi;
      case UnitType::cqw:   return LengthUnit::Cqw;
      case UnitType::cqh:   return LengthUnit::Cqh;
      case UnitType::cqi:   return LengthUnit::Cqi;
      case UnitType::cqb:   return LengthUnit::Cqb;
      case UnitType::cqmin: return LengthUnit::Cqmin;
      case UnitType::cqmax: return LengthUnit::Cqmax;
      default:              break;
    }

    return null;
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

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<LengthUnit> Validate(UnitType cssUnit) noexcept
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

  KRYS_NODISCARD constexpr UnitType ToUnitType(TimeUnit timeUnit) noexcept
  {
    switch (timeUnit)
    {
      case TimeUnit::S:  return UnitType::s;
      case TimeUnit::Ms: return UnitType::ms;
    }

    std::unreachable();
  }

  KRYS_NODISCARD constexpr Maybe<TimeUnit> ToTimeUnit(UnitType cssUnit) noexcept
  {
    switch (cssUnit)
    {
      case UnitType::s:  return TimeUnit::S;
      case UnitType::ms: return TimeUnit::Ms;
      default:           return null;
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

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<TimeUnit> Validate(UnitType cssUnit) noexcept
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

  KRYS_NODISCARD constexpr UnitType ToUnitType(FrequencyUnit frequencyUnit) noexcept
  {
    switch (frequencyUnit)
    {
      case FrequencyUnit::Hz:  return UnitType::hz;
      case FrequencyUnit::Khz: return UnitType::khz;
    }

    std::unreachable();
  }

  KRYS_NODISCARD constexpr Maybe<FrequencyUnit> ToFrequencyUnit(UnitType cssUnit) noexcept
  {
    switch (cssUnit)
    {
      case UnitType::hz:  return FrequencyUnit::Hz;
      case UnitType::khz: return FrequencyUnit::Khz;
      default:            return null;
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

    static consteval bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
    }

    constexpr static Maybe<FrequencyUnit> Validate(UnitType cssUnit) noexcept
    {
      return ToFrequencyUnit(cssUnit);
    }

    template <FrequencyUnit To, typename T>
    constexpr static T Convert(T value, FrequencyUnit unit) noexcept
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

  KRYS_NODISCARD constexpr UnitType ToUnitType(ResolutionUnit resolutionUnit) noexcept
  {
    switch (resolutionUnit)
    {
      case ResolutionUnit::Dppx: return UnitType::dppx;
      case ResolutionUnit::X:    return UnitType::x;
      case ResolutionUnit::Dpi:  return UnitType::dpi;
      case ResolutionUnit::Dpcm: return UnitType::dpcm;
    }

    std::unreachable();
  }

  KRYS_NODISCARD constexpr Maybe<ResolutionUnit> ToResolutionUnit(UnitType cssUnit) noexcept
  {
    switch (cssUnit)
    {
      case UnitType::dppx: return ResolutionUnit::Dppx;
      case UnitType::x:    return ResolutionUnit::X;
      case UnitType::dpi:  return ResolutionUnit::Dpi;
      case UnitType::dpcm: return ResolutionUnit::Dpcm;
      default:             return null;
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

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range tokens) noexcept
    {
      return tokens.Min >= 0;
    }

    KRYS_NODISCARD constexpr static Maybe<ResolutionUnit> Validate(UnitType cssUnit) noexcept
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

  KRYS_NODISCARD constexpr UnitType ToUnitType(FlexUnit) noexcept
  {
    return UnitType::fr;
  }

  KRYS_NODISCARD constexpr Maybe<FlexUnit> ToFlexUnit(UnitType cssUnit) noexcept
  {
    if (cssUnit == UnitType::fr)
    {
      return FlexUnit::Fr;
    }

    return null;
  }

  KRYS_NODISCARD constexpr bool conversionToCanonicalUnitRequiresConversionData(FlexUnit) noexcept
  {
    return false;
  }

  KRYS_NODISCARD constexpr utf8_stringview UnitString(FlexUnit) noexcept
  {
    return u8"fr";
  }

  template <>
  struct UnitTraits<FlexUnit>
  {
    constexpr static auto Count = 1uz;
    constexpr static auto Canonical = FlexUnit::Fr;
    constexpr static auto Category = NumericTypeCategory::Flex;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<FlexUnit> Validate(UnitType cssUnit) noexcept
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

  KRYS_NODISCARD constexpr UnitType ToUnitType(AnglePercentageUnit anglePercentageUnit) noexcept
  {
    switch (anglePercentageUnit)
    {
      case AnglePercentageUnit::Deg:        return UnitType::deg;
      case AnglePercentageUnit::Rad:        return UnitType::rad;
      case AnglePercentageUnit::Grad:       return UnitType::grad;
      case AnglePercentageUnit::Turn:       return UnitType::turn;
      case AnglePercentageUnit::Percentage: return UnitType::Percentage;
    }

    std::unreachable();
  }

  KRYS_NODISCARD constexpr Maybe<AnglePercentageUnit> ToAnglePercentageUnit(UnitType cssUnit) noexcept
  {
    switch (cssUnit)
    {
      case UnitType::deg:        return AnglePercentageUnit::Deg;
      case UnitType::rad:        return AnglePercentageUnit::Rad;
      case UnitType::grad:       return AnglePercentageUnit::Grad;
      case UnitType::turn:       return AnglePercentageUnit::Turn;
      case UnitType::Percentage: return AnglePercentageUnit::Percentage;
      default:                   return null;
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

    KRYS_NODISCARD consteval static bool isValidRangeForCategory(Range) noexcept
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<AnglePercentageUnit> validate(UnitType cssUnit) noexcept
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

  KRYS_NODISCARD constexpr UnitType ToUnitType(LengthPercentageUnit lengthPercentageUnit) noexcept
  {
    using enum LengthPercentageUnit;

    switch (lengthPercentageUnit)
    {
      case LengthPercentageUnit::Px:         return UnitType::px;
      case LengthPercentageUnit::Cm:         return UnitType::cm;
      case LengthPercentageUnit::Mm:         return UnitType::mm;
      case LengthPercentageUnit::Q:          return UnitType::Q;
      case LengthPercentageUnit::In:         return UnitType::in;
      case LengthPercentageUnit::Pt:         return UnitType::pt;
      case LengthPercentageUnit::Pc:         return UnitType::pc;
      case LengthPercentageUnit::Em:         return UnitType::em;
      case LengthPercentageUnit::Ex:         return UnitType::ex;
      case LengthPercentageUnit::Lh:         return UnitType::lh;
      case LengthPercentageUnit::Cap:        return UnitType::cap;
      case LengthPercentageUnit::Ch:         return UnitType::ch;
      case LengthPercentageUnit::Ic:         return UnitType::ic;
      case LengthPercentageUnit::Rcap:       return UnitType::rcap;
      case LengthPercentageUnit::Rch:        return UnitType::rch;
      case LengthPercentageUnit::Rem:        return UnitType::rem;
      case LengthPercentageUnit::Rex:        return UnitType::rex;
      case LengthPercentageUnit::Ric:        return UnitType::ric;
      case LengthPercentageUnit::Rlh:        return UnitType::rlh;
      case LengthPercentageUnit::Vw:         return UnitType::vw;
      case LengthPercentageUnit::Vh:         return UnitType::vh;
      case LengthPercentageUnit::Vmin:       return UnitType::vmin;
      case LengthPercentageUnit::Vmax:       return UnitType::vmax;
      case LengthPercentageUnit::Vb:         return UnitType::vb;
      case LengthPercentageUnit::Vi:         return UnitType::vi;
      case LengthPercentageUnit::Svw:        return UnitType::svw;
      case LengthPercentageUnit::Svh:        return UnitType::svh;
      case LengthPercentageUnit::Svmin:      return UnitType::svmin;
      case LengthPercentageUnit::Svmax:      return UnitType::svmax;
      case LengthPercentageUnit::Svb:        return UnitType::svb;
      case LengthPercentageUnit::Svi:        return UnitType::svi;
      case LengthPercentageUnit::Lvw:        return UnitType::lvw;
      case LengthPercentageUnit::Lvh:        return UnitType::lvh;
      case LengthPercentageUnit::Lvmin:      return UnitType::lvmin;
      case LengthPercentageUnit::Lvmax:      return UnitType::lvmax;
      case LengthPercentageUnit::Lvb:        return UnitType::lvb;
      case LengthPercentageUnit::Lvi:        return UnitType::lvi;
      case LengthPercentageUnit::Dvw:        return UnitType::dvw;
      case LengthPercentageUnit::Dvh:        return UnitType::dvh;
      case LengthPercentageUnit::Dvmin:      return UnitType::dvmin;
      case LengthPercentageUnit::Dvmax:      return UnitType::dvmax;
      case LengthPercentageUnit::Dvb:        return UnitType::dvb;
      case LengthPercentageUnit::Dvi:        return UnitType::dvi;
      case LengthPercentageUnit::Cqw:        return UnitType::cqw;
      case LengthPercentageUnit::Cqh:        return UnitType::cqh;
      case LengthPercentageUnit::Cqi:        return UnitType::cqi;
      case LengthPercentageUnit::Cqb:        return UnitType::cqb;
      case LengthPercentageUnit::Cqmin:      return UnitType::cqmin;
      case LengthPercentageUnit::Cqmax:      return UnitType::cqmax;
      case LengthPercentageUnit::Percentage: return UnitType::Percentage;
    }

    std::unreachable();
  }

  KRYS_NODISCARD constexpr Maybe<LengthPercentageUnit> ToLengthPercentageUnit(UnitType cssUnit) noexcept
  {
    switch (cssUnit)
    {
      case UnitType::px:         return LengthPercentageUnit::Px;
      case UnitType::cm:         return LengthPercentageUnit::Cm;
      case UnitType::mm:         return LengthPercentageUnit::Mm;
      case UnitType::Q:          return LengthPercentageUnit::Q;
      case UnitType::in:         return LengthPercentageUnit::In;
      case UnitType::pt:         return LengthPercentageUnit::Pt;
      case UnitType::pc:         return LengthPercentageUnit::Pc;
      case UnitType::em:         return LengthPercentageUnit::Em;
      case UnitType::ex:         return LengthPercentageUnit::Ex;
      case UnitType::lh:         return LengthPercentageUnit::Lh;
      case UnitType::cap:        return LengthPercentageUnit::Cap;
      case UnitType::ch:         return LengthPercentageUnit::Ch;
      case UnitType::ic:         return LengthPercentageUnit::Ic;
      case UnitType::rcap:       return LengthPercentageUnit::Rcap;
      case UnitType::rch:        return LengthPercentageUnit::Rch;
      case UnitType::rem:        return LengthPercentageUnit::Rem;
      case UnitType::rex:        return LengthPercentageUnit::Rex;
      case UnitType::ric:        return LengthPercentageUnit::Ric;
      case UnitType::rlh:        return LengthPercentageUnit::Rlh;
      case UnitType::vw:         return LengthPercentageUnit::Vw;
      case UnitType::vh:         return LengthPercentageUnit::Vh;
      case UnitType::vmin:       return LengthPercentageUnit::Vmin;
      case UnitType::vmax:       return LengthPercentageUnit::Vmax;
      case UnitType::vb:         return LengthPercentageUnit::Vb;
      case UnitType::vi:         return LengthPercentageUnit::Vi;
      case UnitType::svw:        return LengthPercentageUnit::Svw;
      case UnitType::svh:        return LengthPercentageUnit::Svh;
      case UnitType::svmin:      return LengthPercentageUnit::Svmin;
      case UnitType::svmax:      return LengthPercentageUnit::Svmax;
      case UnitType::svb:        return LengthPercentageUnit::Svb;
      case UnitType::svi:        return LengthPercentageUnit::Svi;
      case UnitType::lvw:        return LengthPercentageUnit::Lvw;
      case UnitType::lvh:        return LengthPercentageUnit::Lvh;
      case UnitType::lvmin:      return LengthPercentageUnit::Lvmin;
      case UnitType::lvmax:      return LengthPercentageUnit::Lvmax;
      case UnitType::lvb:        return LengthPercentageUnit::Lvb;
      case UnitType::lvi:        return LengthPercentageUnit::Lvi;
      case UnitType::dvw:        return LengthPercentageUnit::Dvw;
      case UnitType::dvh:        return LengthPercentageUnit::Dvh;
      case UnitType::dvmin:      return LengthPercentageUnit::Dvmin;
      case UnitType::dvmax:      return LengthPercentageUnit::Dvmax;
      case UnitType::dvb:        return LengthPercentageUnit::Dvb;
      case UnitType::dvi:        return LengthPercentageUnit::Dvi;
      case UnitType::cqw:        return LengthPercentageUnit::Cqw;
      case UnitType::cqh:        return LengthPercentageUnit::Cqh;
      case UnitType::cqi:        return LengthPercentageUnit::Cqi;
      case UnitType::cqb:        return LengthPercentageUnit::Cqb;
      case UnitType::cqmin:      return LengthPercentageUnit::Cqmin;
      case UnitType::cqmax:      return LengthPercentageUnit::Cqmax;
      case UnitType::Percentage: return LengthPercentageUnit::Percentage;
      default:                   return null;
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

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
    }

    KRYS_NODISCARD constexpr static Maybe<LengthPercentageUnit> Validate(UnitType cssUnit) noexcept
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