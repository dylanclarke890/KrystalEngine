#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::style
{
  // <border-image-outset-value> = <length [0,∞]> | <number [0,∞]>
  struct BorderImageOutsetValue
  {
    using Length = style::Length<css::NonNegative, float>;
    using Number = style::Number<css::NonNegative, float>;

    constexpr BorderImageOutsetValue(Length length) noexcept : _value {length}
    {
    }
    constexpr BorderImageOutsetValue(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept
        : _value {Length {literal}}
    {
    }

    constexpr BorderImageOutsetValue(Number number) noexcept : _value {number}
    {
    }
    constexpr BorderImageOutsetValue(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept
        : _value {Number {literal}}
    {
    }

    KRYS_NODISCARD constexpr bool IsLength() const noexcept
    {
      return krys::HoldsAlternative<Length>(_value);
    }
    KRYS_NODISCARD constexpr bool IsNumber() const noexcept
    {
      return krys::HoldsAlternative<Number>(_value);
    }

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return SwitchOn([&](auto &value) { return value == 0; });
    }

    KRYS_NODISCARD constexpr bool operator==(const BorderImageOutsetValue &) const noexcept = default;

    KRYS_NODISCARD constexpr bool HasSameType(const BorderImageOutsetValue &other) const noexcept
    {
      return _value.index() == other._value.index();
    }

  private:
    friend struct Blending<BorderImageOutsetValue>;

    Variant<Length, Number> _value {Number {0}};
  };

  // <'border-image-outset'> = [ <length [0,∞]> | <number [0,∞]> ]{1,4}
  // https://drafts.csswg.org/css-backgrounds/#propdef-border-image-outset
  struct BorderImageOutset
  {
    using Value = BorderImageOutsetValue;
    using Edges = css::MinimallySerialisingSpaceSeparatedRectEdges<Value>;

    Edges Values {Value {Value::Number {0}}};

    BorderImageOutset(Edges values) noexcept : Values {krys::move(values)}
    {
    }
    BorderImageOutset(Value top, Value right, Value bottom, Value left) noexcept
        : Values {top, right, bottom, left}
    {
    }
    BorderImageOutset(Value value) noexcept : Values {krys::move(value)}
    {
    }
    BorderImageOutset(Value::Length length) noexcept : Values {length}
    {
    }
    BorderImageOutset(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept
        : Values {Value::Length {literal}}
    {
    }
    BorderImageOutset(Value::Number number) noexcept : Values {number}
    {
    }
    BorderImageOutset(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept
        : Values {Value::Number {literal}}
    {
    }

    KRYS_NODISCARD bool IsZero() const noexcept
    {
      return Values.AllOf([](auto &edge) { return edge.IsZero(); });
    }

    KRYS_NODISCARD bool operator==(const BorderImageOutset &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(BorderImageOutset, Values);

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<BorderImageOutset>
  {
    KRYS_NODISCARD BorderImageOutset operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<BorderImageOutset>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const BorderImageOutset &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<BorderImageOutsetValue>
  {
    KRYS_NODISCARD bool CanBlend(const BorderImageOutsetValue &, const BorderImageOutsetValue &) noexcept;
    KRYS_NODISCARD bool
      RequiresInterpolationForAccumulativeIteration(const BorderImageOutsetValue &,
                                                    const BorderImageOutsetValue &) noexcept;
    KRYS_NODISCARD BorderImageOutsetValue Blend(const BorderImageOutsetValue &,
                                                const BorderImageOutsetValue &,
                                                const BlendingContext &) noexcept;
  };

  template <>
  struct Blending<BorderImageOutset>
  {
    KRYS_NODISCARD bool CanBlend(const BorderImageOutset &, const BorderImageOutset &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const BorderImageOutset &,
                                                                      const BorderImageOutset &) noexcept;
    KRYS_NODISCARD BorderImageOutset Blend(const BorderImageOutset &, const BorderImageOutset &,
                                           const BlendingContext &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::BorderImageOutset)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::BorderImageOutsetValue)
