/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Web/Css/TypedOM/Color/CSSColorValue.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"
#include "Krystal.Web/Css/Parser/InputStream.hpp"
#include "Krystal.Web/Css/Parser/ParserContext.hpp"
#include "Krystal.Web/Css/Parser/Tokenizer.hpp"
#include "Krystal.Web/Css/Properties/Consumers/Color.hpp"
#include "Krystal.Web/Css/Properties/Consumers/ColorInlines.hpp"
#include "Krystal.Web/Css/Properties/PropertyParserState.hpp"
#include "Krystal.Web/Css/TypedOM/Color/CSSColor.hpp"
#include "Krystal.Web/Css/TypedOM/Color/CSSHSL.hpp"
#include "Krystal.Web/Css/TypedOM/Color/CSSHWB.hpp"
#include "Krystal.Web/Css/TypedOM/Color/CSSLab.hpp"
#include "Krystal.Web/Css/TypedOM/Color/CSSLCH.hpp"
#include "Krystal.Web/Css/TypedOM/Color/CSSOKLab.hpp"
#include "Krystal.Web/Css/TypedOM/Color/CSSOKLCH.hpp"
#include "Krystal.Web/Css/TypedOM/Color/CSSRGB.hpp"
#include "Krystal.Web/Css/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Web/Css/Values/Color/KeywordColor.hpp"
#include "Krystal.Web/Css/Values/SerialisationContext.hpp"
#include "Krystal.Web/Css/Values/Value.hpp"
#include "Krystal.Web/Infra/StringAlgorithms.hpp"

namespace krys::web
{
  namespace
  {
    /// @brief Rectifies a colour component: a number is @p numberToValue's value, a numeric value must match
    /// @p matches, and a keyword must be `none`.
    template <typename TNumberToValue, typename TMatches>
    KRYS_NODISCARD ExceptionOr<RectifiedCSSColorComponent> RectifyCSSColorComponent(
      Variant<double, RefPtr<CSSNumericValue>, CSSOMString, RefPtr<CSSKeywordValue>> component,
      const TNumberToValue &numberToValue, const TMatches &matches) noexcept
    {
      auto keyword = [](Ref<CSSKeywordValue> value) -> ExceptionOr<RectifiedCSSColorComponent>
      {
        if (!infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value->Value(), u8"none"))
        {
          return Exception {ExceptionCode::SyntaxError};
        }
        return RectifiedCSSColorComponent {RefPtr<CSSKeywordValue>(krys::move(value))};
      };
      return krys::SwitchOn(
        krys::move(component), [&](double value) -> ExceptionOr<RectifiedCSSColorComponent>
        { return RectifiedCSSColorComponent {RefPtr<CSSNumericValue>(numberToValue(value))}; },
        [&](RefPtr<CSSNumericValue> &&value) -> ExceptionOr<RectifiedCSSColorComponent>
        {
          if (!value || !matches(value->Type()))
          {
            return Exception {ExceptionCode::SyntaxError};
          }
          return RectifiedCSSColorComponent {krys::move(value)};
        },
        [&](CSSOMString &&value) { return keyword(CSSKeywordValue::RectifyKeywordish(krys::move(value))); },
        [&](RefPtr<CSSKeywordValue> &&value) -> ExceptionOr<RectifiedCSSColorComponent>
        {
          if (!value)
          {
            return Exception {ExceptionCode::SyntaxError};
          }
          return keyword(Ref<CSSKeywordValue>(krys::move(value)));
        });
    }

    /// @brief What a colour function's component is, for reifying it: an RGB channel (a number or a
    /// percentage), a percentage, a number or an angle. A number given for a percentage, or a percentage for
    /// a number, is converted with the component's reference range (a percentage of 100% being @p Reference).
    struct ColorComponentSlot
    {
      enum class Kind : uint8
      {
        RGB,
        Percent,
        Number,
        Angle,
      };

      Kind SlotKind {Kind::Percent};
      double Reference {1.0};
    };

    using SlotKind = ColorComponentSlot::Kind;

    /// @brief The component values of a colour function's arguments, and those after its slash (its alpha).
    struct ColorFunctionArguments
    {
      List<css::TokenRange> Components;
      Maybe<css::TokenRange> Alpha;
    };

