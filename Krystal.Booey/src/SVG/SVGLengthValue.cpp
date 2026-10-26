#include "Krystal.Booey/SVG/SVGLengthValue.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <limits>

namespace krys::boo::svg
{
  namespace
  {
    KRYS_NODISCARD SVGLengthType CssLengthUnitToSVGLengthType(css::LengthPercentageUnit unit) noexcept
    {
      switch (unit)
      {
          // clang-format off
        case css::LengthPercentageUnit::Px:         return SVGLengthType::Pixels;
        case css::LengthPercentageUnit::Percentage: return SVGLengthType::Percentage;
        case css::LengthPercentageUnit::Em:         return SVGLengthType::Ems;
        case css::LengthPercentageUnit::Ex:         return SVGLengthType::Exs;
        case css::LengthPercentageUnit::Cm:         return SVGLengthType::Centimeters;
        case css::LengthPercentageUnit::Mm:         return SVGLengthType::Millimeters;
        case css::LengthPercentageUnit::In:         return SVGLengthType::Inches;
        case css::LengthPercentageUnit::Pt:         return SVGLengthType::Points;
        case css::LengthPercentageUnit::Pc:         return SVGLengthType::Picas;
        case css::LengthPercentageUnit::Lh:         return SVGLengthType::Lh;
        case css::LengthPercentageUnit::Ch:         return SVGLengthType::Ch;
        default:                                    return SVGLengthType::Unknown;
          // clang-format on
      }
    }

    KRYS_NODISCARD css::LengthPercentageUnit SVGLengthTypeToCSSLengthUnit(SVGLengthType type) noexcept
    {
      switch (type)
      {
          // clang-format off
        case SVGLengthType::Number:      return css::LengthPercentageUnit::Px;
        case SVGLengthType::Pixels:      return css::LengthPercentageUnit::Px;
        case SVGLengthType::Percentage:  return css::LengthPercentageUnit::Percentage;
        case SVGLengthType::Ems:         return css::LengthPercentageUnit::Em;
        case SVGLengthType::Exs:         return css::LengthPercentageUnit::Ex;
        case SVGLengthType::Centimeters: return css::LengthPercentageUnit::Cm;
        case SVGLengthType::Millimeters: return css::LengthPercentageUnit::Mm;
        case SVGLengthType::Inches:      return css::LengthPercentageUnit::In;
        case SVGLengthType::Points:      return css::LengthPercentageUnit::Pt;
        case SVGLengthType::Picas:       return css::LengthPercentageUnit::Pc;
        case SVGLengthType::Lh:          return css::LengthPercentageUnit::Lh;
        case SVGLengthType::Ch:          return css::LengthPercentageUnit::Ch;
        default:                         return css::LengthPercentageUnit::Px;
          // clang-format on
      }
    }

    KRYS_NODISCARD Variant<css::Number<>, css::LengthPercentage<>>
      CreateVariantForLengthType(float value, SVGLengthType lengthType) noexcept
    {
      if (lengthType == SVGLengthType::Number)
      {
        return css::Number<>(value);
      }

      if (lengthType == SVGLengthType::Unknown)
      {
        // For unknown types (like container units), fall back to Number
        // FIXME: Add support for container units
        return css::Number<>(value);
      }

      return css::LengthPercentage<>(SVGLengthTypeToCSSLengthUnit(lengthType), value);
    }
  }

  SVGLengthValue::SVGLengthValue(SVGLengthMode lengthMode, dom::DOMStringView valueAsString) noexcept
      : _value(css::Number<>(0)), _lengthMode(lengthMode)
  {
    (void)SetValueAsString(valueAsString);
  }

  SVGLengthValue::SVGLengthValue(float valueInSpecifiedUnits, SVGLengthType lengthType,
                                 SVGLengthMode lengthMode) noexcept
      : _value(CreateVariantForLengthType(valueInSpecifiedUnits, lengthType)), _lengthMode(lengthMode)
  {
  }

  SVGLengthValue::SVGLengthValue(const SVGLengthContext &context, float value, SVGLengthType lengthType,
                                 SVGLengthMode lengthMode) noexcept
      : _value(CreateVariantForLengthType(0, lengthType)), _lengthMode(lengthMode)
  {
    (void)SetValue(context, value);
  }

  SVGLengthValue SVGLengthValue::Construct(SVGLengthMode lengthMode, dom::DOMStringView valueAsString,
                                           SVGParsingError &parseError,
                                           SVGLengthNegativeValuesMode negativeValuesMode,
                                           Maybe<dom::DOMStringView> fallbackValue) noexcept
  {
    SVGLengthValue length(lengthMode);

    parseError = SVGParsingError::None;
    if (length.SetValueAsString(valueAsString).HasException())
    {
      parseError = SVGParsingError::ParsingFailed;
    }
    else if (negativeValuesMode == SVGLengthNegativeValuesMode::Forbid && length.ValueInSpecifiedUnits() < 0)
    {
      parseError = SVGParsingError::ForbiddenNegativeValue;
    }

    // If parsing failed or value is null, and we have a fallback, use it
    if (fallbackValue && (parseError != SVGParsingError::None || !valueAsString.data()))
    {
      return SVGLengthValue(lengthMode, *fallbackValue);
    }

    return length;
  }

