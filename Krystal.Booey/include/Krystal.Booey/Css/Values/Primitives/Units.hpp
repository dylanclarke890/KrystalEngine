/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Css/Values/Primitives/Concepts.hpp"
#include "Krystal.Booey/Css/Values/Primitives/NumericCategory.hpp"
#include "Krystal.Booey/Css/Values/Primitives/Range.hpp"
#include "Krystal.Booey/Css/Values/Units.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <iterator>

namespace krys::boo::css
{
  // MARK: - Unit Literal Type

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

    constexpr ValueLiteral &operator+=(const ConvertibleTo<double> auto &rhs)
    {
      Value += static_cast<double>(rhs);
      return *this;
    }

    friend constexpr ValueLiteral operator+(const ValueLiteral &lhs, const ValueLiteral &rhs)
    {
      return ValueLiteral {lhs.Value + rhs.Value};
    }

    friend constexpr ValueLiteral operator+(const ValueLiteral &lhs, const ConvertibleTo<double> auto &rhs)
    {
      return ValueLiteral {lhs.Value + static_cast<double>(rhs)};
    }

    friend constexpr ValueLiteral operator+(const ConvertibleTo<double> auto &lhs, const ValueLiteral &rhs)
    {
      return ValueLiteral {static_cast<double>(lhs) + rhs.Value};
    }

    // Support subtraction between `ValueLiteral` and machine numeric types.

    constexpr ValueLiteral &operator-=(const ValueLiteral &rhs)
    {
      Value -= rhs.Value;
      return *this;
    }

    constexpr ValueLiteral &operator-=(const ConvertibleTo<double> auto &rhs)
    {
      Value -= static_cast<double>(rhs);
      return *this;
    }

    friend constexpr ValueLiteral operator-(const ValueLiteral &lhs, const ValueLiteral &rhs)
    {
      return ValueLiteral {lhs.Value - rhs.Value};
    }

    friend constexpr ValueLiteral operator-(const ValueLiteral &lhs, const ConvertibleTo<double> auto &rhs)
    {
      return ValueLiteral {lhs.Value - static_cast<double>(rhs)};
    }

    friend constexpr ValueLiteral operator-(const ConvertibleTo<double> auto &lhs, const ValueLiteral &rhs)
    {
      return ValueLiteral {static_cast<double>(lhs) - rhs.Value};
    }

    // Support multiplication between `ValueLiteral` and machine numeric types.

    constexpr ValueLiteral &operator*=(const ConvertibleTo<double> auto &rhs)
    {
      Value *= static_cast<double>(rhs);
      return *this;
    }

    friend constexpr ValueLiteral operator*(const ValueLiteral &lhs, const ConvertibleTo<double> auto &rhs)
    {
      return ValueLiteral {lhs.Value * static_cast<double>(rhs)};
    }

    friend constexpr ValueLiteral operator*(const ConvertibleTo<double> auto &lhs, const ValueLiteral &rhs)
    {
      return ValueLiteral {static_cast<double>(lhs) * rhs.Value};
    }

    // Support division between `ValueLiteral` and machine numeric types.

    constexpr ValueLiteral &operator/=(const ConvertibleTo<double> auto &rhs)
    {
      Value /= static_cast<double>(rhs);
      return *this;
    }

