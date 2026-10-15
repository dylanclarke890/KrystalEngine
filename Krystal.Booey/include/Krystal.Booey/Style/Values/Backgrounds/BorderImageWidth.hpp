#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  struct BorderImageWidthValueLength : LengthWrapperBase<LengthPercentage<css::NonNegative>>
  {
    using Base::Base;
  };

  // <border-image-width-value> = <length-percentage [0,∞]> | <number [0,∞]> | auto
  struct BorderImageWidthValue
  {
    using LengthPercentage = BorderImageWidthValueLength;
    using Number = style::Number<css::NonNegative, float>;

    BorderImageWidthValue(css::keywords::Auto keyword) noexcept : _value {keyword}
    {
    }
    BorderImageWidthValue(LengthPercentage &&length) noexcept : _value {krys::move(length)}
    {
    }
    BorderImageWidthValue(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept
        : _value {LengthPercentage {literal}}
    {
    }
    BorderImageWidthValue(css::ValueLiteral<css::PercentageUnit::Percentage> literal) noexcept
        : _value {LengthPercentage {literal}}
    {
    }
    BorderImageWidthValue(Number number) noexcept : _value {number}
    {
    }
    BorderImageWidthValue(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept
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

    KRYS_NODISCARD bool operator==(const BorderImageWidthValue &) const noexcept = default;

    KRYS_NODISCARD bool HasSameType(const BorderImageWidthValue &other) const noexcept
    {
      return _value.index() == other._value.index();
    }

  private:
    friend struct Blending<BorderImageWidthValue>;

    Variant<css::keywords::Auto, LengthPercentage, Number> _value {Number {1}};
  };

  // <'border-image-width'> = [ <length-percentage [0,∞]> | <number [0,∞]> | auto ]{1,4}
  // https://drafts.csswg.org/css-backgrounds/#propdef-border-image-width
  struct BorderImageWidth
  {
    using Value = BorderImageWidthValue;
    using Edges = css::MinimallySerialisingSpaceSeparatedRectEdges<Value>;

    Edges Values {Value::Number {1}};
    bool LegacyWebkitBorderImage {false};

    BorderImageWidth(Edges values, bool legacyWebkitBorderImage = false) noexcept
        : Values {krys::move(values)}, LegacyWebkitBorderImage {legacyWebkitBorderImage}
    {
    }
    BorderImageWidth(Value top, Value right, Value bottom, Value left,
                     bool legacyWebkitBorderImage = false) noexcept
        : Values {krys::move(top), krys::move(right), krys::move(bottom), krys::move(left)},
          LegacyWebkitBorderImage {legacyWebkitBorderImage}
    {
    }
    BorderImageWidth(Value value, bool legacyWebkitBorderImage = false) noexcept
        : Values {krys::move(value)}, LegacyWebkitBorderImage {legacyWebkitBorderImage}
    {
    }
    BorderImageWidth(css::keywords::Auto keyword, bool legacyWebkitBorderImage = false) noexcept
        : Values {keyword}, LegacyWebkitBorderImage {legacyWebkitBorderImage}
    {
    }
    BorderImageWidth(Value::LengthPercentage &&lengthPercentage,
                     bool legacyWebkitBorderImage = false) noexcept
        : Values {krys::move(lengthPercentage)}, LegacyWebkitBorderImage {legacyWebkitBorderImage}
    {
    }
    BorderImageWidth(css::ValueLiteral<css::LengthUnit::Px> literal,
                     bool legacyWebkitBorderImage = false) noexcept
        : Values {literal}, LegacyWebkitBorderImage {legacyWebkitBorderImage}
    {
    }
    BorderImageWidth(css::ValueLiteral<css::PercentageUnit::Percentage> literal,
                     bool legacyWebkitBorderImage = false) noexcept
        : Values {literal}, LegacyWebkitBorderImage {legacyWebkitBorderImage}
    {
    }
    BorderImageWidth(Value::Number number, bool legacyWebkitBorderImage = false) noexcept
        : Values {number}, LegacyWebkitBorderImage {legacyWebkitBorderImage}
    {
    }
    BorderImageWidth(css::ValueLiteral<css::NumberUnit::Number> literal,
                     bool legacyWebkitBorderImage = false) noexcept
        : Values {literal}, LegacyWebkitBorderImage {legacyWebkitBorderImage}
    {
    }

    KRYS_NODISCARD bool OverridesBorderWidths() const noexcept
    {
      return LegacyWebkitBorderImage;
    }

    KRYS_NODISCARD bool operator==(const BorderImageWidth &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(BorderImageWidth, Values);

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<BorderImageWidth>
  {
    KRYS_NODISCARD BorderImageWidth operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<BorderImageWidth>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const BorderImageWidth &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<BorderImageWidthValue>
  {
    KRYS_NODISCARD bool CanBlend(const BorderImageWidthValue &, const BorderImageWidthValue &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const BorderImageWidthValue &,
                                                                      const BorderImageWidthValue &) noexcept;
    KRYS_NODISCARD BorderImageWidthValue Blend(const BorderImageWidthValue &, const BorderImageWidthValue &,
                                               const BlendingContext &) noexcept;
  };

  template <>
  struct Blending<BorderImageWidth>
  {
    KRYS_NODISCARD bool CanBlend(const BorderImageWidth &, const BorderImageWidth &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const BorderImageWidth &,
                                                                      const BorderImageWidth &) noexcept;
    KRYS_NODISCARD BorderImageWidth Blend(const BorderImageWidth &, const BorderImageWidth &,
                                          const BlendingContext &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::BorderImageWidth)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::BorderImageWidthValueLength)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::BorderImageWidthValue)
