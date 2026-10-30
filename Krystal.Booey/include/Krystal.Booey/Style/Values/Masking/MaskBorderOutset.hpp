#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::style
{
  // <mask-border-outset-value> = <length [0,∞]> | <number [0,∞]>
  struct MaskBorderOutsetValue
  {
    using Length = style::Length<css::NonNegative, float>;
    using Number = style::Number<css::NonNegative, float>;

    constexpr MaskBorderOutsetValue(Length length) noexcept : _value {length}
    {
    }
    constexpr MaskBorderOutsetValue(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept
        : _value {Length {literal}}
    {
    }

    constexpr MaskBorderOutsetValue(Number number) noexcept : _value {number}
    {
    }
    constexpr MaskBorderOutsetValue(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept
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

    KRYS_NODISCARD constexpr bool operator==(const MaskBorderOutsetValue &) const noexcept = default;

    KRYS_NODISCARD constexpr bool HasSameType(const MaskBorderOutsetValue &other) const noexcept
    {
      return _value.index() == other._value.index();
    }

  private:
    friend struct Blending<MaskBorderOutsetValue>;

    Variant<Length, Number> _value {Number {0}};
  };

  // <'mask-border-outset'> = [ <length [0,∞]> | <number [0,∞]> ]{1,4}
  // https://drafts.fxtf.org/css-masking-1/#propdef-mask-border-outset
  struct MaskBorderOutset
  {
    using Value = MaskBorderOutsetValue;
    using Edges = css::MinimallySerialisingSpaceSeparatedRectEdges<Value>;

    Edges Values {Value {Value::Number {0}}};

    MaskBorderOutset(Edges values) noexcept : Values {krys::move(values)}
    {
    }
    MaskBorderOutset(Value top, Value right, Value bottom, Value left) noexcept
        : Values {top, right, bottom, left}
    {
    }
    MaskBorderOutset(Value value) noexcept : Values {krys::move(value)}
    {
    }
    MaskBorderOutset(Value::Length length) noexcept : Values {length}
    {
    }
    MaskBorderOutset(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept
        : Values {Value::Length {literal}}
    {
    }
    MaskBorderOutset(Value::Number number) noexcept : Values {number}
    {
    }
    MaskBorderOutset(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept
        : Values {Value::Number {literal}}
    {
    }

    KRYS_NODISCARD bool IsZero() const noexcept
    {
      return Values.AllOf([](auto &edge) { return edge.IsZero(); });
    }

    KRYS_NODISCARD bool operator==(const MaskBorderOutset &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(MaskBorderOutset, Values);

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<MaskBorderOutset>
  {
    KRYS_NODISCARD MaskBorderOutset operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<MaskBorderOutset>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const MaskBorderOutset &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<MaskBorderOutsetValue>
  {
    KRYS_NODISCARD bool CanBlend(const MaskBorderOutsetValue &, const MaskBorderOutsetValue &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const MaskBorderOutsetValue &,
                                                                      const MaskBorderOutsetValue &) noexcept;
    KRYS_NODISCARD MaskBorderOutsetValue Blend(const MaskBorderOutsetValue &, const MaskBorderOutsetValue &,
                                               const BlendingContext &) noexcept;
  };

  template <>
  struct Blending<MaskBorderOutset>
  {
    KRYS_NODISCARD bool CanBlend(const MaskBorderOutset &, const MaskBorderOutset &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const MaskBorderOutset &,
                                                                      const MaskBorderOutset &) noexcept;
    KRYS_NODISCARD MaskBorderOutset Blend(const MaskBorderOutset &, const MaskBorderOutset &,
                                          const BlendingContext &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::MaskBorderOutset)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::MaskBorderOutsetValue)
