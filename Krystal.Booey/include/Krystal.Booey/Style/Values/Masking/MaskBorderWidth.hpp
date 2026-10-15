#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  struct MaskBorderWidthValueLength : LengthWrapperBase<LengthPercentage<css::NonNegative>>
  {
    using Base::Base;
  };

  // <mask-border-width-value> = <length-percentage [0,∞]> | <number [0,∞]> | auto
  struct MaskBorderWidthValue
  {
    using LengthPercentage = MaskBorderWidthValueLength;
    using Number = style::Number<css::NonNegative, float>;

    MaskBorderWidthValue(css::keywords::Auto keyword) noexcept : _value {keyword}
    {
    }
    MaskBorderWidthValue(LengthPercentage &&length) noexcept : _value {krys::move(length)}
    {
    }
    MaskBorderWidthValue(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept
        : _value {LengthPercentage {literal}}
    {
    }
    MaskBorderWidthValue(css::ValueLiteral<css::PercentageUnit::Percentage> literal) noexcept
        : _value {LengthPercentage {literal}}
    {
    }
    MaskBorderWidthValue(Number number) noexcept : _value {number}
    {
    }
    MaskBorderWidthValue(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept
        : _value {Number {literal}}
    {
    }

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::Auto>(_value);
    }
    KRYS_NODISCARD bool IsLengthPercentage() const noexcept
    {
      return krys::HoldsAlternative<LengthPercentage>(_value);
    }
    KRYS_NODISCARD bool IsNumber() const noexcept
    {
      return krys::HoldsAlternative<Number>(_value);
    }

    KRYS_NODISCARD Maybe<LengthPercentage::Fixed> TryFixed() const noexcept
    {
      if (auto *length = std::get_if<LengthPercentage>(&_value))
      {
        return length->TryFixed();
      }
      return {};
    }

    KRYS_NODISCARD bool IsFixed() const noexcept
    {
      if (auto *length = std::get_if<LengthPercentage>(&_value))
      {
        return length->IsFixed();
      }
      return false;
    }

    KRYS_NODISCARD bool IsCalculated() const noexcept
    {
      if (auto *length = std::get_if<LengthPercentage>(&_value))
      {
        return length->IsCalculated();
      }
      return false;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const MaskBorderWidthValue &) const noexcept = default;

    KRYS_NODISCARD bool HasSameType(const MaskBorderWidthValue &other) const noexcept
    {
      return _value.index() == other._value.index();
    }

  private:
    friend struct Blending<MaskBorderWidthValue>;

    Variant<css::keywords::Auto, LengthPercentage, Number> _value {css::keywords::Auto {}};
  };

  // <'mask-border-width'> = [ <length-percentage [0,∞]> | <number [0,∞]> | auto ]{1,4}
  // https://drafts.fxtf.org/css-masking-1/#propdef-mask-border-width
  struct MaskBorderWidth
  {
    using Value = MaskBorderWidthValue;
    using Edges = css::MinimallySerialisingSpaceSeparatedRectEdges<Value>;

    Edges Values {css::keywords::Auto {}};

    MaskBorderWidth(Edges values) noexcept : Values {krys::move(values)}
    {
    }
    MaskBorderWidth(Value top, Value right, Value bottom, Value left) noexcept
        : Values {krys::move(top), krys::move(right), krys::move(bottom), krys::move(left)}
    {
    }
    MaskBorderWidth(Value value) noexcept : Values {krys::move(value)}
    {
    }
    MaskBorderWidth(css::keywords::Auto keyword) noexcept : Values {keyword}
    {
    }
    MaskBorderWidth(Value::LengthPercentage &&lengthPercentage) noexcept
        : Values {krys::move(lengthPercentage)}
    {
    }
    MaskBorderWidth(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept : Values {literal}
    {
    }
    MaskBorderWidth(css::ValueLiteral<css::PercentageUnit::Percentage> literal) noexcept : Values {literal}
    {
    }
    MaskBorderWidth(Value::Number number) noexcept : Values {number}
    {
    }
    MaskBorderWidth(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept : Values {literal}
    {
    }

    KRYS_NODISCARD bool operator==(const MaskBorderWidth &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(MaskBorderWidth, Values);

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<MaskBorderWidth>
  {
    KRYS_NODISCARD MaskBorderWidth operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<MaskBorderWidth>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const MaskBorderWidth &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<MaskBorderWidthValue>
  {
    KRYS_NODISCARD bool CanBlend(const MaskBorderWidthValue &, const MaskBorderWidthValue &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const MaskBorderWidthValue &,
                                                                      const MaskBorderWidthValue &) noexcept;
    KRYS_NODISCARD MaskBorderWidthValue Blend(const MaskBorderWidthValue &, const MaskBorderWidthValue &,
                                              const BlendingContext &) noexcept;
  };

  template <>
  struct Blending<MaskBorderWidth>
  {
    KRYS_NODISCARD bool CanBlend(const MaskBorderWidth &, const MaskBorderWidth &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const MaskBorderWidth &,
                                                                      const MaskBorderWidth &) noexcept;
    KRYS_NODISCARD MaskBorderWidth Blend(const MaskBorderWidth &, const MaskBorderWidth &,
                                         const BlendingContext &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::MaskBorderWidth)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::MaskBorderWidthValueLength)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::MaskBorderWidthValue)
