#pragma once

#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"

namespace krys::boo::style
{
  // <border-image-slice-value> = <number [0,∞]> | <percentage [0,∞]>
  struct BorderImageSliceValue
  {
    using Number = style::Number<css::NonNegative, float>;
    using Percentage = style::Percentage<css::NonNegative, float>;

    constexpr BorderImageSliceValue(Number number) noexcept : _value {number}
    {
    }
    constexpr BorderImageSliceValue(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept
        : _value {Number {literal}}
    {
    }
    constexpr BorderImageSliceValue(Percentage percentage) noexcept : _value {percentage}
    {
    }
    constexpr BorderImageSliceValue(css::ValueLiteral<css::PercentageUnit::Percentage> literal) noexcept
        : _value {Percentage {literal}}
    {
    }

    KRYS_NODISCARD constexpr bool IsNumber() const noexcept
    {
      return krys::HoldsAlternative<Number>(_value);
    }
    KRYS_NODISCARD constexpr bool IsPercentage() const noexcept
    {
      return krys::HoldsAlternative<Percentage>(_value);
    }

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD constexpr bool operator==(const BorderImageSliceValue &) const noexcept = default;

    KRYS_NODISCARD constexpr bool HasSameType(const BorderImageSliceValue &other) const noexcept
    {
      return _value.index() == other._value.index();
    }

  private:
    friend struct Blending<BorderImageSliceValue>;

    Variant<Number, Percentage> _value {Percentage {100}};
  };

  // <'border-image-slice'> = [<number [0,∞]> | <percentage [0,∞]>]{1,4} && fill?
  // https://drafts.csswg.org/css-backgrounds/#propdef-border-image-slice
  struct BorderImageSlice
  {
    using Value = BorderImageSliceValue;
    using Edges = css::MinimallySerialisingSpaceSeparatedRectEdges<Value>;

    Edges Values {Value::Percentage {100}};
    Maybe<css::keywords::Fill> Fill {};

    BorderImageSlice(Edges values, Maybe<css::keywords::Fill> fill = {}) noexcept
        : Values {krys::move(values)}, Fill {fill}
    {
    }
    BorderImageSlice(Value top, Value right, Value bottom, Value left,
                     Maybe<css::keywords::Fill> fill = {}) noexcept
        : Values {top, right, bottom, left}, Fill {fill}
    {
    }
    BorderImageSlice(Value value, Maybe<css::keywords::Fill> fill = {}) noexcept : Values {value}, Fill {fill}
    {
    }
    BorderImageSlice(Value::Number number, Maybe<css::keywords::Fill> fill = {}) noexcept
        : Values {number}, Fill {fill}
    {
    }
    BorderImageSlice(css::ValueLiteral<css::NumberUnit::Number> literal,
                     Maybe<css::keywords::Fill> fill = {}) noexcept
        : Values {Value::Number {literal}}, Fill {fill}
    {
    }
    BorderImageSlice(Value::Percentage percentage, Maybe<css::keywords::Fill> fill = {}) noexcept
        : Values {percentage}, Fill {fill}
    {
    }
    BorderImageSlice(css::ValueLiteral<css::PercentageUnit::Percentage> literal,
                     Maybe<css::keywords::Fill> fill = {}) noexcept
        : Values {Value::Percentage {literal}}, Fill {fill}
    {
    }

    KRYS_NODISCARD bool operator==(const BorderImageSlice &) const noexcept = default;
  };
  template <size_t I>
  const auto &get(const BorderImageSlice &value)
  {
    if constexpr (!I)
    {
      return value.Values;
    }
    else if constexpr (I == 1)
    {
      return value.Fill;
    }
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<BorderImageSlice>
  {
    KRYS_NODISCARD BorderImageSlice operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<BorderImageSlice>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const BorderImageSlice &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<BorderImageSliceValue>
  {
    KRYS_NODISCARD bool CanBlend(const BorderImageSliceValue &, const BorderImageSliceValue &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const BorderImageSliceValue &,
                                                                      const BorderImageSliceValue &) noexcept;
    KRYS_NODISCARD BorderImageSliceValue Blend(const BorderImageSliceValue &, const BorderImageSliceValue &,
                                               const BlendingContext &) noexcept;
  };

  template <>
  struct Blending<BorderImageSlice>
  {
    KRYS_NODISCARD bool CanBlend(const BorderImageSlice &, const BorderImageSlice &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const BorderImageSlice &,
                                                                      const BorderImageSlice &) noexcept;
    KRYS_NODISCARD BorderImageSlice Blend(const BorderImageSlice &, const BorderImageSlice &,
                                          const BlendingContext &) noexcept;
  };
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::BorderImageSlice, 2)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::BorderImageSliceValue)