  dom::ExceptionOr<void> SVGLengthValue::SetValueAsString(dom::DOMStringView valueAsString,
                                                          SVGLengthMode lengthMode) noexcept
  {
    _lengthMode = lengthMode;
    return SetValueAsString(valueAsString);
  }

  SVGLengthType SVGLengthValue::LengthType() const noexcept
  {
    return krys::SwitchOn(
      _value, [](const css::Number<> &) -> SVGLengthType { return SVGLengthType::Number; },
      [](const css::LengthPercentage<> &length) -> SVGLengthType
      {
        if (auto raw = length.GetRaw())
        {
          return CssLengthUnitToSVGLengthType(raw->Unit);
        }

        return SVGLengthType::Unknown;
      });
  }

  bool SVGLengthValue::IsZero() const noexcept
  {
    return krys::SwitchOn(_value, [](const auto &value) { return value.IsKnownZero(); });
  }

  bool SVGLengthValue::IsRelative() const noexcept
  {
    return krys::SwitchOn(
      _value, [](const css::Number<> &) { return false; },
      [](const css::LengthPercentage<> &length)
      {
        if (auto raw = length.GetRaw())
        {
          using Unit = css::LengthPercentageUnit;
          switch (raw->Unit)
          {
            case Unit::Percentage:
            case Unit::Em:
            case Unit::Ex:
            case Unit::Ch:
            case Unit::Lh:
            case Unit::Rem:
            case Unit::Rex:
            case Unit::Rlh:
            case Unit::Rch:
            {
              return true;
            }
            default:
            {
              return false;
            }
          }
        }

        return false;
      });
  }

  float SVGLengthValue::Value(const SVGLengthContext &context) const noexcept
  {
    auto result = ValueForBindings(context);
    if (result.HasException())
    {
      return 0;
    }
    return result.ReleaseValue();
  }

  float SVGLengthValue::ValueAsPercentage() const noexcept
  {
    return krys::SwitchOn(
      _value,
      [](const css::Number<> &number) -> float
      {
        if (auto raw = number.GetRaw())
        {
          return static_cast<float>(raw->Value);
        }

        return 0.0f;
      },
      [](const css::LengthPercentage<> &length) -> float
      {
        if (auto raw = length.GetRaw())
        {
          if (raw->Unit == css::LengthPercentageUnit::Percentage)
          {
            return static_cast<float>(raw->Value / 100.0f);
          }

          return static_cast<float>(raw->Value);
        }

        return 0.0f;
      });
  }

  float SVGLengthValue::ValueInSpecifiedUnits() const noexcept
  {
    // Per SVG spec: return 0 for non-scalar values like calc()
    // https://svgwg.org/svg2-draft/types.html#__svg__SVGLength__valueInSpecifiedUnits

    return krys::SwitchOn(_value,
                          [](const auto &value)
                          {
                            if (auto raw = value.GetRaw())
                            {
                              return ClampTo<float>(raw->Value);
                            }

                            return 0.f;
                          });
  }

  dom::DOMString SVGLengthValue::ValueAsString() const noexcept
  {
    return krys::SwitchOn(_value,
                          [](const auto &value)
                          {
                            if (auto raw = value.GetRaw())
                            {
                              // FIXME: Handle calc() expressions and consider exponential notation for very
                              // large/small values
                              float numericValue = ClampTo<float>(raw->Value);
                              return css::FormatCSSNumberValue(
                                css::SerialisableNumber {numericValue, css::UnitString(raw->Unit)});
                            }

                            return dom::DOMString();
                          });
  }

  dom::DOMStringAtom SVGLengthValue::ValueAsAtomString() const noexcept
  {
    return dom::DOMStringAtom(ValueAsString());
  }

  dom::ExceptionOr<float> SVGLengthValue::ValueForBindings(const SVGLengthContext &context) const noexcept
  {
    return krys::SwitchOn(
      _value,
      [&](const css::Number<> &number) -> dom::ExceptionOr<float>
      {
        if (auto raw = number.GetRaw())
        {
          return static_cast<float>(raw->Value);
        }

        return dom::Exception {dom::ExceptionCode::NotFoundError};
      },
      [&](const css::LengthPercentage<> &length) -> dom::ExceptionOr<float>
      {
        if (auto raw = length.GetRaw())
        {
          return context.ResolveValueToUserUnits(static_cast<float>(raw->Value), raw->Unit, _lengthMode);
        }

        return dom::Exception {dom::ExceptionCode::NotFoundError};
      });
  }

