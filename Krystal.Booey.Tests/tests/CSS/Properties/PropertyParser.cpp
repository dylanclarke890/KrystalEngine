#include "Krystal.Booey/CSS/Properties/PropertyParser.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"
#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Parser/CustomPropertySyntax.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/CSS/Values/GlobalKeyword.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include <string>

namespace krys::boo::css::tests
{
  namespace
  {
    /// @brief Lists `properties` as "name: value" pairs joined by "; ", with " !important" where set.
    KRYS_NODISCARD std::string PropertiesToText(const ParsedPropertyList &properties)
    {
      std::string text;
      for (const auto &property : properties)
      {
        if (!text.empty())
        {
          text += "; ";
        }

        text += ToNarrow(ToString(property.Id())) + ": " + CssText(property.Value());
        if (property.Important())
        {
          text += " !important";
        }
      }

      return text;
    }

    /// @brief Parses `css` as the value of `property` in a rule of type `ruleType`.
    /// @return "<fail>" if parsing failed, otherwise the parsed properties (see `PropertiesToText`).
    KRYS_NODISCARD std::string ParseToText(utf8_stringview css, PropertyId property,
                                           const ParserContext &context, RuleType ruleType = RuleType::Style,
                                           IsImportant important = IsImportant(false))
    {
      Tokens tokens {css};
      ParsedPropertyList properties;
      if (!PropertyParser::ParseValue(property, important, tokens.Tokenizer.Tokens(), context, properties,
                                      ruleType))
      {
        CHECK(properties.empty());
        return "<fail>";
      }

      return PropertiesToText(properties);
    }

    KRYS_NODISCARD std::string ParseToText(utf8_stringview css, PropertyId property,
                                           RuleType ruleType = RuleType::Style,
                                           IsImportant important = IsImportant(false))
    {
      ParserContext context {ParserMode::HTMLStandard};
      return ParseToText(css, property, context, ruleType, important);
    }

    KRYS_NODISCARD std::string LonghandToText(utf8_stringview css, PropertyId property)
    {
      ParserContext context {ParserMode::HTMLStandard};
      auto value = PropertyParser::ParseStylePropertyLonghand(property, CSSOMString(css), context);
      return value ? CssText(*value) : "<null>";
    }
  }