    friend constexpr ValueLiteral operator/(const ValueLiteral &lhs, const ConvertibleTo<double> auto &rhs)
    {
      return ValueLiteral {lhs.Value / static_cast<double>(rhs)};
    }
  };

  /// @brief Checks if casting `other` to unit type `T` is a valid cast.
  template <UnitEnum T, UnitEnum U>
  KRYS_NODISCARD constexpr bool IsUnit(U other) noexcept
  {
    if constexpr (SameType<T, U> || NestedUnitEnumOf<U, T>)
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
      krys_assert(IsUnit<T>(other));
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
    krys_assert(IsUnit<T>(other));
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

#pragma region Units of a unit enum

  // Each unit enum lists the `UnitType` of each of its enumerators, in order, as `UnitTraits<E>::UnitTypes`;
  // everything else about its units comes from Units.hpp.

  /// @brief The `UnitType` @p unit stands for.
  template <UnitEnum E>
  KRYS_NODISCARD constexpr UnitType ToUnitType(E unit) noexcept
  {
    return UnitTraits<E>::UnitTypes[ToUnderlying(unit)];
  }

  /// @brief The enumerator of `E` that stands for @p unit, if there is one.
  template <UnitEnum E>
  KRYS_NODISCARD constexpr Maybe<E> ToUnit(UnitType unit) noexcept
  {
    for (auto i = 0uz; i < UnitTraits<E>::Count; ++i)
    {
      if (UnitTraits<E>::UnitTypes[i] == unit)
      {
        return static_cast<E>(i);
      }
    }
    return null;
  }

  /// @brief @p unit as it's serialised after a number.
  template <UnitEnum E>
  KRYS_NODISCARD constexpr utf8_stringview UnitString(E unit) noexcept
  {
    return UnitTypeString(ToUnitType(unit));
  }

  template <UnitEnum E>
  KRYS_NODISCARD constexpr bool ConversionToCanonicalUnitRequiresConversionData(E unit) noexcept
  {
    return css::ConversionToCanonicalUnitRequiresConversionData(ToUnitType(unit));
  }

  /// @brief @p value in @p from converted to `To`, a unit of the same quantity with a fixed ratio to it.
  template <auto To, typename T>
  requires UnitEnum<decltype(To)>
  KRYS_NODISCARD constexpr T ConvertUnit(T value, decltype(To) from) noexcept
  {
    auto converted = css::ConvertUnit(static_cast<double>(value), ToUnitType(from), ToUnitType(To));
    krys_debug_assert(converted.has_value());
    return static_cast<T>(converted.value_or(0.0));
  }

  namespace detail
  {
    // Whether a unit enum's traits agree with the units it lists: they all measure its category, and its
    // canonical unit is its category's (but for integers, whose canonical unit is a number).
    template <UnitEnum E>
    KRYS_NODISCARD consteval bool TraitsAgreeWithUnits() noexcept
    {
      for (auto unit : UnitTraits<E>::UnitTypes)
      {
        if (CategoryForUnitType(unit) != UnitTraits<E>::Category)
        {
          return false;
        }
      }
      return UnitTraits<E>::Category == Category::Integer
             || ToUnitType(UnitTraits<E>::Canonical) == CanonicalUnitTypeForCategory(UnitTraits<E>::Category);
    }

    // Whether composite enum `C` starts with the units of `E` in the same order, so casting between them by
    // value keeps the unit.
    template <CompositeUnitEnum C, UnitEnum E>
    KRYS_NODISCARD consteval bool StartsWithUnitsOf() noexcept
    {
      for (auto i = 0uz; i < UnitTraits<E>::Count; ++i)
      {
        if (UnitTraits<C>::UnitTypes[i] != UnitTraits<E>::UnitTypes[i])
        {
          return false;
        }
      }
      return true;
    }
  }

#pragma endregion

#define KRYS_DEFINE_CSS_UNIT_LITERAL(type, name)                                                             \
  inline namespace literals                                                                                  \
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
    Integer,
  };

  template <>
  struct UnitTraits<IntegerUnit>
  {
    constexpr static UnitType UnitTypes[] = {UnitType::Integer};
    constexpr static auto Count = std::size(UnitTypes);
    constexpr static auto Canonical = IntegerUnit::Integer;
    constexpr static auto Category = css::Category::Integer;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
    }
  };

  static_assert(UnitTraits<IntegerUnit>::Count == ToUnderlying(IntegerUnit::Integer) + 1uz);
  static_assert(detail::TraitsAgreeWithUnits<IntegerUnit>());

  KRYS_DEFINE_CSS_UNIT_LITERAL(IntegerUnit::Integer, integer)