  void SVGLengthValue::SetValueInSpecifiedUnits(float value) noexcept
  {
    _value = krys::SwitchOn(
      _value, [&](const css::Number<> &) -> decltype(_value) { return css::Number<>(value); },
      [&](const css::LengthPercentage<> &current) -> decltype(_value)
      {
        if (auto raw = current.GetRaw())
        {
          return css::LengthPercentage<>(raw->Unit, value);
        }

        return css::Number<>(value);
      });
  }

  dom::ExceptionOr<void> SVGLengthValue::SetValue(const SVGLengthContext &context, float value) noexcept
  {
    return krys::SwitchOn(
      _value,
      [&](const css::Number<> &) -> dom::ExceptionOr<void>
      {
        _value = css::Number<>(value);
        return {};
      },
      [&](const css::LengthPercentage<> &current) -> dom::ExceptionOr<void>
      {
        if (auto raw = current.GetRaw())
        {
          auto resolvedValue = context.ResolveValueFromUserUnits(value, raw->Unit, _lengthMode);
          if (resolvedValue.HasException())
          {
            return resolvedValue.ReleaseException();
          }

          _value = resolvedValue.ReleaseValue();
          return {};
        }

        _value = css::Number<>(value);
        return {};
      });
  }

  dom::ExceptionOr<void> SVGLengthValue::SetValue(const SVGLengthContext &context, float value,
                                                  SVGLengthType lengthType, SVGLengthMode lengthMode) noexcept
  {
    // FIXME: Seems like a bug that we change the value of m_unit even if setValue throws an exception.
    _lengthMode = lengthMode;
    _value = CreateVariantForLengthType(value, lengthType);

    return SetValue(context, value);
  }

  dom::ExceptionOr<void> SVGLengthValue::SetValueAsString(dom::DOMStringView string) noexcept
  {
    if (string.empty())
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }

    // Trim leading and trailing whitespace to match SVG parsing expectations.
    auto trimmedString = infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(string);
    if (trimmedString.empty())
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }

    // CSS::Range only clamps to boundaries, but we historically handled
    // overflow values like "-45e58" to 0 instead of FLT_MAX.
    // FIXME: Consider setting to a proper value
    auto isFloatOverflow = [](const auto &parsedValue)
    {
      if (auto raw = parsedValue.GetRaw())
      {
        double value = raw->Value;
        return value > std::numeric_limits<float>::max() || value < -std::numeric_limits<float>::max();
      }
      return true;
    };

    auto parserContext = css::ParserContext {css::ParserMode::SVGAttribute};
    auto parserState = css::PropertyParserState {.Context = parserContext};

    css::InputStream input {css::CSSOMString(trimmedString)};
    css::Tokenizer tokenizer {input};
    tokenizer.PumpTokenizer();
    auto tokenRange = tokenizer.Tokens();

    if (auto number = css::MetaConsumer<css::Number<>>::Consume(tokenRange, parserState))
    {
      if (!tokenRange.IsAtEnd())
      {
        return dom::Exception {dom::ExceptionCode::SyntaxError};
      }

      _value = isFloatOverflow(*number) ? css::Number<>(0) : krys::move(*number);

      return {};
    }

    tokenRange = tokenizer.Tokens();
    if (auto length = css::MetaConsumer<css::LengthPercentage<>>::Consume(tokenRange, parserState))
    {
      if (!tokenRange.IsAtEnd())
      {
        return dom::Exception {dom::ExceptionCode::SyntaxError};
      }

      // FIXME: Add support for calculated lengths.
      if (length->IsCalc())
      {
        return dom::Exception {dom::ExceptionCode::SyntaxError};
      }

      _value = krys::move(*length);

      return {};
    }

    return dom::Exception {dom::ExceptionCode::SyntaxError};
  }

  dom::ExceptionOr<void> SVGLengthValue::ConvertToSpecifiedUnits(const SVGLengthContext &context,
                                                                 SVGLengthType targetType) noexcept
  {
    auto valueInUserUnits = ValueForBindings(context);
    if (valueInUserUnits.HasException())
    {
      return valueInUserUnits.ReleaseException();
    }

    float userUnits = valueInUserUnits.ReleaseValue();

    if (targetType == SVGLengthType::Number)
    {
      _value = css::Number<>(userUnits);
      return {};
    }

    auto convertedValue =
      context.ResolveValueFromUserUnits(userUnits, SVGLengthTypeToCSSLengthUnit(targetType), _lengthMode);

    if (convertedValue.HasException())
    {
      return convertedValue.ReleaseException();
    }

    _value = convertedValue.ReleaseValue();
    return {};
  }
}
