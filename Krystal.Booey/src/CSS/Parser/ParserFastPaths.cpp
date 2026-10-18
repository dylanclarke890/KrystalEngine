#include "Krystal.Booey/CSS/Parser/ParserFastPaths.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParsing.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyShorthand.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorDescriptors.hpp"
#include "Krystal.Booey/CSS/Values/Color/Convert/ToColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/Convert/ToTypedColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/GlobalKeyword.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Units.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/TransformListValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Graphics/Color/NamedColors.hpp"
#include "Krystal.Core/Colour/ColourUtilities.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <array>
#include <charconv>
#include <cmath>

namespace krys::boo::css
{
  Maybe<Range> ParserFastPaths::LengthValueRangeForPropertiesSupportingSimpleLengths(PropertyId id) noexcept
  {
    switch (id)
    {
      case PropertyId::FontSize:
      case PropertyId::Height:
      case PropertyId::Width:
      case PropertyId::MinHeight:
      case PropertyId::MinWidth:
      case PropertyId::PaddingBottom:
      case PropertyId::PaddingLeft:
      case PropertyId::PaddingRight:
      case PropertyId::PaddingTop:
      case PropertyId::InlineSize:
      case PropertyId::BlockSize:
      case PropertyId::MinInlineSize:
      case PropertyId::MinBlockSize:
      case PropertyId::PaddingBlockEnd:
      case PropertyId::PaddingBlockStart:
      case PropertyId::PaddingInlineEnd:
      case PropertyId::PaddingInlineStart:
      case PropertyId::R:
      case PropertyId::Rx:
      case PropertyId::Ry:
      case PropertyId::ShapeMargin:
      {
        return NonNegative;
      }
      case PropertyId::Bottom:
      case PropertyId::Cx:
      case PropertyId::Cy:
      case PropertyId::Left:
      case PropertyId::InsetBlockEnd:
      case PropertyId::InsetBlockStart:
      case PropertyId::InsetInlineEnd:
      case PropertyId::InsetInlineStart:
      case PropertyId::MarginBottom:
      case PropertyId::MarginLeft:
      case PropertyId::MarginRight:
      case PropertyId::MarginTop:
      case PropertyId::Right:
      case PropertyId::Top:
      case PropertyId::MarginBlockEnd:
      case PropertyId::MarginBlockStart:
      case PropertyId::MarginInlineEnd:
      case PropertyId::MarginInlineStart:
      case PropertyId::X:
      case PropertyId::Y:
      {
        return All;
      }
      default:
      {
        return null;
      }
    }
  }

  namespace
  {
    KRYS_NODISCARD bool SkipExactly(CSSOMStringView &string, char8_t character) noexcept
    {
      if (!string.empty() && string.front() == character)
      {
        string.remove_prefix(1);
        return true;
      }

      return false;
    }

    template <typename Predicate>
    void SkipWhile(CSSOMStringView &string, Predicate &&predicate) noexcept
    {
      while (!string.empty() && predicate(string.front()))
      {
        string.remove_prefix(1);
      }
    }

    KRYS_NODISCARD Maybe<double> ParseCSSNumber(CSSOMStringView characters) noexcept
    {
      // A trailing '.' is not allowed in CSS number values.
      if (!characters.empty() && characters.back() == u8'.')
      {
        return null;
      }

      // FIXME: If we don't want to skip over leading spaces, we should use parseDouble, not
      // charactersToDouble.
      SkipWhile(characters, [](char8_t c) { return text::IsASCIIWhitespace(c); });

      auto begin = reinterpret_cast<const char *>(characters.data());
      auto end = begin + characters.size();

      double number = 0;
      auto [ptr, ec] = std::from_chars(begin, end, number);
      if (ec != std::errc {} || ptr == begin || ptr != end)
      {
        return null;
      }

      return number;
    }

    enum class RequireUnits : bool
    {
      No,
      Yes
    };

    KRYS_NODISCARD bool ParseSimpleAngle(CSSOMStringView characters, RequireUnits requireUnits,
                                         AngleUnit &unit, double &number) noexcept
    {
      // "0deg" or "1rad"
      if (characters.size() >= 4)
      {
        auto size = characters.size();
        if (text::IsASCIICaselessEqual(characters[size - 3], 'd')
            && text::IsASCIICaselessEqual(characters[size - 2], 'e')
            && text::IsASCIICaselessEqual(characters[size - 1], 'g'))
        {
          characters.remove_suffix(3);
          unit = AngleUnit::Deg;
        }
        else if (text::IsASCIICaselessEqual(characters[size - 3], 'r')
                 && text::IsASCIICaselessEqual(characters[size - 2], 'a')
                 && text::IsASCIICaselessEqual(characters[size - 1], 'd'))
        {
          characters.remove_suffix(3);
          unit = AngleUnit::Rad;
        }
        else if (requireUnits == RequireUnits::Yes)
        {
          return false;
        }
      }
      else
      {
        if (requireUnits == RequireUnits::Yes || characters.empty())
        {
          return false;
        }

        unit = AngleUnit::Deg;
      }

      auto parsedNumber = ParseCSSNumber(characters);
      number = parsedNumber.value_or(0);
      return parsedNumber.has_value();
    }

