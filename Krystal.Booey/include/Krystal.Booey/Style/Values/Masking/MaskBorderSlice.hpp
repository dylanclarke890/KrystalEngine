#pragma once

#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"

namespace krys::boo::style
{
  // <mask-border-slice-value> = <number [0,∞]> | <percentage [0,∞]>
  struct MaskBorderSliceValue
  {
    using Number = style::Number<css::NonNegative, float>;
    using Percentage = style::Percentage<css::NonNegative, float>;

    constexpr MaskBorderSliceValue(Number number) noexcept : _value {number}
    {
    }
    constexpr MaskBorderSliceValue(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept
        : _value {Number {literal}}
    {
    }
    constexpr MaskBorderSliceValue(Percentage percentage) noexcept : _value {percentage}
    {
    }
    constexpr MaskBorderSliceValue(css::ValueLiteral<css::PercentageUnit::Percentage> literal) noexcept
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

    KRYS_NODISCARD constexpr bool operator==(const MaskBorderSliceValue &) const noexcept = default;

    KRYS_NODISCARD constexpr bool HasSameType(const MaskBorderSliceValue &other) const noexcept
    {
      return _value.index() == other._value.index();
    }

  private:
    friend struct Blending<MaskBorderSliceValue>;

    Variant<Number, Percentage> _value {Number {0}};
  };

  // <'mask-border-slice'> = [<number [0,∞]> | <percentage [0,∞]>]{1,4} && fill?
  // https://drafts.fxtf.org/css-masking-1/#propdef-mask-border-slice
  struct MaskBorderSlice
  {
    using Value = MaskBorderSliceValue;
    using Edges = css::MinimallySerialisingSpaceSeparatedRectEdges<Value>;

    Edges Values {Value::Number {0}};
    Maybe<css::keywords::Fill> Fill {};

    MaskBorderSlice(Edges values, Maybe<css::keywords::Fill> fill = {}) noexcept
        : Values {krys::move(values)}, Fill {fill}
    {
    }
    MaskBorderSlice(Value top, Value right, Value bottom, Value left,
                    Maybe<css::keywords::Fill> fill = {}) noexcept
        : Values {top, right, bottom, left}, Fill {fill}
    {
    }
    MaskBorderSlice(Value value, Maybe<css::keywords::Fill> fill = {}) noexcept : Values {value}, Fill {fill}
    {
    }
    MaskBorderSlice(Value::Number number, Maybe<css::keywords::Fill> fill = {}) noexcept
        : Values {number}, Fill {fill}
    {
    }
    MaskBorderSlice(css::ValueLiteral<css::NumberUnit::Number> literal,
                    Maybe<css::keywords::Fill> fill = {}) noexcept
        : Values {Value::Number {literal}}, Fill {fill}
    {
    }
    MaskBorderSlice(Value::Percentage percentage, Maybe<css::keywords::Fill> fill = {}) noexcept
        : Values {percentage}, Fill {fill}
    {
    }
    MaskBorderSlice(css::ValueLiteral<css::PercentageUnit::Percentage> literal,
                    Maybe<css::keywords::Fill> fill = {}) noexcept
        : Values {Value::Percentage {literal}}, Fill {fill}
    {
    }

    KRYS_NODISCARD bool operator==(const MaskBorderSlice &) const noexcept = default;
  };
  template <size_t I>
  const auto &get(const MaskBorderSlice &value)
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
  struct CSSValueConversion<MaskBorderSlice>
  {
    KRYS_NODISCARD MaskBorderSlice operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<MaskBorderSlice>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const MaskBorderSlice &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<MaskBorderSliceValue>
  {
    KRYS_NODISCARD bool CanBlend(const MaskBorderSliceValue &, const MaskBorderSliceValue &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const MaskBorderSliceValue &,
                                                                      const MaskBorderSliceValue &) noexcept;
    KRYS_NODISCARD MaskBorderSliceValue Blend(const MaskBorderSliceValue &, const MaskBorderSliceValue &,
                                              const BlendingContext &) noexcept;
  };

  template <>
  struct Blending<MaskBorderSlice>
  {
    KRYS_NODISCARD bool CanBlend(const MaskBorderSlice &, const MaskBorderSlice &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const MaskBorderSlice &,
                                                                      const MaskBorderSlice &) noexcept;
    KRYS_NODISCARD MaskBorderSlice Blend(const MaskBorderSlice &, const MaskBorderSlice &,
                                         const BlendingContext &) noexcept;
  };
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::MaskBorderSlice, 2)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::MaskBorderSliceValue)