#pragma endregion

#pragma region <number>

  enum class NumberUnit : uint8
  {
    Number,
  };

  template <>
  struct UnitTraits<NumberUnit>
  {
    constexpr static UnitType UnitTypes[] = {UnitType::Number};
    constexpr static auto Count = std::size(UnitTypes);
    constexpr static auto Canonical = NumberUnit::Number;
    constexpr static auto Category = css::Category::Number;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
    }
  };

  static_assert(UnitTraits<NumberUnit>::Count == ToUnderlying(NumberUnit::Number) + 1uz);
  static_assert(detail::TraitsAgreeWithUnits<NumberUnit>());

  KRYS_DEFINE_CSS_UNIT_LITERAL(NumberUnit::Number, number)

#pragma endregion

#pragma region <percentage>

  enum class PercentageUnit : uint8
  {
    Percentage,
  };

  template <>
  struct UnitTraits<PercentageUnit>
  {
    constexpr static UnitType UnitTypes[] = {UnitType::Percentage};
    constexpr static auto Count = std::size(UnitTypes);
    constexpr static auto Canonical = PercentageUnit::Percentage;
    constexpr static auto Category = css::Category::Percentage;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
    }

    KRYS_NODISCARD consteval static bool IsValidRangeForUnitType(Range) noexcept
    {
      return true;
    }
  };

  static_assert(UnitTraits<PercentageUnit>::Count == ToUnderlying(PercentageUnit::Percentage) + 1uz);
  static_assert(detail::TraitsAgreeWithUnits<PercentageUnit>());

  KRYS_DEFINE_CSS_UNIT_LITERAL(PercentageUnit::Percentage, percentage)

#pragma endregion

#pragma region <angle>

  enum class AngleUnit : uint8
  {
    Deg,
    Rad,
    Grad,
    Turn,
  };

  template <>
  struct UnitTraits<AngleUnit>
  {
    constexpr static UnitType UnitTypes[] = {UnitType::deg, UnitType::rad, UnitType::grad, UnitType::turn};
    constexpr static auto Count = std::size(UnitTypes);
    constexpr static auto Canonical = AngleUnit::Deg;
    constexpr static auto Category = css::Category::Angle;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
    }
  };

  static_assert(UnitTraits<AngleUnit>::Count == ToUnderlying(AngleUnit::Turn) + 1uz);
  static_assert(detail::TraitsAgreeWithUnits<AngleUnit>());

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
    QuirkyEm,
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
    Cqmax,
  };

  template <>
  struct UnitTraits<LengthUnit>
  {
    constexpr static UnitType UnitTypes[] = {
      UnitType::px,    UnitType::cm,   UnitType::mm,       UnitType::Q,   UnitType::in,    UnitType::pt,
      UnitType::pc,    UnitType::em,   UnitType::QuirkyEm, UnitType::ex,  UnitType::lh,    UnitType::cap,
      UnitType::ch,    UnitType::ic,   UnitType::rcap,     UnitType::rch, UnitType::rem,   UnitType::rex,
      UnitType::ric,   UnitType::rlh,  UnitType::vw,       UnitType::vh,  UnitType::vmin,  UnitType::vmax,
      UnitType::vb,    UnitType::vi,   UnitType::svw,      UnitType::svh, UnitType::svmin, UnitType::svmax,
      UnitType::svb,   UnitType::svi,  UnitType::lvw,      UnitType::lvh, UnitType::lvmin, UnitType::lvmax,
      UnitType::lvb,   UnitType::lvi,  UnitType::dvw,      UnitType::dvh, UnitType::dvmin, UnitType::dvmax,
      UnitType::dvb,   UnitType::dvi,  UnitType::cqw,      UnitType::cqh, UnitType::cqi,   UnitType::cqb,
      UnitType::cqmin, UnitType::cqmax};
    constexpr static auto Count = std::size(UnitTypes);
    constexpr static auto Canonical = LengthUnit::Px;
    constexpr static auto Category = css::Category::Length;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
    }
  };

  static_assert(UnitTraits<LengthUnit>::Count == ToUnderlying(LengthUnit::Cqmax) + 1uz);
  static_assert(detail::TraitsAgreeWithUnits<LengthUnit>());

  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Px, px)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Cm, cm)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Mm, mm)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Q, q)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::In, in)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Pt, pt)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Pc, pc)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::Em, em)
  KRYS_DEFINE_CSS_UNIT_LITERAL(LengthUnit::QuirkyEm, quirky_em)
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
    Ms,
  };

  template <>
  struct UnitTraits<TimeUnit>
  {
    constexpr static UnitType UnitTypes[] = {UnitType::s, UnitType::ms};
    constexpr static auto Count = std::size(UnitTypes);
    constexpr static auto Canonical = TimeUnit::S;
    constexpr static auto Category = css::Category::Time;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
    }
  };

  static_assert(UnitTraits<TimeUnit>::Count == ToUnderlying(TimeUnit::Ms) + 1uz);
  static_assert(detail::TraitsAgreeWithUnits<TimeUnit>());

  KRYS_DEFINE_CSS_UNIT_LITERAL(TimeUnit::S, s)
  KRYS_DEFINE_CSS_UNIT_LITERAL(TimeUnit::Ms, ms)