    KRYS_NODISCARD bool ParseSimpleLength(CSSOMStringView characters, UnitType &unit, double &number) noexcept
    {
      if (characters.size() > 2 && text::IsASCIICaselessEqual(characters[characters.size() - 2], 'p')
          && text::IsASCIICaselessEqual(characters[characters.size() - 1], 'x'))
      {
        characters.remove_suffix(2);
        unit = UnitType::px;
      }
      else if (!characters.empty() && characters.back() == u8'%')
      {
        characters.remove_suffix(1);
        unit = UnitType::Percentage;
      }

      auto parsedNumber = ParseCSSNumber(characters);
      number = parsedNumber.value_or(0);
      return parsedNumber.has_value();
    }

    KRYS_NODISCARD bool ParseSimpleNumberOrPercentageDividedBy100(CSSOMStringView characters,
                                                                  double &number) noexcept
    {
      bool isPercentage = false;
      if (!characters.empty() && characters.back() == u8'%')
      {
        characters.remove_suffix(1);
        isPercentage = true;
      }

      auto parsedNumber = ParseCSSNumber(characters);
      if (!parsedNumber)
      {
        return false;
      }

      number = *parsedNumber;
      if (std::isinf(number))
      {
        return false;
      }

      if (isPercentage)
      {
        number /= 100.0;
      }

      return true;
    }

    /// @brief Returns the number of characters which form a valid double and are terminated by the given
    /// terminator character.
    KRYS_NODISCARD size_t CheckForValidDouble(CSSOMStringView string, char8_t terminator) noexcept
    {
      if (string.empty())
      {
        return 0;
      }

      Maybe<size_t> decimalMarkOffset;
      size_t processedLength = 0;

      for (size_t i = 0; i < string.size(); ++i)
      {
        if (string[i] == terminator)
        {
          processedLength = i;
          break;
        }

        if (!text::IsASCIIDigit(string[i]))
        {
          if (!decimalMarkOffset && string[i] == u8'.')
          {
            decimalMarkOffset = i;
          }
          else
          {
            return 0;
          }
        }
      }

      // CSS disallows a period without a subsequent digit.
      if (decimalMarkOffset && *decimalMarkOffset == processedLength - 1)
      {
        return 0;
      }

      return processedLength;
    }

    /// @brief Returns the number of characters consumed for parsing a valid double terminated by the given
    /// terminator character.
    KRYS_NODISCARD size_t ParseDouble(CSSOMStringView string, char8_t terminator, double &value) noexcept
    {
      size_t length = CheckForValidDouble(string, terminator);
      if (!length)
      {
        return 0;
      }

      size_t position = 0;
      double localValue = 0;

      // The consumed characters here are guaranteed to be ASCII digits with or without a decimal mark.
      for (; position < length; ++position)
      {
        if (string[position] == u8'.')
        {
          break;
        }
        localValue = localValue * 10 + string[position] - u8'0';
      }

      if (++position == length)
      {
        value = localValue;
        return length;
      }

      double fraction = 0;
      double scale = 1;

      double maxScale = 1'000'000;
      while (position < length && scale < maxScale)
      {
        fraction = fraction * 10 + string[position++] - u8'0';
        scale *= 10;
      }

      value = localValue + fraction / scale;
      return length;
    }

