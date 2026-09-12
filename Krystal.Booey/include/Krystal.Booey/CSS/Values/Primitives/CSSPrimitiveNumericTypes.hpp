#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/CSSPrimitiveNumeric.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/CSSPrimitiveNumericRange.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::css
{
  // NOTE: This is spelled with an explicit "Or" to distinguish it from types like
  // AnglePercentage/LengthPercentage that have behavior distinctions beyond just being a union of the two
  // types (specifically, calc() has specific behaviors for those types).
  template <CSSRange nR = All, CSSRange pR = nR, typename V = double>
  struct NumberOrPercentage
  {
    friend struct MarkableTraits<NumberOrPercentage>;

    using Number = Number<nR, V>;
    using Percentage = Percentage<pR, V>;

  private:
    Variant<PrimitiveDataEmptyToken, Number, Percentage> _value;

    constexpr explicit NumberOrPercentage(PrimitiveDataEmptyToken token) noexcept : _value {krys::move(token)}
    {
    }

  public:
    constexpr NumberOrPercentage(Variant<Number, Percentage> &&value) noexcept
    {
      krys::SwitchOn(krys::move(value), [this](auto &&alternative) { this->_value = krys::move(alternative); });
    }

    constexpr NumberOrPercentage(typename Number::Raw value) noexcept : _value {Number {krys::move(value)}}
    {
    }

    constexpr NumberOrPercentage(Number value) noexcept : _value {krys::move(value)}
    {
    }

    constexpr NumberOrPercentage(typename Percentage::Raw value) noexcept
        : _value {Percentage {krys::move(value)}}
    {
    }

    constexpr NumberOrPercentage(Percentage value) noexcept : _value {krys::move(value)}
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
        : _value {krys::move(token)}
    {
    }

  public:
    constexpr NumberOrPercentageResolvedToNumber(Variant<Number, Percentage> &&value) noexcept
    {
      SwitchOn(krys::move(value), [this](auto &&alternative) { this->_value = krys::move(alternative); });
    }

    constexpr NumberOrPercentageResolvedToNumber(typename Number::Raw value) noexcept
        : _value {Number {krys::move(value)}}
    {
    }

    constexpr NumberOrPercentageResolvedToNumber(Number value) noexcept : _value {krys::move(value)}
    {
    }

    constexpr NumberOrPercentageResolvedToNumber(typename Percentage::Raw value) noexcept
        : _value {Percentage {krys::move(value)}}
    {
    }

    constexpr NumberOrPercentageResolvedToNumber(Percentage value) noexcept : _value {krys::move(value)}
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

namespace krys
{
  template <krys::boo::css::CSSRange nR, krys::boo::css::CSSRange pR, typename V>
  struct MarkableTraits<krys::boo::css::NumberOrPercentage<nR, pR, V>>
  {
    KRYS_NODISCARD constexpr static bool
      IsEmptyValue(const krys::boo::css::NumberOrPercentage<nR, pR, V> &value) noexcept
    {
      return value.IsEmpty();
    }

    KRYS_NODISCARD constexpr static krys::boo::css::NumberOrPercentage<nR, pR, V> EmptyValue() noexcept
    {
      return krys::boo::css::NumberOrPercentage<nR, pR, V>(krys::boo::css::PrimitiveDataEmptyToken());
    }
  };

  template <krys::boo::css::CSSRange nR, krys::boo::css::CSSRange pR, typename V>
  struct MarkableTraits<krys::boo::css::NumberOrPercentageResolvedToNumber<nR, pR, V>>
  {
    KRYS_NODISCARD constexpr static bool
      IsEmptyValue(const krys::boo::css::NumberOrPercentageResolvedToNumber<nR, pR, V> &value) noexcept
    {
      return value.IsEmpty();
    }

    KRYS_NODISCARD constexpr static krys::boo::css::NumberOrPercentageResolvedToNumber<nR, pR, V>
      EmptyValue() noexcept
    {
      return krys::boo::css::NumberOrPercentageResolvedToNumber<nR, pR, V>(krys::boo::css::PrimitiveDataEmptyToken());
    }
  };

}