#pragma endregion

#pragma region <frequency>

  enum class FrequencyUnit : uint8
  {
    Hz,
    Khz,
  };

  template <>
  struct UnitTraits<FrequencyUnit>
  {
    constexpr static UnitType UnitTypes[] = {UnitType::hz, UnitType::khz};
    constexpr static auto Count = std::size(UnitTypes);
    constexpr static auto Canonical = FrequencyUnit::Hz;
    constexpr static auto Category = css::Category::Frequency;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
    }
  };

  static_assert(UnitTraits<FrequencyUnit>::Count == ToUnderlying(FrequencyUnit::Khz) + 1);
  static_assert(detail::TraitsAgreeWithUnits<FrequencyUnit>());

  KRYS_DEFINE_CSS_UNIT_LITERAL(FrequencyUnit::Hz, hz)
  KRYS_DEFINE_CSS_UNIT_LITERAL(FrequencyUnit::Khz, khz)

#pragma endregion

#pragma region <resolution>

  enum class ResolutionUnit : uint8
  {
    Dppx,
    X,
    Dpi,
    Dpcm,
  };

  template <>
  struct UnitTraits<ResolutionUnit>
  {
    constexpr static UnitType UnitTypes[] = {UnitType::dppx, UnitType::x, UnitType::dpi, UnitType::dpcm};
    constexpr static auto Count = std::size(UnitTypes);
    constexpr static auto Canonical = ResolutionUnit::Dppx;
    constexpr static auto Category = css::Category::Resolution;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range range) noexcept
    {
      return range.Min >= 0;
    }
  };

  static_assert(UnitTraits<ResolutionUnit>::Count == ToUnderlying(ResolutionUnit::Dpcm) + 1uz);
  static_assert(detail::TraitsAgreeWithUnits<ResolutionUnit>());

  KRYS_DEFINE_CSS_UNIT_LITERAL(ResolutionUnit::Dppx, dppx)
  KRYS_DEFINE_CSS_UNIT_LITERAL(ResolutionUnit::X, x)
  KRYS_DEFINE_CSS_UNIT_LITERAL(ResolutionUnit::Dpi, dpi)
  KRYS_DEFINE_CSS_UNIT_LITERAL(ResolutionUnit::Dpcm, dpcm)

#pragma endregion