    KRYS_NODISCARD ColorFunctionArguments SplitColorFunctionArguments(css::TokenRange arguments) noexcept
    {
      ColorFunctionArguments split;
      bool afterSlash = false;
      while (true)
      {
        arguments.DiscardWhitespace();
        if (arguments.IsAtEnd())
        {
          break;
        }
        const auto &token = arguments.Peek();
        if (token.Type() == css::TokenType::Comma)
        {
          arguments.Discard();
          continue;
        }
        if (token.Type() == css::TokenType::Delim && token.Delimiter() == U'/')
        {
          arguments.Discard();
          afterSlash = true;
          continue;
        }
        auto start = arguments;
        arguments.DiscardComponentValue();
        auto component = start.RangeUntil(arguments);
        if (afterSlash)
        {
          split.Alpha = component;
        }
        else
        {
          split.Components.push_back(component);
        }
      }
      // The legacy comma-separated syntax gives alpha as a fourth argument.
      if (!split.Alpha && split.Components.size() == 4u)
      {
        split.Alpha = split.Components.back();
        split.Components.pop_back();
      }
      return split;
    }

    /// @brief @p component reified for @p slot: `none`, or its numeric value, a number or percentage given
    /// for the other being converted; null if it can't be (a calculation of the other type).
    KRYS_NODISCARD Maybe<CSSColorPercent> ReifyColorComponent(Document &document, css::TokenRange component,
                                                              ColorComponentSlot slot) noexcept
    {
      const auto &token = component.Peek();
      if (token.Type() == css::TokenType::Ident)
      {
        return CSSColorPercent {
          RefPtr<CSSKeywordValue>(CSSKeywordValue::RectifyKeywordish(CSSOMString(token.IdentCodePoints())))};
      }
      auto parsed =
        CSSNumericValue::Parse(document, component.Serialise(css::TokenSerialisationMode::Normal));
      if (parsed.HasException())
      {
        return null;
      }
      RefPtr<CSSNumericValue> value = parsed.ReleaseValue();
      if (auto *unit = DynamicDowncast<CSSUnitValue>(value.get()))
      {
        const auto unitType = unit->UnitEnum();
        if (slot.SlotKind == SlotKind::Percent && unitType == css::UnitType::Number)
        {
          value = CSSUnitValue::Create(unit->Value() * 100 / slot.Reference, css::UnitType::Percentage);
        }
        else if (slot.SlotKind == SlotKind::Number && unitType == css::UnitType::Percentage)
        {
          value = CSSUnitValue::Create(unit->Value() / 100 * slot.Reference, css::UnitType::Number);
        }
        else if (slot.SlotKind == SlotKind::Angle && unitType == css::UnitType::Number)
        {
          value = CSSUnitValue::Create(unit->Value(), css::UnitType::deg);
        }
      }
      return CSSColorPercent {krys::move(value)};
    }

    /// @brief The components of a colour function reified for @p slots (and its alpha as a percentage, 100%
    /// when it has none); null if one can't be, or their number isn't the slots'.
    KRYS_NODISCARD Maybe<List<CSSColorPercent>>
      ReifyColorComponents(Document &document, const ColorFunctionArguments &arguments,
                           std::initializer_list<ColorComponentSlot> slots) noexcept
    {
      if (arguments.Components.size() != slots.size())
      {
        return null;
      }
      List<CSSColorPercent> components;
      size_t index = 0;
      for (const auto &slot : slots)
      {
        auto component = ReifyColorComponent(document, arguments.Components[index++], slot);
        if (!component)
        {
          return null;
        }
        components.push_back(krys::move(*component));
      }
      if (arguments.Alpha)
      {
        auto alpha = ReifyColorComponent(document, *arguments.Alpha, {SlotKind::Percent, 1.0});
        if (!alpha)
        {
          return null;
        }
        components.push_back(krys::move(*alpha));
      }
      else
      {
        components.emplace_back(1.0);
      }
      return components;
    }

    /// @brief The colour value a successful construction made, or null.
    template <typename T>
    KRYS_NODISCARD RefPtr<CSSStyleValue> Made(ExceptionOr<Ref<T>> made) noexcept
    {
      if (made.HasException())
      {
        return nullptr;
      }
      return RefPtr<CSSStyleValue>(made.ReleaseValue());
    }

