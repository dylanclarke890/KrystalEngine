#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"
#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumericOrKeyword.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::style
{
  template <css::Range nR, css::Range pR, typename V>
  struct NumberOrPercentage;
}

namespace krys
{
  template <boo::css::Range nR, boo::css::Range pR, typename V>
  struct MarkableTraits<boo::style::NumberOrPercentage<nR, pR, V>>;
}

namespace krys::boo::style
{
  // NOTE: This is spelled with an explicit "Or" to distinguish it from types like
  // AnglePercentage/LengthPercentage that have behavior distinctions beyond just being a union of the two
  // types (specifically, calc() has specific behaviors for those types).
  template <css::Range nR = css::All, css::Range pR = nR, typename V = double>
  struct NumberOrPercentage
  {
    friend struct krys::MarkableTraits<NumberOrPercentage>;

    using Number = style::Number<nR, V>;
    using Percentage = style::Percentage<pR, V>;

  private:
    Variant<css::PrimitiveDataEmptyToken, Number, Percentage> _value;

  public:
    NumberOrPercentage(Variant<Number, Percentage> &&value) noexcept
        : _value {krys::SwitchOn(
            krys::move(value), [](auto &&alternative)
            { return Variant<css::PrimitiveDataEmptyToken, Number, Percentage> {krys::move(alternative)}; })}
    {
    }

    NumberOrPercentage(Number value) noexcept : _value {krys::move(value)}
    {
    }

    NumberOrPercentage(Percentage value) noexcept : _value {krys::move(value)}
    {
    }

    KRYS_NODISCARD bool operator==(const NumberOrPercentage &) const noexcept = default;

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);
      using ResultType = decltype(visitor(std::declval<Number>()));

      return krys::SwitchOn(
        _value, [](css::PrimitiveDataEmptyToken) -> ResultType { krys_unreachable(); },
        [&](const Number &number) -> ResultType { return visitor(number); },
        [&](const Percentage &percentage) -> ResultType { return visitor(percentage); });
    }

  private:
    explicit NumberOrPercentage(css::PrimitiveDataEmptyToken token) noexcept : _value {krys::move(token)}
    {
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return std::holds_alternative<css::PrimitiveDataEmptyToken>(_value);
    }
  };

  template <css::Range nR = css::All, css::Range pR = nR, typename V = double>
  struct NumberOrPercentageResolvedToNumber
  {
    using Number = style::Number<nR, V>;
    using Percentage = style::Percentage<pR, V>;

    Number Value {0};

    constexpr NumberOrPercentageResolvedToNumber(Number number) noexcept : Value {number}
    {
    }

    constexpr NumberOrPercentageResolvedToNumber(Percentage percentage) noexcept
        : Value {percentage.Value / static_cast<V>(100)}
    {
    }

    constexpr NumberOrPercentageResolvedToNumber(typename Number::ResolvedValueType value) noexcept
        : NumberOrPercentageResolvedToNumber {Number {value}}
    {
    }

    constexpr NumberOrPercentageResolvedToNumber(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept
        : NumberOrPercentageResolvedToNumber {Number {literal}}
    {
    }

    constexpr NumberOrPercentageResolvedToNumber(
      css::ValueLiteral<css::PercentageUnit::Percentage> literal) noexcept
        : NumberOrPercentageResolvedToNumber {Percentage {literal}}
    {
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return Value.IsZero();
    }

    KRYS_NODISCARD constexpr bool
      operator==(const NumberOrPercentageResolvedToNumber &) const noexcept = default;

    KRYS_NODISCARD constexpr bool operator==(typename Number::ResolvedValueType other) const noexcept
    {
      return Value.Value == other;
    }
  };

  // Standard Numbers
  using NumberAll = Number<css::All>;
  using NumberNonNegative = Number<css::NonNegative>;

  // Standard Angles
  using AngleAllFloat = Angle<css::All, float>;

  // Standard Lengths
  using LengthAll = Length<css::All>;
  using LengthNonNegative = Length<css::NonNegative>;

  // Standard LengthPercentages
  using LengthPercentageAll = LengthPercentage<css::All>;
  using LengthPercentageNonNegative = LengthPercentage<css::NonNegative>;

  // Standard Percentages
  using PercentageAll = Percentage<css::All>;
  using PercentageNonNegative = Percentage<css::NonNegative>;
  using PercentageAllFloat = Percentage<css::All, float>;
  using PercentageNonNegativeFloat = Percentage<css::NonNegative, float>;
  using Percentage0To100 = LengthPercentage<css::Range {0, 100}>;

  // Standard Points
  using LengthPercentageSpaceSeparatedPointAll = css::SpaceSeparatedPoint<LengthPercentageAll>;
  using LengthPercentageSpaceSeparatedPointNonNegative =
    css::SpaceSeparatedPoint<LengthPercentageNonNegative>;

  // Standard Sizes
  using LengthPercentageSpaceSeparatedSizeAll = css::SpaceSeparatedSize<LengthPercentageAll>;
  using LengthPercentageSpaceSeparatedSizeNonNegative = css::SpaceSeparatedSize<LengthPercentageNonNegative>;
  using LengthPercentageMinimallySerialisingSpaceSeparatedSizeAll =
    css::MinimallySerialisingSpaceSeparatedSize<LengthPercentageAll>;
  using LengthPercentageMinimallySerialisingSpaceSeparatedSizeNonNegative =
    css::MinimallySerialisingSpaceSeparatedSize<LengthPercentageNonNegative>;

  // MARK: CSS -> Style

  template <auto nR, auto pR, typename V>
  struct ToStyleMapping<css::NumberOrPercentage<nR, pR, V>>
  {
    using type = NumberOrPercentage<nR, pR, V>;
  };

  template <auto nR, auto pR, typename V>
  struct ToStyleMapping<css::NumberOrPercentageResolvedToNumber<nR, pR, V>>
  {
    using type = NumberOrPercentageResolvedToNumber<nR, pR, V>;
  };

  // MARK: Style -> CSS

  template <auto nR, auto pR, typename V>
  struct ToCSSMapping<NumberOrPercentage<nR, pR, V>>
  {
    using type = css::NumberOrPercentage<nR, pR, V>;
  };

  template <auto nR, auto pR, typename V>
  struct ToCSSMapping<NumberOrPercentageResolvedToNumber<nR, pR, V>>
  {
    using type = css::NumberOrPercentageResolvedToNumber<nR, pR, V>;
  };
}

namespace krys
{
  template <boo::css::Range nR, boo::css::Range pR, typename V>
  struct MarkableTraits<boo::style::NumberOrPercentage<nR, pR, V>>
  {
    KRYS_NODISCARD static bool IsEmptyValue(const boo::style::NumberOrPercentage<nR, pR, V> &value) noexcept
    {
      return value.IsEmpty();
    }

    KRYS_NODISCARD static boo::style::NumberOrPercentage<nR, pR, V> EmptyValue() noexcept
    {
      return boo::style::NumberOrPercentage<nR, pR, V>(boo::css::PrimitiveDataEmptyToken {});
    }
  };
}

namespace krys::boo::css
{
  template <auto nR, auto pR, typename V>
  constexpr auto TreatAsVariantLike<style::NumberOrPercentage<nR, pR, V>> = true;
}