    KRYS_NODISCARD Maybe<uint8> ParseColorIntOrPercentage(CSSOMStringView &string,
                                                          Maybe<char8_t> consumableTerminator,
                                                          UnitType &expectedUnitType) noexcept
    {
      auto current = string;
      double localValue = 0;
      bool negative = false;
      SkipWhile(current, [](char8_t c) { return text::IsASCIIWhitespace(c); });

      if (SkipExactly(current, u8'-'))
      {
        negative = true;
      }

      if (current.empty() || !text::IsASCIIDigit(current.front()))
      {
        return null;
      }

      while (!current.empty() && text::IsASCIIDigit(current.front()))
      {
        double newValue = localValue * 10 + current.front() - u8'0';
        current.remove_prefix(1);
        if (newValue >= 255)
        {
          // Clamp values at 255.
          localValue = 255;
          SkipWhile(current, [](char8_t c) { return text::IsASCIIDigit(c); });
          break;
        }
        localValue = newValue;
      }

      if (current.empty())
      {
        return null;
      }

      if (expectedUnitType == UnitType::Number && (current.front() == u8'.' || current.front() == u8'%'))
      {
        return null;
      }

      if (current.front() == u8'.')
      {
        // We already parsed the integral part, try to parse
        // the fraction part of the percentage value.
        double percentage = 0;
        size_t numCharactersParsed = ParseDouble(current, u8'%', percentage);
        if (!numCharactersParsed)
        {
          return null;
        }
        current.remove_prefix(numCharactersParsed);
        if (current.front() != u8'%')
        {
          return null;
        }
        localValue += percentage;
      }

      if (expectedUnitType == UnitType::Percentage && current.front() != u8'%')
      {
        return null;
      }

      if (SkipExactly(current, u8'%'))
      {
        expectedUnitType = UnitType::Percentage;
        localValue = localValue / 100.0 * 255.0;
        // Clamp values at 255 for percentages over 100%
        if (localValue > 255)
        {
          localValue = 255;
        }
      }
      else
      {
        expectedUnitType = UnitType::Number;
      }

      SkipWhile(current, [](char8_t c) { return text::IsASCIIWhitespace(c); });

      if (consumableTerminator && !SkipExactly(current, *consumableTerminator))
      {
        return null;
      }

      string = current;

      // Clamp negative values at zero.
      krys_debug_assert(localValue <= 255);
      return negative ? uint8 {0} : ConvertPrescaledSRGBAFloatToSRGBAByte(static_cast<float>(localValue));
    }

    KRYS_NODISCARD bool IsTenthAlpha(CSSOMStringView string) noexcept
    {
      // "0.X"
      if (string.size() == 3 && string[0] == u8'0' && string[1] == u8'.' && text::IsASCIIDigit(string[2]))
      {
        return true;
      }

      // ".X"
      if (string.size() == 2 && string[0] == u8'.' && text::IsASCIIDigit(string[1]))
      {
        return true;
      }

      return false;
    }

    KRYS_NODISCARD Maybe<uint8> ParseRGBAlphaValue(CSSOMStringView &string, char8_t terminator) noexcept
    {
      SkipWhile(string, [](char8_t c) { return text::IsASCIIWhitespace(c); });

      bool negative = false;

      if (SkipExactly(string, u8'-'))
      {
        negative = true;
      }

      size_t length = string.size();
      if (length < 2)
      {
        return null;
      }

      if (string[length - 1] != terminator || !text::IsASCIIDigit(string[length - 2]))
      {
        return null;
      }

      if (string.front() != u8'0' && string.front() != u8'1' && string.front() != u8'.')
      {
        if (CheckForValidDouble(string, terminator))
        {
          string = {};
          return negative ? uint8 {0} : uint8 {255};
        }
        return null;
      }

      if (length == 2 && string.front() != u8'.')
      {
        uint8 result = !negative && string.front() == u8'1' ? 255 : 0;
        string = {};
        return result;
      }

      if (IsTenthAlpha(string.substr(0, length - 1)))
      {
        constexpr std::array<uint8, 10> tenthAlphaValues {0, 26, 51, 77, 102, 128, 153, 179, 204, 230};
        uint8 result = negative ? uint8 {0} : tenthAlphaValues[string[length - 2] - u8'0'];
        string = {};
        return result;
      }

      double alpha = 0;
      if (!ParseDouble(string, terminator, alpha))
      {
        return null;
      }

      string = {};
      return negative ? uint8 {0} : ConvertFloatAlphaTo<uint8>(static_cast<float>(alpha));
    }

    KRYS_NODISCARD bool MightBeRGBA(CSSOMStringView characters) noexcept
    {
      if (characters.size() < 5)
      {
        return false;
      }
      return characters[4] == u8'(' && text::IsASCIICaselessEqual(characters[0], 'r')
             && text::IsASCIICaselessEqual(characters[1], 'g')
             && text::IsASCIICaselessEqual(characters[2], 'b')
             && text::IsASCIICaselessEqual(characters[3], 'a');
    }

    KRYS_NODISCARD bool MightBeRGB(CSSOMStringView characters) noexcept
    {
      if (characters.size() < 4)
      {
        return false;
      }
      return characters[3] == u8'(' && text::IsASCIICaselessEqual(characters[0], 'r')
             && text::IsASCIICaselessEqual(characters[1], 'g')
             && text::IsASCIICaselessEqual(characters[2], 'b');
    }

    KRYS_NODISCARD bool MightBeHSLA(CSSOMStringView characters) noexcept
    {
      if (characters.size() < 5)
      {
        return false;
      }
      return characters[4] == u8'(' && text::IsASCIICaselessEqual(characters[0], 'h')
             && text::IsASCIICaselessEqual(characters[1], 's')
             && text::IsASCIICaselessEqual(characters[2], 'l')
             && text::IsASCIICaselessEqual(characters[3], 'a');
    }

