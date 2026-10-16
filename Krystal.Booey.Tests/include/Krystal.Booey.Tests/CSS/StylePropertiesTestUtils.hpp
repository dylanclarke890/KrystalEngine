#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/VariableParser.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParser.hpp"
#include "Krystal.Booey/CSS/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include <string>

namespace krys::boo::css::tests
{
  /// @brief Trims ASCII whitespace from both ends of `text`.
  KRYS_NODISCARD inline utf8_stringview TrimWhitespace(utf8_stringview text) noexcept
  {
    auto isSpace = [](char8 c)
    {
      return c == u8' ' || c == u8'\t' || c == u8'\n';
    };
    while (!text.empty() && isSpace(text.front()))
    {
      text.remove_prefix(1);
    }

    while (!text.empty() && isSpace(text.back()))
    {
      text.remove_suffix(1);
    }

    return text;
  }

  /// @brief Parses one declaration (`name: value`, optionally `!important`) and adds it to `properties`.
  inline void AddDeclaration(MutableStyleProperties &properties, utf8_stringview declaration,
                             const ParserContext &context)
  {
    auto colon = declaration.find(u8':');
    REQUIRE(colon != utf8_stringview::npos);
    auto name = TrimWhitespace(declaration.substr(0, colon));
    auto value = TrimWhitespace(declaration.substr(colon + 1));

    constexpr utf8_stringview importantSuffix = u8"!important";
    IsImportant important(value.ends_with(importantSuffix));
    if (important)
    {
      value = TrimWhitespace(value.substr(0, value.size() - importantSuffix.size()));
    }

    Tokens tokens {value};
    ParsedPropertyList parsed;
    if (name.starts_with(u8"--"))
    {
      auto customValue = VariableParser::ParseDeclarationValue(CSSOMStringAtom(CSSOMString(name)),
                                                               tokens.Tokenizer.Tokens(), context);
      REQUIRE(customValue);
      parsed.push_back(Property(PropertyId::Custom, Ref<Value>(krys::move(customValue)), important));
    }
    else
    {
      auto property = FindProperty(name);
      INFO(ToNarrow(name));
      REQUIRE(property != PropertyId::Invalid);
      REQUIRE(IsExposed(property, context.PropertySettings));
      REQUIRE(PropertyParser::ParseValue(property, important, tokens.Tokenizer.Tokens(), context, parsed,
                                         RuleType::Style));
    }

    properties.AddParsedProperties(parsed);
  }

  /// @brief A parser context with the settings-gated properties the declaration block tests use enabled.
  KRYS_NODISCARD inline ParserContext DeclarationTestContext(ParserMode mode = ParserMode::HTMLStandard)
  {
    ParserContext context {mode};
    return context;
  }

  /// @brief Parses `css`, a `;`-separated declaration list, into a mutable declaration block.
  /// @note Values must not contain `;`. A later declaration replaces an earlier one, important or not.
  KRYS_NODISCARD inline Ref<MutableStyleProperties>
    ParseDeclarations(utf8_stringview css, ParserMode mode = ParserMode::HTMLStandard)
  {
    auto context = DeclarationTestContext(mode);
    auto properties = MutableStyleProperties::Create(mode);
    while (!css.empty())
    {
      auto end = css.find(u8';');
      auto declaration = TrimWhitespace(css.substr(0, end));
      if (!declaration.empty())
      {
        AddDeclaration(*properties, declaration, context);
      }

      css = end == utf8_stringview::npos ? utf8_stringview {} : css.substr(end + 1);
    }

    return properties;
  }

  /// @brief The serialisation of a declaration block, as a narrow string.
  KRYS_NODISCARD inline std::string AsText(const StyleProperties &properties)
  {
    return ToNarrow(properties.AsText(DefaultSerialisationContext()));
  }

  /// @brief The value of `property` (a longhand or shorthand) in `properties`, as a narrow string.
  KRYS_NODISCARD inline std::string ValueOf(const StyleProperties &properties, PropertyId property)
  {
    return ToNarrow(properties.GetPropertyValue(property));
  }

  template <typename T>
  KRYS_NODISCARD std::string AsText(const Ref<T> &properties)
  {
    return AsText(static_cast<const StyleProperties &>(*properties));
  }

  template <typename T>
  KRYS_NODISCARD std::string ValueOf(const Ref<T> &properties, PropertyId property)
  {
    return ValueOf(static_cast<const StyleProperties &>(*properties), property);
  }

  /// @brief Parses `css` and serialises `shorthand` from it.
  KRYS_NODISCARD inline std::string SerialiseShorthand(utf8_stringview css, PropertyId shorthand)
  {
    return ValueOf(ParseDeclarations(css), shorthand);
  }
}