#pragma region <flex>

  enum class FlexUnit : uint8
  {
    Fr,
  };

  template <>
  struct UnitTraits<FlexUnit>
  {
    constexpr static UnitType UnitTypes[] = {UnitType::fr};
    constexpr static auto Count = std::size(UnitTypes);
    constexpr static auto Canonical = FlexUnit::Fr;
    constexpr static auto Category = css::Category::Flex;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
    }
  };

  static_assert(UnitTraits<FlexUnit>::Count == ToUnderlying(FlexUnit::Fr) + 1uz);
  static_assert(detail::TraitsAgreeWithUnits<FlexUnit>());

  KRYS_DEFINE_CSS_UNIT_LITERAL(FlexUnit::Fr, fr)

#pragma endregion

#pragma region <angle-percentage>

  enum class AnglePercentageUnit : uint8
  {
    Deg,
    Rad,
    Grad,
    Turn,
    Percentage,
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

  template <>
  struct UnitTraits<AnglePercentageUnit>
  {
    using Composite = std::tuple<AngleUnit, PercentageUnit>;

    constexpr static UnitType UnitTypes[] = {UnitType::deg, UnitType::rad, UnitType::grad, UnitType::turn,
                                             UnitType::Percentage};
    constexpr static auto Count = std::size(UnitTypes);
    constexpr static auto Canonical = AnglePercentageUnit::Deg;
    constexpr static auto Category = css::Category::AnglePercentage;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
    }

    template <UnitEnum E>
    KRYS_NODISCARD constexpr static AnglePercentageUnit Upcast(E unit) noexcept
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
      else if constexpr (SameType<E, PercentageUnit>)
      {
        return PercentageUnit::Percentage;
      }
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
  static_assert(detail::StartsWithUnitsOf<AnglePercentageUnit, AngleUnit>());

#pragma endregion

#pragma region <length-percentage>

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
    QuirkyEm,
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
    Percentage,
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

  template <>
  struct UnitTraits<LengthPercentageUnit>
  {
    using Composite = std::tuple<LengthUnit, PercentageUnit>;

    constexpr static UnitType UnitTypes[] = {
      UnitType::px,    UnitType::cm,    UnitType::mm,        UnitType::Q,   UnitType::in,    UnitType::pt,
      UnitType::pc,    UnitType::em,    UnitType::QuirkyEm,  UnitType::ex,  UnitType::lh,    UnitType::cap,
      UnitType::ch,    UnitType::ic,    UnitType::rcap,      UnitType::rch, UnitType::rem,   UnitType::rex,
      UnitType::ric,   UnitType::rlh,   UnitType::vw,        UnitType::vh,  UnitType::vmin,  UnitType::vmax,
      UnitType::vb,    UnitType::vi,    UnitType::svw,       UnitType::svh, UnitType::svmin, UnitType::svmax,
      UnitType::svb,   UnitType::svi,   UnitType::lvw,       UnitType::lvh, UnitType::lvmin, UnitType::lvmax,
      UnitType::lvb,   UnitType::lvi,   UnitType::dvw,       UnitType::dvh, UnitType::dvmin, UnitType::dvmax,
      UnitType::dvb,   UnitType::dvi,   UnitType::cqw,       UnitType::cqh, UnitType::cqi,   UnitType::cqb,
      UnitType::cqmin, UnitType::cqmax, UnitType::Percentage};
    constexpr static auto Count = std::size(UnitTypes);
    constexpr static auto Canonical = LengthPercentageUnit::Px;
    constexpr static auto Category = css::Category::LengthPercentage;

    KRYS_NODISCARD consteval static bool IsValidRangeForCategory(Range) noexcept
    {
      return true;
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
  static_assert(detail::StartsWithUnitsOf<LengthPercentageUnit, LengthUnit>());

  template <auto unit, typename T>
  KRYS_NODISCARD constexpr decltype(auto) ConvertToValueInUnitsOf(T value) noexcept
  requires SameType<typename T::UnitType, decltype(unit)>
  {
    return ConvertUnit<unit>(value.Value, value.Unit);
  }

#pragma endregion
}
