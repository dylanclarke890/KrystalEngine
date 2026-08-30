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
    using Number = Number<nR, V>;
    using Percentage = Percentage<pR, V>;

  private:
    Variant<PrimitiveDataEmptyToken, Number, Percentage> _value;

    constexpr explicit NumberOrPercentage(PrimitiveDataEmptyToken token) noexcept : _value {Krys::Move(token)}
    {
    }

  public:
    constexpr NumberOrPercentage(Variant<Number, Percentage> &&value) noexcept
    {
      SwitchOn(Krys::Move(value), [this](auto &&alternative) { this->_value = Krys::Move(alternative); });
    }

    constexpr NumberOrPercentage(typename Number::Raw value) noexcept : _value {Number {Krys::Move(value)}}
    {
    }

    constexpr NumberOrPercentage(Number value) noexcept : _value {Krys::Move(value)}
    {
    }

    constexpr NumberOrPercentage(typename Percentage::Raw value) noexcept
        : _value {Percentage {Krys::Move(value)}}
    {
    }

    constexpr NumberOrPercentage(Percentage value) noexcept : _value {Krys::Move(value)}
    {
    }

    constexpr bool operator==(const NumberOrPercentage &) const noexcept = default;

    template <typename... F>
    KRYS_NODISCARD constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = CreateVisitor(std::forward<F>(f)...);
      using ResultType = decltype(visitor(std::declval<Number>()));

      return SwitchOn([](PrimitiveDataEmptyToken) -> ResultType { assert(false); },
                      [&](const Number &number) -> ResultType { return visitor(number); },
                      [&](const Percentage &percentage) -> ResultType { return visitor(percentage); });
    }

  private:
    KRYS_NODISCARD constexpr bool IsEmpty() const noexcept
    {
      return std::holds_alternative<PrimitiveDataEmptyToken>(_value);
    }
  };

  template <CSSRange nR = All, CSSRange pR = nR, typename V = double>
  struct NumberOrPercentageResolvedToNumber
  {
    using Number = Number<nR, V>;
    using Percentage = Percentage<pR, V>;

  private:
    Variant<PrimitiveDataEmptyToken, Number, Percentage> _value;

    constexpr explicit NumberOrPercentageResolvedToNumber(PrimitiveDataEmptyToken token) noexcept
        : _value {Krys::Move(token)}
    {
    }

  public:
    constexpr NumberOrPercentageResolvedToNumber(Variant<Number, Percentage> &&value) noexcept
    {
      SwitchOn(Krys::Move(value), [this](auto &&alternative) { this->_value = Krys::Move(alternative); });
    }

    constexpr NumberOrPercentageResolvedToNumber(typename Number::Raw value) noexcept
        : _value {Number {Krys::Move(value)}}
    {
    }

    constexpr NumberOrPercentageResolvedToNumber(Number value) noexcept : _value {Krys::Move(value)}
    {
    }

    constexpr NumberOrPercentageResolvedToNumber(typename Percentage::Raw value) noexcept
        : _value {Percentage {Krys::Move(value)}}
    {
    }

    constexpr NumberOrPercentageResolvedToNumber(Percentage value) noexcept : _value {Krys::Move(value)}
    {
    }

    constexpr bool operator==(const NumberOrPercentageResolvedToNumber &) const noexcept = default;

    template <typename... F>
    KRYS_NODISCARD constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = CreateVisitor(std::forward<F>(f)...);
      using ResultType = decltype(visitor(std::declval<Number>()));

      return SwitchOn([](PrimitiveDataEmptyToken) -> ResultType { assert(false); },
                      [&](const Number &number) -> ResultType { return visitor(number); },
                      [&](const Percentage &percentage) -> ResultType { return visitor(percentage); });
    }

  private:
    KRYS_NODISCARD constexpr bool IsEmpty() const noexcept
    {
      return std::holds_alternative<PrimitiveDataEmptyToken>(_value);
    }
  };

  template <auto nR, auto pR, typename V>
  constexpr auto TreatAsVariantLike<NumberOrPercentage<nR, pR, V>> = true;

  template <auto nR, auto pR, typename V>
  constexpr auto TreatAsVariantLike<NumberOrPercentageResolvedToNumber<nR, pR, V>> = true;
}

namespace Krys
{
  template <Krys::HTML::CSSRange nR, Krys::HTML::CSSRange pR, typename V>
  struct MarkableTraits<Krys::HTML::NumberOrPercentage<nR, pR, V>>
  {
    KRYS_NODISCARD constexpr static bool
      IsEmptyValue(const Krys::HTML::NumberOrPercentage<nR, pR, V> &value) noexcept
    {
      return value.IsEmpty();
    }

    KRYS_NODISCARD constexpr static Krys::HTML::NumberOrPercentage<nR, pR, V> EmptyValue() noexcept
    {
      return Krys::HTML::NumberOrPercentage<nR, pR, V>(Krys::HTML::PrimitiveDataEmptyToken());
    }
  };

  template <Krys::HTML::CSSRange nR, Krys::HTML::CSSRange pR, typename V>
  struct MarkableTraits<Krys::HTML::NumberOrPercentageResolvedToNumber<nR, pR, V>>
  {
    KRYS_NODISCARD constexpr static bool
      IsEmptyValue(const Krys::HTML::NumberOrPercentageResolvedToNumber<nR, pR, V> &value) noexcept
    {
      return value.IsEmpty();
    }

    KRYS_NODISCARD constexpr static Krys::HTML::NumberOrPercentageResolvedToNumber<nR, pR, V>
      EmptyValue() noexcept
    {
      return Krys::HTML::NumberOrPercentageResolvedToNumber<nR, pR, V>(Krys::HTML::PrimitiveDataEmptyToken());
    }
  };

}