  TEST_CASE_METHOD(ValueFixture, "PropertyParser::ParseValue", "[CSS][Properties][PropertyParser]")
  {
    SECTION("Longhands")
    {
      CHECK(ParseToText(u8"red", PropertyId::Color) == "color: red");
      CHECK(ParseToText(u8"  10px  ", PropertyId::Width) == "width: 10px");
      CHECK(ParseToText(u8"red", PropertyId::Color, RuleType::Style, IsImportant(true))
            == "color: red !important");
      CHECK(ParseToText(u8"10px", PropertyId::Color) == "<fail>");
      CHECK(ParseToText(u8"red blue", PropertyId::Color) == "<fail>");
    }

    SECTION("Shorthands")
    {
      CHECK(ParseToText(u8"1px 2px", PropertyId::Margin)
            == "margin-top: 1px; margin-right: 2px; margin-bottom: 1px; margin-left: 2px");
      CHECK(ParseToText(u8"1px 2px 3px 4px 5px", PropertyId::Margin) == "<fail>");
    }

    SECTION("CSS-wide keywords")
    {
      CHECK(ParseToText(u8"inherit", PropertyId::Color) == "color: inherit");
      CHECK(ParseToText(u8"revert-layer", PropertyId::Width) == "width: revert-layer");
      CHECK(ParseToText(u8"initial", PropertyId::Margin)
            == "margin-top: initial; margin-right: initial; margin-bottom: initial; margin-left: initial");
      CHECK(ParseToText(u8"inherit red", PropertyId::Color) == "<fail>");
    }

    SECTION("Variable references")
    {
      CHECK(ParseToText(u8"var(--x)", PropertyId::Color) == "color: var(--x)");
      CHECK(ParseToText(u8"1px var(--x)", PropertyId::Margin).starts_with("margin-top: "));
      CHECK(ParseToText(u8"var(x)", PropertyId::Color) == "<fail>");
    }

    SECTION("A failed parse leaves earlier properties alone")
    {
      Tokens tokens {u8"1px 2px nonsense"};
      ParsedPropertyList properties;
      properties.push_back(Property(PropertyId::Color, PrimitiveValue::Create(ValueId::Red)));

      ParserContext context {ParserMode::HTMLStandard};
      CHECK_FALSE(PropertyParser::ParseValue(PropertyId::Margin, IsImportant(false),
                                             tokens.Tokenizer.Tokens(), context, properties,
                                             RuleType::Style));
      REQUIRE(properties.size() == 1uz);
      CHECK(properties[0].Id() == PropertyId::Color);
    }

    SECTION("-internal-auto-base()")
    {
      ParserContext context {ParserMode::HTMLStandard};
      CHECK(ParseToText(u8"-internal-auto-base(red, blue)", PropertyId::Color, context) == "<fail>");

      context.cssInternalAutoBaseParsingEnabled = true;
      CHECK(ParseToText(u8"-internal-auto-base(red, blue)", PropertyId::Color, context)
            == "color: -internal-auto-base(red, blue)");
      CHECK(ParseToText(u8"-internal-auto-base(red, 10px)", PropertyId::Color, context) == "<fail>");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "PropertyParser::ParseValue in at-rules",
                   "[CSS][Properties][PropertyParser]")
  {
    SECTION("@keyframes")
    {
      CHECK(ParseToText(u8"red", PropertyId::Color, RuleType::Keyframe) == "color: red");
      CHECK(ParseToText(u8"ease-in", PropertyId::AnimationTimingFunction, RuleType::Keyframe)
            == "animation-timing-function: ease-in");
      CHECK(ParseToText(u8"foo", PropertyId::AnimationName, RuleType::Keyframe) == "<fail>");
      CHECK(ParseToText(u8"1s", PropertyId::AnimationDuration, RuleType::Keyframe) == "<fail>");
    }

    SECTION("@position-try")
    {
      ParserContext context {ParserMode::HTMLStandard};
      context.PropertySettings.cssAnchorPositioningEnabled = true;

      CHECK(ParseToText(u8"1px", PropertyId::Top, context, RuleType::PositionTry) == "top: 1px");
      CHECK(ParseToText(u8"1px", PropertyId::MarginLeft, context, RuleType::PositionTry)
            == "margin-left: 1px");
      CHECK(ParseToText(u8"red", PropertyId::Color, context, RuleType::PositionTry) == "<fail>");
      CHECK(ParseToText(u8"1px", PropertyId::Top, context, RuleType::PositionTry, IsImportant(true))
            == "<fail>");
    }

    SECTION("@page")
    {
      CHECK(ParseToText(u8"a4", PropertyId::Size, RuleType::Page) == "size: a4");
      CHECK(ParseToText(u8"1px", PropertyId::MarginTop, RuleType::Page) == "margin-top: 1px");
      CHECK(ParseToText(u8"auto", PropertyId::Page, RuleType::Page) == "<fail>");
    }

    SECTION("Descriptors")
    {
      CHECK(ParseToText(u8"swap", PropertyId::FontDisplay, RuleType::FontFace) == "font-display: swap");
      CHECK(ParseToText(u8"cyclic", PropertyId::System, RuleType::CounterStyle) == "system: cyclic");
      CHECK(ParseToText(u8"true", PropertyId::Inherits, RuleType::Property) == "inherits: true");
      CHECK(ParseToText(u8"red", PropertyId::Color, RuleType::FontFace) == "<fail>");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "PropertyParser::ParseStylePropertyLonghand",
                   "[CSS][Properties][PropertyParser]")
  {
    // Through the fast path.
    CHECK(LonghandToText(u8"block", PropertyId::Display) == "block");
    CHECK(LonghandToText(u8"50%", PropertyId::Opacity) == "0.5");
    CHECK(LonghandToText(u8"10px", PropertyId::Width) == "10px");
    CHECK(LonghandToText(u8"red", PropertyId::Color) == "red");

    // Through the tokenizer.
    CHECK(LonghandToText(u8"  calc(1px + 2px) ", PropertyId::Width) == "calc(3px)");
    CHECK(LonghandToText(u8"inherit", PropertyId::Width) == "inherit");
    CHECK(LonghandToText(u8"rgb(0 0 0 / 50%)", PropertyId::Color) == "rgba(0, 0, 0, 0.5)");

    CHECK(LonghandToText(u8"", PropertyId::Width) == "<null>");
    CHECK(LonghandToText(u8"red", PropertyId::Width) == "<null>");
    CHECK(LonghandToText(u8"10px 20px", PropertyId::Width) == "<null>");

    ParserContext context {ParserMode::HTMLStandard};
    auto counterStyle =
      PropertyParser::ParseCounterStyleDescriptor(PropertyId::System, u8" fixed 3 ", context);
    REQUIRE(counterStyle);
    CHECK(CssText(*counterStyle) == "fixed 3");
    CHECK_FALSE(PropertyParser::ParseCounterStyleDescriptor(PropertyId::System, u8"cyclic 3", context));
  }

  TEST_CASE_METHOD(ValueFixture, "PropertyParser custom property syntax", "[CSS][Properties][PropertyParser]")
  {
    ParserContext context {ParserMode::HTMLStandard};
    auto isValid = [&](utf8_stringview syntax, utf8_stringview css)
    {
      auto parsed = CustomPropertySyntax::Parse(syntax);
      REQUIRE(parsed);
      Tokens tokens {css};
      return PropertyParser::IsValidCustomPropertyValueForSyntax(*parsed, tokens.Tokenizer.Tokens(), context);
    };

    CHECK(isValid(u8"*", u8"anything at all"));
    CHECK(isValid(u8"<length>", u8"10px"));
    CHECK(isValid(u8"<length>", u8" 10px"));
    CHECK_FALSE(isValid(u8"<length>", u8"red"));
    CHECK(isValid(u8"<length> | <color>", u8"red"));
    CHECK(isValid(u8"<color>#", u8"red, blue"));
    CHECK(isValid(u8"<integer>+", u8"1 2 3"));
    CHECK_FALSE(isValid(u8"<integer>+", u8"1, 2"));
    CHECK(isValid(u8"foo | bar", u8"bar"));
    CHECK_FALSE(isValid(u8"foo | bar", u8"baz"));
    CHECK(isValid(u8"<custom-ident>", u8"baz"));

    auto dependencies = [&](utf8_stringview css)
    {
      auto parsed = CustomPropertySyntax::Parse(u8"<length>");
      REQUIRE(parsed);
      Tokens tokens {css};
      return PropertyParser::CollectParsedCustomPropertyValueDependencies(*parsed, tokens.Tokenizer.Tokens(),
                                                                          context);
    };

    CHECK(dependencies(u8"10px").IsComputationallyIndependent());
    CHECK(dependencies(u8"1em").Properties == List<PropertyId> {PropertyId::FontSize});
    CHECK(dependencies(u8"1rem").RootProperties == List<PropertyId> {PropertyId::FontSize});
    CHECK(dependencies(u8"1vw").ViewportDimensions);
  }

  TEST_CASE_METHOD(ValueFixture, "PropertyParser::ParseGlobalKeyword", "[CSS][Properties][PropertyParser]")
  {
    auto parse = [](utf8_stringview css)
    {
      Tokens tokens {css};
      return PropertyParser::ParseGlobalKeyword(tokens.Tokenizer.Tokens());
    };

    CHECK(parse(u8"inherit") == GlobalKeyword::Inherit);
    CHECK(parse(u8"REVERT-LAYER") == GlobalKeyword::RevertLayer);
    CHECK_FALSE(parse(u8"red"));
    CHECK_FALSE(parse(u8"inherit inherit"));
  }

  TEST_CASE_METHOD(ValueFixture, "ValuePool::CreateFontFaceValue", "[CSS][Properties][PropertyParser]")
  {
    auto &pool = ValuePool::MainThreadPool();

    auto value = pool.CreateFontFaceValue(CSSOMStringAtom(u8"Arial, serif"));
    REQUIRE(value);
    CHECK(CssText(*value) == "Arial, serif");

    // Cached.
    CHECK(pool.CreateFontFaceValue(CSSOMStringAtom(u8"Arial, serif")) == value);

    CHECK_FALSE(pool.CreateFontFaceValue(CSSOMStringAtom(u8"10px")));
  }
}