    /// @brief A colour function reified as its colour value, or null if its components don't fit one.
    KRYS_NODISCARD RefPtr<CSSStyleValue> ReifyColorFunction(Document &document, css::ValueId function,
                                                            css::TokenRange arguments) noexcept
    {
      auto split = SplitColorFunctionArguments(arguments);
      // A relative colour (`rgb(from ...)`) has no colour value.
      if (!split.Components.empty() && split.Components.front().Peek().Type() == css::TokenType::Ident
          && infra::StringAlgorithms::ASCIICaseInsensitiveMatch(
            split.Components.front().Peek().IdentCodePoints(), u8"from"))
      {
        return nullptr;
      }

      // The reference ranges are CSS Color's: https://drafts.csswg.org/css-color-4/#color-syntax
      constexpr ColorComponentSlot rgb {SlotKind::RGB};
      constexpr ColorComponentSlot angle {SlotKind::Angle};
      auto percent = [](double reference)
      {
        return ColorComponentSlot {SlotKind::Percent, reference};
      };
      auto number = [](double reference)
      {
        return ColorComponentSlot {SlotKind::Number, reference};
      };
      auto take = [](List<CSSColorPercent> &components, size_t index)
      {
        return krys::move(components[index]);
      };

      switch (function)
      {
        // 1. If value is a hex color, rgb() function, or rgba() function: return a new CSSRGB.
        case css::ValueId::Rgb:
        case css::ValueId::Rgba:
          if (auto c = ReifyColorComponents(document, split, {rgb, rgb, rgb}))
          {
            return Made(CSSRGB::Create(take(*c, 0), take(*c, 1), take(*c, 2), take(*c, 3)));
          }
          return nullptr;
        // 2. If value is an hsl() or hsla() function: return a new CSSHSL.
        case css::ValueId::Hsl:
        case css::ValueId::Hsla:
          if (auto c = ReifyColorComponents(document, split, {angle, percent(100), percent(100)}))
          {
            return Made(CSSHSL::Create(take(*c, 0), take(*c, 1), take(*c, 2), take(*c, 3)));
          }
          return nullptr;
        // 3. If value is an hwb() function: return a new CSSHWB.
        case css::ValueId::Hwb:
          if (auto c = ReifyColorComponents(document, split, {angle, percent(100), percent(100)}))
          {
            // Its components are numeric values, without `none`.
            auto numeric = [](CSSColorPercent component) -> RefPtr<CSSNumericValue>
            {
              auto *value = std::get_if<RefPtr<CSSNumericValue>>(&component);
              return value ? krys::move(*value) : nullptr;
            };
            auto hue = numeric(take(*c, 0));
            auto whiteness = numeric(take(*c, 1));
            auto blackness = numeric(take(*c, 2));
            auto alphaComponent = take(*c, 3);
            auto *alphaNumber = std::get_if<double>(&alphaComponent);
            auto alpha =
              alphaNumber ? CSSNumberish {*alphaNumber} : CSSNumberish {numeric(krys::move(alphaComponent))};
            if (!hue || !whiteness || !blackness
                || (std::holds_alternative<RefPtr<CSSNumericValue>>(alpha)
                    && !std::get<RefPtr<CSSNumericValue>>(alpha)))
            {
              return nullptr;
            }
            return Made(CSSHWB::Create(Ref<CSSNumericValue>(krys::move(hue)),
                                       CSSNumberish {krys::move(whiteness)},
                                       CSSNumberish {krys::move(blackness)}, krys::move(alpha)));
          }
          return nullptr;
        // 4. If value is an lch() function: return a new CSSLCH.
        case css::ValueId::Lch:
          if (auto c = ReifyColorComponents(document, split, {percent(100), percent(150), angle}))
          {
            return Made(CSSLCH::Create(take(*c, 0), take(*c, 1), take(*c, 2), take(*c, 3)));
          }
          return nullptr;
        // 5. If value is an lab() function: return a new CSSLab.
        case css::ValueId::Lab:
          if (auto c = ReifyColorComponents(document, split, {percent(100), number(125), number(125)}))
          {
            return Made(CSSLab::Create(take(*c, 0), take(*c, 1), take(*c, 2), take(*c, 3)));
          }
          return nullptr;
        // 6. If value is an oklch() function: return a new CSSOKLCH.
        case css::ValueId::Oklch:
          if (auto c = ReifyColorComponents(document, split, {percent(1), percent(0.4), angle}))
          {
            return Made(CSSOKLCH::Create(take(*c, 0), take(*c, 1), take(*c, 2), take(*c, 3)));
          }
          return nullptr;
        // 7. If value is an oklab() function: return a new CSSOKLab.
        case css::ValueId::Oklab:
          if (auto c = ReifyColorComponents(document, split, {percent(1), number(0.4), number(0.4)}))
          {
            return Made(CSSOKLab::Create(take(*c, 0), take(*c, 1), take(*c, 2), take(*c, 3)));
          }
          return nullptr;
        // 8. If value is a color() function: return a new CSSColor, its colorSpace the colour space's
        //    identifier reified, its channels the components other than alpha, and its alpha.
        case css::ValueId::Color:
        {
          if (split.Components.empty() || split.Components.front().Peek().Type() != css::TokenType::Ident)
          {
            return nullptr;
          }
          auto colorSpace = CSSOMString(split.Components.front().Peek().IdentCodePoints());
          List<CSSColorPercent> channels;
          for (size_t index = 1; index < split.Components.size(); ++index)
          {
            auto channel = ReifyColorComponent(document, split.Components[index], percent(1));
            if (!channel)
            {
              return nullptr;
            }
            channels.push_back(krys::move(*channel));
          }
          CSSNumberish alpha {1.0};
          if (split.Alpha)
          {
            auto reified = ReifyColorComponent(document, *split.Alpha, percent(1));
            auto *value = reified ? std::get_if<RefPtr<CSSNumericValue>>(&*reified) : nullptr;
            if (!value)
            {
              return nullptr;
            }
            alpha = CSSNumberish {krys::move(*value)};
          }
          return Made(CSSColor::Create(CSSKeywordish {krys::move(colorSpace)}, krys::move(channels),
                                       krys::move(alpha)));
        }
        default: return nullptr;
      }
    }

