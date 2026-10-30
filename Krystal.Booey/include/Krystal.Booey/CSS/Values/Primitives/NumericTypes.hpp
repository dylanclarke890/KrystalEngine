#pragma once

// Umbrella header for family of CSS numeric types.

#include "Krystal.Booey/CSS/Values/Primitives/PrimitiveNumeric.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/PrimitiveNumericOrKeyword.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::css
{
  template <Range nR, Range pR, typename V>
  struct NumberOrPercentage;

  template <Range nR, Range pR, typename V>
  struct NumberOrPercentageResolvedToNumber;
}

namespace krys
{
  template <krys::boo::css::Range nR, krys::boo::css::Range pR, typename V>
  struct MarkableTraits<krys::boo::css::NumberOrPercentage<nR, pR, V>>;

  template <krys::boo::css::Range nR, krys::boo::css::Range pR, typename V>
  struct MarkableTraits<krys::boo::css::NumberOrPercentageResolvedToNumber<nR, pR, V>>;
}

namespace krys::boo::css
{
  // NOTE: This is spelled with an explicit "Or" to distinguish it from types like
  // AnglePercentage/LengthPercentage that have behavior distinctions beyond just being a union of the two
  // types (specifically, calc() has specific behaviors for those types).
  template <Range nR = All, Range pR = nR, typename V = double>
  struct NumberOrPercentage
  {
    friend struct krys::MarkableTraits<NumberOrPercentage>;

    using Number = css::Number<nR, V>;
    using Percentage = css::Percentage<pR, V>;

  private:
    Variant<PrimitiveDataEmptyToken, Number, Percentage> _value;

  public:
    NumberOrPercentage(Variant<Number, Percentage> &&value) noexcept
        : _value {krys::SwitchOn(
            krys::move(value), [](auto &&alternative)
            { return Variant<PrimitiveDataEmptyToken, Number, Percentage> {krys::move(alternative)}; })}
    {
    }

    NumberOrPercentage(typename Number::Raw value) noexcept : _value {Number {krys::move(value)}}
    {
    }

    NumberOrPercentage(Number value) noexcept : _value {krys::move(value)}
    {
    }

    NumberOrPercentage(typename Percentage::Raw value) noexcept : _value {Percentage {krys::move(value)}}
    {
    }

    NumberOrPercentage(Percentage value) noexcept : _value {krys::move(value)}
    {
    }

    KRYS_NODISCARD bool operator==(const NumberOrPercentage &) const noexcept = default;

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);
      using ResultType = decltype(visitor(std::declval<Number>()));

      return krys::SwitchOn(
        _value, [](PrimitiveDataEmptyToken) -> ResultType { krys_unreachable(); },
        [&](const Number &number) -> ResultType { return visitor(number); },
        [&](const Percentage &percentage) -> ResultType { return visitor(percentage); });
    }

  private:
    explicit NumberOrPercentage(PrimitiveDataEmptyToken token) noexcept : _value {krys::move(token)}
    {
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return std::holds_alternative<PrimitiveDataEmptyToken>(_value);
    }
  };

  template <Range nR = All, Range pR = nR, typename V = double>
  struct NumberOrPercentageResolvedToNumber
  {
    friend struct krys::MarkableTraits<NumberOrPercentageResolvedToNumber>;

    using Number = css::Number<nR, V>;
    using Percentage = css::Percentage<pR, V>;

  private:
    Variant<PrimitiveDataEmptyToken, Number, Percentage> _value;

  public:
    NumberOrPercentageResolvedToNumber(Variant<Number, Percentage> &&value) noexcept
        : _value {krys::SwitchOn(
            krys::move(value), [](auto &&alternative)
            { return Variant<PrimitiveDataEmptyToken, Number, Percentage> {krys::move(alternative)}; })}
    {
    }

    NumberOrPercentageResolvedToNumber(typename Number::Raw value) noexcept
        : _value {Number {krys::move(value)}}
    {
    }

    NumberOrPercentageResolvedToNumber(Number value) noexcept : _value {krys::move(value)}
    {
    }

    NumberOrPercentageResolvedToNumber(typename Percentage::Raw value) noexcept
        : _value {Percentage {krys::move(value)}}
    {
    }

    NumberOrPercentageResolvedToNumber(Percentage value) noexcept : _value {krys::move(value)}
    {
    }

    KRYS_NODISCARD bool operator==(const NumberOrPercentageResolvedToNumber &) const noexcept = default;

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);
      using ResultType = decltype(visitor(std::declval<Number>()));

      return krys::SwitchOn(
        _value, [](PrimitiveDataEmptyToken) -> ResultType { krys_unreachable(); },
        [&](const Number &number) -> ResultType { return visitor(number); },
        [&](const Percentage &percentage) -> ResultType { return visitor(percentage); });
    }

  private:
    explicit NumberOrPercentageResolvedToNumber(PrimitiveDataEmptyToken token) noexcept
        : _value {krys::move(token)}
    {
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return std::holds_alternative<PrimitiveDataEmptyToken>(_value);
    }
  };

  template <Range nR, Range pR, typename V>
  constexpr auto TreatAsVariantLike<NumberOrPercentage<nR, pR, V>> = true;

  template <Range nR, Range pR, typename V>
  constexpr auto TreatAsVariantLike<NumberOrPercentageResolvedToNumber<nR, pR, V>> = true;
}

namespace krys
{
  template <krys::boo::css::Range nR, krys::boo::css::Range pR, typename V>
  struct MarkableTraits<krys::boo::css::NumberOrPercentage<nR, pR, V>>
  {
    KRYS_NODISCARD static bool
      IsEmptyValue(const krys::boo::css::NumberOrPercentage<nR, pR, V> &value) noexcept
    {
      return value.IsEmpty();
    }

    KRYS_NODISCARD static krys::boo::css::NumberOrPercentage<nR, pR, V> EmptyValue() noexcept
    {
      return krys::boo::css::NumberOrPercentage<nR, pR, V>(krys::boo::css::PrimitiveDataEmptyToken {});
    }
  };

  template <krys::boo::css::Range nR, krys::boo::css::Range pR, typename V>
  struct MarkableTraits<krys::boo::css::NumberOrPercentageResolvedToNumber<nR, pR, V>>
  {
    KRYS_NODISCARD static bool
      IsEmptyValue(const krys::boo::css::NumberOrPercentageResolvedToNumber<nR, pR, V> &value) noexcept
    {
      return value.IsEmpty();
    }

    KRYS_NODISCARD static krys::boo::css::NumberOrPercentageResolvedToNumber<nR, pR, V> EmptyValue() noexcept
    {
      return krys::boo::css::NumberOrPercentageResolvedToNumber<nR, pR, V>(
        krys::boo::css::PrimitiveDataEmptyToken {});
    }
  };
}
