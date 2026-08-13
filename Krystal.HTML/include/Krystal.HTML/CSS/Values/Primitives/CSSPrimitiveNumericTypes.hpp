#pragma once

#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumeric.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericRange.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Types/Variant.hpp"

namespace Krys::HTML
{
  // NOTE: This is spelled with an explicit "Or" to distinguish it from types like
  // AnglePercentage/LengthPercentage that have behavior distinctions beyond just being a union of the two
  // types (specifically, calc() has specific behaviors for those types).
  template <CSSRange nR = All, CSSRange pR = nR, typename V = double>
  struct NumberOrPercentage
  {
  private:
    Variant<Number, Percentage> _value;

  public:
    using Number = Number<nR, V>;
    using Percentage = Percentage<pR, V>;

    NumberOrPercentage(Variant<Number, Percentage> &&value) noexcept
    {
      SwitchOn(Krys::Move(value), [this](auto &&alternative) { this->_value = Krys::Move(alternative); });
    }

    NumberOrPercentage(typename Number::Raw value) noexcept : _value {Number {Krys::Move(value)}}
    {
    }

    NumberOrPercentage(Number value) noexcept : _value {Krys::Move(value)}
    {
    }

    NumberOrPercentage(typename Percentage::Raw value) noexcept : _value {Percentage {Krys::Move(value)}}
    {
    }

    NumberOrPercentage(Percentage value) noexcept : _value {Krys::Move(value)}
    {
    }

    bool operator==(const NumberOrPercentage &) const noexcept = default;

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = CreateVisitor(std::forward<F>(f)...);
      using ResultType = decltype(visitor(std::declval<Number>()));

      return SwitchOn([&](const Number &number) -> ResultType { return visitor(number); },
                      [&](const Percentage &percentage) -> ResultType { return visitor(percentage); });
    }
  };

  template <CSSRange nR = All, CSSRange pR = nR, typename V = double>
  struct NumberOrPercentageResolvedToNumber
  {
  private:
    Variant<Number, Percentage> _value;

  public:
    using Number = Number<nR, V>;
    using Percentage = Percentage<pR, V>;

    NumberOrPercentageResolvedToNumber(Variant<Number, Percentage> &&value) noexcept
    {
      SwitchOn(Krys::Move(value), [this](auto &&alternative) { this->_value = Krys::Move(alternative); });
    }

    NumberOrPercentageResolvedToNumber(typename Number::Raw value) noexcept
        : _value {Number {Krys::Move(value)}}
    {
    }

    NumberOrPercentageResolvedToNumber(Number value) noexcept : _value {Krys::Move(value)}
    {
    }

    NumberOrPercentageResolvedToNumber(typename Percentage::Raw value) noexcept
        : _value {Percentage {Krys::Move(value)}}
    {
    }

    NumberOrPercentageResolvedToNumber(Percentage value) noexcept : _value {Krys::Move(value)}
    {
    }

    bool operator==(const NumberOrPercentageResolvedToNumber &) const noexcept = default;

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = CreateVisitor(std::forward<F>(f)...);
      using ResultType = decltype(visitor(std::declval<Number>()));

      return SwitchOn([&](const Number &number) -> ResultType { return visitor(number); },
                      [&](const Percentage &percentage) -> ResultType { return visitor(percentage); });
    }
  };

  template <auto nR, auto pR, typename V>
  constexpr auto TreatAsVariantLike<NumberOrPercentage<nR, pR, V>> = true;

  template <auto nR, auto pR, typename V>
  constexpr auto TreatAsVariantLike<NumberOrPercentageResolvedToNumber<nR, pR, V>> = true;
}