    /// @brief A colour in sRGB as a CSSRGB, its channels numbers from 0 to 255.
    KRYS_NODISCARD RefPtr<CSSStyleValue> ReifySRGBColor(const web::Color &color) noexcept
    {
      auto [red, green, blue, alpha] = color.ToColorTypeLossy<SRGBA<uint8>>().Resolved();
      auto channel = [](uint8 value)
      {
        return CSSColorRGBComp {RefPtr<CSSNumericValue>(CSSUnitValue::Create(value, css::UnitType::Number))};
      };
      return Made(
        CSSRGB::Create(channel(red), channel(green), channel(blue), CSSColorPercent {alpha / 255.0}));
    }
  }

  ExceptionOr<Ref<CSSStyleValue>> CSSColorValue::Parse(Document &document, CSSOMStringView cssText) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csscolorvalue-parse
    // 1. Let result be the result of parsing cssText as a <color>. If result is a syntax error, throw a
    //    SyntaxError and abort this algorithm.
    css::InputStream inputStream {CSSOMString(cssText)};
    css::Tokenizer tokenizer(inputStream);
    std::ignore = tokenizer.PumpTokenizer();
    auto tokens = tokenizer.Tokens();
    tokens.DiscardWhitespace();
    auto colorTokens = tokens;
    auto parserContext = css::ParserContext {document};
    auto parserState = css::PropertyParserState {.Context = parserContext};
    auto result = css::parse_property::ConsumeColor(tokens, parserState);
    tokens.DiscardWhitespace();
    if (!result || !tokens.IsAtEnd())
    {
      return Exception {ExceptionCode::SyntaxError, u8"Failed to parse CSS text as a colour"};
    }

    // 2. Reify a color value from result, and return the result.
    // https://drafts.css-houdini.org/css-typed-om/#reify-a-color-value
    const auto &first = colorTokens.Peek();
    if (first.Type() == css::TokenType::Hash)
    {
      // 1. (A hex color.)
      if (auto reified = ReifySRGBColor(css::parse_property::ParseColorRaw(cssText, parserContext)))
      {
        return Ref<CSSStyleValue>(krys::move(reified));
      }
    }
    else if (first.Type() == css::TokenType::Ident)
    {
      // 9. If value is a named color or the transparent keyword, return a new CSSRGB with its color.
      if (css::IsAbsoluteColorKeyword(first.ValueId()))
      {
        if (auto reified = ReifySRGBColor(css::parse_property::ParseColorRaw(cssText, parserContext)))
        {
          return Ref<CSSStyleValue>(krys::move(reified));
        }
      }
      // 10. Otherwise (any other color keyword), reify an identifier from value.
      return Ref<CSSStyleValue>(CSSKeywordValue::RectifyKeywordish(CSSOMString(first.IdentCodePoints())));
    }
    else if (first.Type() == css::TokenType::Function)
    {
      const auto function = first.FunctionId();
      auto arguments = colorTokens.ConsumeBlock();
      if (auto reified = ReifyColorFunction(document, function, arguments))
      {
        return Ref<CSSStyleValue>(krys::move(reified));
      }
    }
    // A colour the colour values can't hold (`color-mix()`, a relative colour, or a component of another
    // type) is a plain style value.
    return CSSStyleValue::Create(krys::move(result));
  }

  CSSOMString CSSColorValue::FunctionText(utf8_stringview name, Span<const CSSColorPercent> components,
                                          const CSSColorPercent &alpha) noexcept
  {
    StringBuilder builder;
    auto append = [&](const CSSColorPercent &component)
    {
      krys::SwitchOn(
        component,
        [&](double value) { builder.Append(CSSUnitValue::Create(value, css::UnitType::Number)->ToString()); },
        [&](const RefPtr<CSSNumericValue> &value) { builder.Append(value->ToString()); },
        [&](const CSSOMString &value) { builder.Append(value); },
        [&](const RefPtr<CSSKeywordValue> &value) { builder.Append(value->Value()); });
    };
    builder.Append(name, u8"(");
    bool first = true;
    for (const auto &component : components)
    {
      if (!first)
      {
        builder.Append(u8" ");
      }
      first = false;
      append(component);
    }
    builder.Append(u8" / ");
    append(alpha);
    builder.Append(u8")");
    return krys::move(builder).ToString();
  }

  ExceptionOr<RectifiedCSSColorComponent>
    CSSColorValue::RectifyCSSColorPercent(CSSColorPercent percent) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#rectify-a-csscolorpercent
    return RectifyCSSColorComponent(
      krys::move(percent),
      [](double value) { return CSSUnitValue::Create(value * 100, css::UnitType::Percentage); },
      [](const CSSNumericType &type) { return type.Matches<CSSNumericBaseType::Percent>(); });
  }

  ExceptionOr<RectifiedCSSColorComponent> CSSColorValue::RectifyCSSColorAngle(CSSColorAngle angle) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#rectify-a-csscolorangle
    return RectifyCSSColorComponent(
      krys::move(angle), [](double value) { return CSSUnitValue::Create(value, css::UnitType::deg); },
      [](const CSSNumericType &type) { return type.Matches<CSSNumericBaseType::Angle>(); });
  }

  ExceptionOr<RectifiedCSSColorComponent> CSSColorValue::RectifyCSSColorNumber(CSSColorNumber number) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#rectify-a-csscolornumber
    return RectifyCSSColorComponent(
      krys::move(number), [](double value) { return CSSUnitValue::Create(value, css::UnitType::Number); },
      [](const CSSNumericType &type) { return type.MatchesNumber(); });
  }

  CSSColorPercent CSSColorValue::ToCSSColorComponent(const RectifiedCSSColorComponent &component) noexcept
  {
    return krys::SwitchOn(
      component, [](const RefPtr<CSSNumericValue> &value) { return CSSColorPercent {value}; },
      [](const RefPtr<CSSKeywordValue> &value) { return CSSColorPercent {value}; });
  }

  CSSColorPercent CSSColorValue::ToCSSColorPercent(const CSSNumberish &numberish) noexcept
  {
    return krys::SwitchOn(
      numberish, [](double value) { return CSSColorPercent {value}; },
      [](const RefPtr<CSSNumericValue> &value) { return CSSColorPercent {value}; });
  }

  RefPtr<css::Value> CSSColorValue::ToCSSValue() const noexcept
  {
    // The colour function its components make, parsed as a `<color>`; null if they don't make one.
    css::InputStream inputStream {ColorFunctionText()};
    css::Tokenizer tokenizer(inputStream);
    std::ignore = tokenizer.PumpTokenizer();
    auto tokens = tokenizer.Tokens();
    auto parserContext = css::ParserContext {css::ParserMode::HTMLStandard};
    auto parserState = css::PropertyParserState {.Context = parserContext};
    auto value = css::parse_property::ConsumeColor(tokens, parserState);
    tokens.DiscardWhitespace();
    return tokens.IsAtEnd() ? value : nullptr;
  }

  void CSSColorValue::Serialise(StringBuilder &builder, EnumSet<SerialisationArguments>) const noexcept
  {
    if (auto value = ToCSSValue())
    {
      builder.Append(value->CssText(css::DefaultSerialisationContext()));
    }
  }
}
