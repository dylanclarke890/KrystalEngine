#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/StylePropertiesTestUtils.hpp"
#include "Krystal.Booey/CSS/Properties/ImmutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"

namespace krys::boo::css::tests
{
  /// @brief Parses `css`, a `;`-separated list of descriptors valid in a `ruleType` at-rule, into a mutable
  /// declaration block.
  /// @note Values must not contain `;`.
  KRYS_NODISCARD inline Ref<MutableStyleProperties> ParseDescriptors(utf8_stringview css, RuleType ruleType)
  {
    auto context = DeclarationTestContext();
    context.PropertySettings.crossDocumentViewTransitionsEnabled = true;
    auto properties = MutableStyleProperties::Create(ParserMode::HTMLStandard);
    while (!css.empty())
    {
      auto end = css.find(u8';');
      auto declaration = TrimWhitespace(css.substr(0, end));
      if (!declaration.empty())
      {
        auto colon = declaration.find(u8':');
        REQUIRE(colon != utf8_stringview::npos);
        auto name = TrimWhitespace(declaration.substr(0, colon));
        auto value = TrimWhitespace(declaration.substr(colon + 1));

        Tokens tokens {value};
        ParsedPropertyList parsed;
        auto property = FindProperty(name);
        INFO(ToNarrow(name));
        REQUIRE(property != PropertyId::Invalid);
        REQUIRE(PropertyParser::ParseValue(property, IsImportant(false), tokens.Tokenizer.Tokens(), context,
                                           parsed, ruleType));
        properties->AddParsedProperties(parsed);
      }

      css = end == utf8_stringview::npos ? utf8_stringview {} : css.substr(end + 1);
    }

    return properties;
  }

  /// @brief Parses `css` (see `ParseDeclarations`) into an immutable declaration block, as the parser
  /// creates them.
  KRYS_NODISCARD inline Ref<StyleProperties> ImmutableDeclarations(utf8_stringview css)
  {
    return ParseDeclarations(css)->ImmutableCopyIfNeeded();
  }

  /// @brief Parses `text` as a selector list, which must be valid.
  KRYS_NODISCARD inline SelectorList ParseSelectors(CSSOMStringView text)
  {
    auto list = SelectorParser::ParseSelectorList(text, SelectorParserContext {});
    REQUIRE(list);
    return krys::move(*list);
  }

  /// @brief A style rule with the selectors `selectors` and the declarations `declarations`.
  KRYS_NODISCARD inline Ref<StyleRule> CreateStyleRule(CSSOMStringView selectors,
                                                       utf8_stringview declarations)
  {
    return StyleRule::Create(ImmutableDeclarations(declarations), false, ParseSelectors(selectors));
  }

  /// @brief The selector text of `list`, as a narrow string.
  KRYS_NODISCARD inline std::string SelectorsText(const SelectorList &list)
  {
    return ToNarrow(list.SelectorsText());
  }
}