    KRYS_NODISCARD bool MightBeHSL(CSSOMStringView characters) noexcept
    {
      if (characters.size() < 4)
      {
        return false;
      }
      return characters[3] == u8'(' && text::IsASCIICaselessEqual(characters[0], 'h')
             && text::IsASCIICaselessEqual(characters[1], 's')
             && text::IsASCIICaselessEqual(characters[2], 'l');
    }

    KRYS_NODISCARD Maybe<SRGBA<uint8>> FinishParsingHexColor(uint32 value, size_t length) noexcept
    {
      switch (length)
      {
        case 3:
        {
          // #234 converts to #223344.
          auto r = static_cast<uint8>((value & 0x0F00) >> 8);
          auto g = static_cast<uint8>((value & 0x00F0) >> 4);
          auto b = static_cast<uint8>(value & 0x000F);
          return SRGBA<uint8>(static_cast<uint8>(r << 4 | r), static_cast<uint8>(g << 4 | g),
                              static_cast<uint8>(b << 4 | b));
        }
        case 4:
        {
          // #234a converts to #223344aa.
          auto r = static_cast<uint8>((value & 0xF000) >> 12);
          auto g = static_cast<uint8>((value & 0x0F00) >> 8);
          auto b = static_cast<uint8>((value & 0x00F0) >> 4);
          auto a = static_cast<uint8>(value & 0x000F);
          return SRGBA<uint8>(static_cast<uint8>(r << 4 | r), static_cast<uint8>(g << 4 | g),
                              static_cast<uint8>(b << 4 | b), static_cast<uint8>(a << 4 | a));
        }
        case 6:
        {
          return AsSRGBA(PackedColour::ARGB {0xFF'00'00'00 | value});
        }
        case 8:
        {
          return AsSRGBA(PackedColour::RGBA {value});
        }
        default:
        {
          return null;
        }
      }
    }

    KRYS_NODISCARD Maybe<SRGBA<uint8>> ParseHexColorInternal(CSSOMStringView characters) noexcept
    {
      if (characters.size() != 3 && characters.size() != 4 && characters.size() != 6
          && characters.size() != 8)
      {
        return null;
      }

      uint32 value = 0;
      for (auto digit : characters)
      {
        if (!text::IsASCIIHexDigit(digit))
        {
          return null;
        }
        value <<= 4;
        value |= text::ToASCIIHexValue(digit);
      }

      return FinishParsingHexColor(value, characters.size());
    }

    KRYS_NODISCARD Maybe<SRGBA<uint8>> ParseLegacyHSL(CSSOMStringView characters) noexcept
    {
      // Commas only exist in the legacy syntax.
      size_t delimiter = characters.find(u8',');
      if (delimiter == CSSOMStringView::npos)
      {
        return null;
      }

      auto skipWhitespace = [](CSSOMStringView &characters)
      {
        SkipWhile(characters, [](char8_t c) { return IsCSSSpace(c); });
      };

      auto parsePercentageWithOptionalLeadingWhitespace = [&](CSSOMStringView &characters) -> Maybe<double>
      {
        skipWhitespace(characters);

        double value = 0;
        size_t numCharactersParsed = ParseDouble(characters, u8'%', value);
        if (!numCharactersParsed)
        {
          return null;
        }

        characters.remove_prefix(numCharactersParsed);
        if (!SkipExactly(characters, u8'%'))
        {
          return null;
        }

        return value;
      };

      auto skipComma = [](CSSOMStringView &characters)
      {
        return SkipExactly(characters, u8',');
      };

      double hue = 0;
      auto angleChars = characters.substr(0, delimiter);
      auto angleUnit = AngleUnit::Deg;
      if (!ParseSimpleAngle(angleChars, RequireUnits::No, angleUnit, hue))
      {
        return null;
      }

      characters.remove_prefix(delimiter);
      if (!skipComma(characters))
      {
        return null;
      }

      auto saturation = parsePercentageWithOptionalLeadingWhitespace(characters);
      if (!saturation)
      {
        return null;
      }

      if (!skipComma(characters))
      {
        return null;
      }

      auto lightness = parsePercentageWithOptionalLeadingWhitespace(characters);
      if (!lightness)
      {
        return null;
      }

      auto parseAlpha = [&](CSSOMStringView &characters) -> Maybe<double>
      {
        skipWhitespace(characters);

        size_t numCharactersParsed;
        double alpha = 1;
        if ((numCharactersParsed = ParseDouble(characters, u8')', alpha)))
        {
          characters.remove_prefix(numCharactersParsed);
          return alpha;
        }

        if ((numCharactersParsed = ParseDouble(characters, u8'%', alpha)))
        {
          characters.remove_prefix(numCharactersParsed + 1); // Skip the '%'
          return alpha / 100.0;
        }

        return null;
      };

      double alpha = 1.0;
      // Alpha is optional for both hsl() and hsla().
      if (skipComma(characters))
      {
        auto alphaValue = parseAlpha(characters);
        if (!alphaValue)
        {
          return null;
        }

        alpha = *alphaValue;
      }

      skipWhitespace(characters);

      if (characters.empty() || characters.front() != u8')')
      {
        return null;
      }

      auto parsedColor = StyleColorParseType<HSLFunctionLegacy> {
        style::Angle<> {static_cast<float>(ConvertAngle<AngleUnit::Deg>(hue, angleUnit))},
        style::Percentage<> {static_cast<float>(*saturation)},
        style::Percentage<> {static_cast<float>(*lightness)},
        style::Number<> {static_cast<float>(alpha)},
      };
      auto typedColor = ConvertToTypedColor<HSLFunctionLegacy>(parsedColor, 1.0);
      auto resultColor = ConvertToColor<HSLFunctionLegacy, ColorFunctionForm::Absolute>(typedColor, 0);
      return resultColor.TryGetAsSRGBABytes();
    }

    KRYS_NODISCARD Maybe<SRGBA<uint8>> ParseNumericColor(CSSOMStringView characters,
                                                         const ParserContext &context) noexcept
    {
      if (characters.size() >= 4 && characters.front() == u8'#')
      {
        if (auto hexColor = ParseHexColorInternal(characters.substr(1)))
        {
          return *hexColor;
        }
      }

      if (IsQuirksModeBehavior(context.Mode) && (characters.size() == 3 || characters.size() == 6))
      {
        if (auto hexColor = ParseHexColorInternal(characters))
        {
          return *hexColor;
        }
      }

      if (MightBeRGB(characters) || MightBeRGBA(characters))
      {
        auto expectedUnitType = UnitType::Unknown;
        auto current = MightBeRGBA(characters) ? characters.substr(5) : characters.substr(4);

        // Red and green will both terminate with ','.
        auto red = ParseColorIntOrPercentage(current, u8',', expectedUnitType);
        if (!red)
        {
          return null;
        }
        auto green = ParseColorIntOrPercentage(current, u8',', expectedUnitType);
        if (!green)
        {
          return null;
        }

        // Blue may terminate with ',' or ')', but do not consume either terminator.
        auto blue = ParseColorIntOrPercentage(current, null, expectedUnitType);
        if (!blue)
        {
          return null;
        }
        if (current.empty())
        {
          return null;
        }

        // Finish parsing rgb if no alpha value.
        if (current.front() == u8')')
        {
          current.remove_prefix(1);
          if (!current.empty())
          {
            return null;
          }
          return SRGBA<uint8> {*red, *green, *blue};
        }

        // Parse alpha value if present.
        if (current.front() == u8',')
        {
          current.remove_prefix(1);
          auto alpha = ParseRGBAlphaValue(current, u8')');
          if (!alpha)
          {
            return null;
          }
          if (!current.empty())
          {
            return null;
          }
          return SRGBA<uint8> {*red, *green, *blue, *alpha};
        }

        return null;
      }

      // hsl() and hsla() are synonyms.
      if (MightBeHSLA(characters))
      {
        return ParseLegacyHSL(characters.substr(5));
      }

      if (MightBeHSL(characters))
      {
        return ParseLegacyHSL(characters.substr(4));
      }

      return null;
    }

    KRYS_NODISCARD Maybe<SRGBA<uint8>> ParseNamedColorInternal(CSSOMStringView characters) noexcept
    {
      std::array<char8_t, 64> buffer; // Easily big enough for the longest color name.
      if (characters.size() > buffer.size() - 1)
      {
        return null;
      }

      for (size_t i = 0; i < characters.size(); ++i)
      {
        auto character = characters[i];
        if (!character || !text::IsASCII(character))
        {
          return null;
        }
        buffer[i] = text::ToASCIILower(character);
      }

      auto namedColor = FindColor(std::u8string_view {buffer.data(), characters.size()});
      if (!namedColor)
      {
        return null;
      }

      return AsSRGBA(PackedColour::ARGB {namedColor->ARGBValue});
    }
  }

  Maybe<SRGBA<uint8>> ParserFastPaths::ParseSimpleColor(CSSOMStringView string,
                                                        const ParserContext &context) noexcept
  {
    if (auto color = ParseNumericColor(string, context))
    {
      return color;
    }
    return ParseNamedColorInternal(string);
  }

  Maybe<SRGBA<uint8>> ParserFastPaths::ParseHexColor(CSSOMStringView string) noexcept
  {
    return ParseHexColorInternal(string);
  }

  Maybe<SRGBA<uint8>> ParserFastPaths::ParseNamedColor(CSSOMStringView string) noexcept
  {
    return ParseNamedColorInternal(string);
  }

  namespace
  {
    KRYS_NODISCARD RefPtr<Value> ParseSimpleLengthValue(CSSOMStringView string, ParserMode parserMode,
                                                        Range valueRange) noexcept
    {
      krys_debug_assert(!string.empty());

      double number;
      auto unit = UnitType::Number;
      if (!ParseSimpleLength(string, unit, number))
      {
        return nullptr;
      }

      if (unit == UnitType::Number)
      {
        if (number && parserMode != ParserMode::SVGAttribute)
        {
          return nullptr;
        }

        unit = UnitType::px;
      }

      if (number < valueRange.Min || number > valueRange.Max)
      {
        return nullptr;
      }

      if (std::isinf(number))
      {
        return nullptr;
      }

      return PrimitiveValue::Create(number, unit);
    }

    KRYS_NODISCARD RefPtr<Value> ParseColor(CSSOMStringView string, const ParserContext &context) noexcept
    {
      krys_debug_assert(!string.empty());

      auto valueId = ValueIdFromName(string);
      if (IsColorKeyword(valueId))
      {
        if (!PropertyParserHelpers::IsColorKeywordAllowed(valueId, context))
        {
          return nullptr;
        }

        return PrimitiveValue::Create(valueId);
      }

      if (auto color = ParseNumericColor(string, context))
      {
        return ValuePool::MainThreadPool().CreateColorValue(*color);
      }

      return nullptr;
    }

    KRYS_NODISCARD bool IsUniversalKeyword(CSSOMStringView string) noexcept
    {
      // These keywords can be used for all properties.
      return text::IsASCIICaselessEqual(string, CSSOMStringView {u8"initial"})
             || text::IsASCIICaselessEqual(string, CSSOMStringView {u8"inherit"})
             || text::IsASCIICaselessEqual(string, CSSOMStringView {u8"unset"})
             || text::IsASCIICaselessEqual(string, CSSOMStringView {u8"revert"})
             || text::IsASCIICaselessEqual(string, CSSOMStringView {u8"revert-layer"});
    }

    KRYS_NODISCARD RefPtr<Value> ParseKeywordValue(PropertyId property, CSSOMStringView string,
                                                   PropertyParserState &state) noexcept
    {
      krys_debug_assert(!string.empty());

      if (!PropertyParsing::IsKeywordFastPathEligibleStyleProperty(property))
      {
        // All properties, including non-keyword properties, accept the CSS-wide keywords.
        if (!IsUniversalKeyword(string))
        {
          return nullptr;
        }

        // Leave shorthands to parse CSS-wide keywords using PropertyParser.
        if (ShorthandForProperty(property).Size())
        {
          return nullptr;
        }
      }

      auto valueId = ValueIdFromName(string);
      if (valueId == ValueId::Invalid)
      {
        return nullptr;
      }

      if (IsGlobalKeyword(valueId))
      {
        return PrimitiveValue::Create(valueId);
      }

      if (PropertyParsing::IsKeywordValidForStyleProperty(property, valueId, state))
      {
        return PrimitiveValue::Create(valueId);
      }

      return nullptr;
    }

    KRYS_NODISCARD bool ParseTransformTranslateArguments(CSSOMStringView &buffer, size_t expectedCount,
                                                         ValueId transformType,
                                                         ValueListBuilder &arguments) noexcept
    {
      while (expectedCount)
      {
        auto delimiter = buffer.find(expectedCount == 1uz ? u8')' : u8',');
        if (delimiter == CSSOMStringView::npos)
        {
          return false;
        }

        auto unit = UnitType::Number;
        double number;
        if (!ParseSimpleLength(buffer.substr(0, delimiter), unit, number))
        {
          return false;
        }

        if (!number && unit == UnitType::Number)
        {
          unit = UnitType::px;
        }

        if (unit == UnitType::Number
            || (unit == UnitType::Percentage
                && (transformType == ValueId::TranslateZ
                    || (transformType == ValueId::Translate3d && expectedCount == 1uz))))
        {
          return false;
        }

        arguments.push_back(PrimitiveValue::Create(number, unit));
        buffer.remove_prefix(delimiter + 1);
        --expectedCount;
      }

      return true;
    }

    KRYS_NODISCARD RefPtr<Value> ParseTransformAngleArgument(CSSOMStringView &buffer) noexcept
    {
      auto delimiter = buffer.find(u8')');
      if (delimiter == CSSOMStringView::npos)
      {
        return nullptr;
      }

      auto angleUnit = AngleUnit::Deg;
      double number;
      if (!ParseSimpleAngle(buffer.substr(0, delimiter), RequireUnits::Yes, angleUnit, number))
      {
        return nullptr;
      }

      buffer.remove_prefix(delimiter + 1);

      return PrimitiveValue::Create(number, ToUnitType(angleUnit));
    }

    KRYS_NODISCARD bool ParseTransformNumberArguments(CSSOMStringView &buffer, size_t expectedCount,
                                                      ValueListBuilder &arguments) noexcept
    {
      while (expectedCount)
      {
        auto delimiter = buffer.find(expectedCount == 1uz ? u8')' : u8',');
        if (delimiter == CSSOMStringView::npos)
        {
          return false;
        }

        auto number = ParseCSSNumber(buffer.substr(0, delimiter));
        if (!number)
        {
          return false;
        }

        arguments.push_back(PrimitiveValue::Create(*number, UnitType::Number));
        buffer.remove_prefix(delimiter + 1);
        --expectedCount;
      }

      return true;
    }

    /// @brief Whether `buffer` starts with `prefix`, ignoring ASCII case. `prefix` must be lowercase.
    KRYS_NODISCARD bool StartsWithLettersIgnoringASCIICase(CSSOMStringView buffer,
                                                           CSSOMStringView prefix) noexcept
    {
      return buffer.size() >= prefix.size()
             && text::IsASCIICaselessEqual(buffer.substr(0, prefix.size()), prefix);
    }

    KRYS_NODISCARD RefPtr<FunctionValue> ParseSimpleTransformValue(CSSOMStringView &buffer) noexcept
    {
      // Also guarantees indexes up to buffer[8] are safe to access below.
      constexpr auto shortestValidTransformStringLength = 9uz; // "rotate(0)"
      if (buffer.size() < shortestValidTransformStringLength)
      {
        return nullptr;
      }

      if (StartsWithLettersIgnoringASCIICase(buffer, u8"translate"))
      {
        // Also guarantees indexes up to buffer[11] are safe to access below.
        constexpr auto shortestValidTranslateStringLength = 12uz; // "translate(0)"
        if (buffer.size() < shortestValidTranslateStringLength)
        {
          return nullptr;
        }

        ValueId transformType;
        size_t expectedArgumentCount = 1uz;
        size_t argumentStart = 11uz;
        auto c9 = text::ToASCIILower(buffer[9]);
        if (c9 == u8'x' && buffer[10] == u8'(')
        {
          transformType = ValueId::TranslateX;
        }
        else if (c9 == u8'y' && buffer[10] == u8'(')
        {
          transformType = ValueId::TranslateY;
        }
        else if (c9 == u8'z' && buffer[10] == u8'(')
        {
          transformType = ValueId::TranslateZ;
        }
        else if (c9 == u8'(')
        {
          transformType = ValueId::Translate;
          expectedArgumentCount = 2uz;
          argumentStart = 10uz;
        }
        else if (c9 == u8'3' && text::ToASCIILower(buffer[10]) == u8'd' && buffer[11] == u8'(')
        {
          transformType = ValueId::Translate3d;
          expectedArgumentCount = 3uz;
          argumentStart = 12uz;
        }
        else
        {
          return nullptr;
        }

        ValueListBuilder arguments;
        buffer.remove_prefix(argumentStart);
        if (!ParseTransformTranslateArguments(buffer, expectedArgumentCount, transformType, arguments))
        {
          return nullptr;
        }

        return FunctionValue::Create(transformType, krys::move(arguments));
      }

      if (StartsWithLettersIgnoringASCIICase(buffer, u8"matrix3d("))
      {
        buffer.remove_prefix(9);
        ValueListBuilder arguments;
        if (!ParseTransformNumberArguments(buffer, 16uz, arguments))
        {
          return nullptr;
        }

        return FunctionValue::Create(ValueId::Matrix3d, krys::move(arguments));
      }

      if (StartsWithLettersIgnoringASCIICase(buffer, u8"scale3d("))
      {
        buffer.remove_prefix(8);
        ValueListBuilder arguments;
        if (!ParseTransformNumberArguments(buffer, 3uz, arguments))
        {
          return nullptr;
        }

        return FunctionValue::Create(ValueId::Scale3d, krys::move(arguments));
      }

      if (StartsWithLettersIgnoringASCIICase(buffer, u8"rotate"))
      {
        ValueId transformType;
        size_t argumentStart = 7uz;
        auto c6 = text::ToASCIILower(buffer[6]);
        if (c6 == u8'(')
        {
          transformType = ValueId::Rotate;
        }
        else if (c6 == u8'z' && buffer[7] == u8'(')
        {
          transformType = ValueId::RotateZ;
          argumentStart = 8uz;
        }
        else
        {
          return nullptr;
        }

        buffer.remove_prefix(argumentStart);
        auto angle = ParseTransformAngleArgument(buffer);
        if (!angle)
        {
          return nullptr;
        }

        return FunctionValue::Create(transformType, krys::move(angle));
      }

      return nullptr;
    }

    KRYS_NODISCARD RefPtr<Value> ParseSimpleTransform(CSSOMStringView buffer) noexcept
    {
      krys_debug_assert(!buffer.empty());

      ValueListBuilder builder;
      while (!buffer.empty())
      {
        SkipWhile(buffer, [](char8_t c) { return IsCSSSpace(c); });
        if (buffer.empty())
        {
          break;
        }

        auto transformValue = ParseSimpleTransformValue(buffer);
        if (!transformValue)
        {
          return nullptr;
        }

        builder.push_back(krys::move(transformValue));
      }

      if (builder.empty())
      {
        return nullptr;
      }

      return TransformListValue::Create(krys::move(builder));
    }

    KRYS_NODISCARD RefPtr<Value> ParseDisplay(CSSOMStringView string) noexcept
    {
      krys_debug_assert(!string.empty());

      auto valueId = ValueIdFromName(string);
      switch (valueId)
      {
        case ValueId::None:
        // <display-outside>
        case ValueId::Block:
        case ValueId::Inline:
        // <display-inside> (except for ValueId::Flow since it becomes "block")
        case ValueId::Flex:
        case ValueId::FlowRoot:
        case ValueId::Grid:
        case ValueId::Table:
        // <display-internal>
        case ValueId::TableCaption:
        case ValueId::TableCell:
        case ValueId::TableColumnGroup:
        case ValueId::TableColumn:
        case ValueId::TableHeaderGroup:
        case ValueId::TableFooterGroup:
        case ValueId::TableRow:
        case ValueId::TableRowGroup:
        // <display-legacy>
        case ValueId::InlineBlock:
        case ValueId::InlineFlex:
        case ValueId::InlineGrid:
        case ValueId::InlineTable:
        // Prefixed values
        case ValueId::WebkitInlineBox:
        case ValueId::WebkitBox:
        // No layout support for the full <display-listitem> syntax, so treat it as <display-legacy>
        case ValueId::ListItem:
        {
          return PrimitiveValue::Create(valueId);
        }
        default:
        {
          if (IsGlobalKeyword(valueId))
          {
            return PrimitiveValue::Create(valueId);
          }

          return nullptr;
        }
      }
    }

    KRYS_NODISCARD RefPtr<Value> ParseOpacity(CSSOMStringView string) noexcept
    {
      double number;
      if (!ParseSimpleNumberOrPercentageDividedBy100(string, number))
      {
        return nullptr;
      }

      return PrimitiveValue::Create(number, UnitType::Number);
    }

    KRYS_NODISCARD RefPtr<Value> ParseColorWithAuto(CSSOMStringView string,
                                                    const ParserContext &context) noexcept
    {
      krys_debug_assert(!string.empty());

      if (ValueIdFromName(string) == ValueId::Auto)
      {
        return PrimitiveValue::Create(ValueId::Auto);
      }

      return ParseColor(string, context);
    }
  }

  RefPtr<Value> ParserFastPaths::MaybeParseValue(PropertyId property, CSSOMStringView string,
                                                 PropertyParserState &state) noexcept
  {
    // Some at-rules like @keyframes, @position-try restrict which properties
    // are allowed inside the rule. Fallback to slow path for at-rules since
    // the restriction logic is in the slow-path parser (PropertyParser).
    if (state.CurrentRule != RuleType::Style)
    {
      return nullptr;
    }

    switch (property)
    {
      case PropertyId::Display:
      {
        return ParseDisplay(string);
      }
      case PropertyId::Opacity:
      {
        return ParseOpacity(string);
      }
      case PropertyId::Transform:
      {
        return ParseSimpleTransform(string);
      }
      case PropertyId::CaretColor:
      case PropertyId::AccentColor:
      {
        if (IsExposed(property, &state.Context.PropertySettings))
        {
          return ParseColorWithAuto(string, state.Context);
        }

        break;
      }
      default:
      {
        break;
      }
    }

    if (Property::IsColorProperty(property))
    {
      auto context = state.Context;
      if (!Property::AcceptsQuirkyColor(property))
      {
        context.Mode = ParserMode::HTMLStandard;
      }

      return ParseColor(string, context);
    }

    if (auto valueRange = LengthValueRangeForPropertiesSupportingSimpleLengths(property))
    {
      if (auto result = ParseSimpleLengthValue(string, state.Context.Mode, *valueRange))
      {
        return result;
      }
    }

    return ParseKeywordValue(property, string, state);
  }
}
