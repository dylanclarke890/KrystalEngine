#include "Krystal.Booey/CSS/Calc/TreeParser.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Calc/AllowedSymbols.hpp"
#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserOptions.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/NoConversionDataRequiredTag.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericCategory.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/Style/AnchorSizeDimension.hpp"
#include <cmath>
#include <string>

namespace krys::boo::css::calc::tests
{
  namespace
  {
    struct ParseOptions
    {
      css::Category Category {css::Category::Length};
      css::Range Range {All};
      ParserContext Context {ParserMode::HTMLStandard};
      css::AllowedSymbols Symbols {};
      PropertyParserOptions PropertyOptions {};
      RuleType Rule {RuleType::Style};
      PropertyId Property {PropertyId::Width};
    };

    /// @brief Tokenizes `css` and parses it as a calc value, keeping the tokens and state for inspection.
    struct Parsed
    {
      List<Token> Tokens;
      RefPtr<Value> Result;
      bool AtEnd {false};
      uint32 RandomFunctionCount {0};
    };

    Parsed Parse(utf8_stringview css, const ParseOptions &options = {})
    {
      InputStream input {CSSOMString(css)};
      Tokenizer tokenizer {input};
      REQUIRE(tokenizer.PumpTokenizer());

      Parsed parsed;
      auto tokens = tokenizer.Tokens();
      PropertyParserState state {
        .Context = options.Context,
        .CurrentRule = options.Rule,
        .CurrentProperty = options.Property,
      };

      parsed.Result = Value::Parse(tokens, state, options.Category, options.Range, options.Symbols,
                                   options.PropertyOptions);
      parsed.AtEnd = tokens.IsAtEnd();
      parsed.RandomFunctionCount = state.CSSRandomFunctionCount;
      return parsed;
    }

    RefPtr<Value> ParseValue(utf8_stringview css, const ParseOptions &options = {})
    {
      return Parse(css, options).Result;
    }

    // Catch2 can't print UTF-8 strings, so serialise to `std::string` for readable failures.
    std::string Serialise(utf8_stringview css, const ParseOptions &options = {})
    {
      auto value = ParseValue(css, options);
      if (!value)
      {
        return "<null>";
      }

      auto text = value->CssText(DefaultSerialisationContext());
      return std::string(text.begin(), text.end());
    }

    double Evaluate(utf8_stringview css, const ParseOptions &options = {})
    {
      auto value = ParseValue(css, options);
      REQUIRE(value);
      return value->DoubleValue(NoConversionDataRequiredTag {});
    }

    ParseOptions WithCategory(css::Category category)
    {
      return ParseOptions {.Category = category};
    }
  }

  TEST_CASE("TreeParser(IsCalcFunction)", "[CSS][Calc][TreeParser]")
  {
    CHECK(IsCalcFunction(ValueId::Calc));
    CHECK(IsCalcFunction(ValueId::WebkitCalc));
    CHECK(IsCalcFunction(ValueId::Clamp));
    CHECK(IsCalcFunction(ValueId::AnchorSize));
    CHECK_FALSE(IsCalcFunction(ValueId::Rgb));
    CHECK_FALSE(IsCalcFunction(ValueId::Invalid));
  }

  TEST_CASE("TreeParser(Arithmetic)", "[CSS][Calc][TreeParser]")
  {
    CHECK(Serialise(u8"calc(1px + 2px)") == "calc(3px)");
    CHECK(Serialise(u8"calc(1px - 2px)") == "calc(-1px)");
    CHECK(Serialise(u8"calc(10px / 4)") == "calc(2.5px)");
    CHECK(Serialise(u8"calc((1px + 2px) * 2)") == "calc(6px)");
    CHECK(Serialise(u8"calc(1in + 4px)") == "calc(100px)");
    CHECK(Serialise(u8"-webkit-calc(1px + 1px)") == "calc(2px)");
    CHECK(Serialise(u8"calc(2 * 3)", WithCategory(css::Category::Number)) == "calc(6)");
    CHECK(Serialise(u8"calc(1px + 2em)") == "calc(2em + 1px)");
    CHECK(Serialise(u8"calc(50% + 10px)", WithCategory(css::Category::LengthPercentage))
          == "calc(50% + 10px)");
    CHECK(Serialise(u8"CALC(1PX + 2Px)") == "calc(3px)");
  }

  TEST_CASE("TreeParser(Whitespace)", "[CSS][Calc][TreeParser]")
  {
    // '+' and '-' must be surrounded by whitespace; '*' and '/' needn't be.
    CHECK_FALSE(ParseValue(u8"calc(1px+ 2px)"));
    CHECK_FALSE(ParseValue(u8"calc(1px +2px)"));
    CHECK_FALSE(ParseValue(u8"calc(1px -2px)"));
    CHECK(Serialise(u8"calc(2px*3)") == "calc(6px)");
    CHECK(Serialise(u8"calc( 1px  +  2px )") == "calc(3px)");
  }

  TEST_CASE("TreeParser(TypeChecking)", "[CSS][Calc][TreeParser]")
  {
    CHECK_FALSE(ParseValue(u8"calc(1px)", WithCategory(css::Category::Number)));
    CHECK_FALSE(ParseValue(u8"calc(5)"));
    CHECK_FALSE(ParseValue(u8"calc(1px * 2px)"));
    CHECK_FALSE(ParseValue(u8"calc(1px + 1s)"));
    CHECK_FALSE(ParseValue(u8"calc(1px + 1)"));
    CHECK_FALSE(ParseValue(u8"calc(50%)", WithCategory(css::Category::Length)));
    CHECK(ParseValue(u8"calc(1px / 2)"));
    CHECK(ParseValue(u8"calc(10deg)", WithCategory(css::Category::Angle)));
    CHECK(ParseValue(u8"calc(1s)", WithCategory(css::Category::Time)));
  }

  TEST_CASE("TreeParser(Failures)", "[CSS][Calc][TreeParser]")
  {
    CHECK_FALSE(ParseValue(u8"foo(1px)"));
    CHECK_FALSE(ParseValue(u8"1px"));
    CHECK_FALSE(ParseValue(u8"calc()"));
    CHECK_FALSE(ParseValue(u8"calc(1foo)"));
    CHECK_FALSE(ParseValue(u8"calc(1px 2px)"));
    CHECK_FALSE(ParseValue(u8"calc(1px +)"));
    CHECK_FALSE(ParseValue(u8"calc(foo)"));
    CHECK_FALSE(ParseValue(u8"calc(\"1px\")"));
    CHECK_FALSE(ParseValue(u8"calc(1px,)"));
  }

  TEST_CASE("TreeParser(Depth)", "[CSS][Calc][TreeParser]")
  {
    auto nested = [](size_t depth)
    {
      return CSSOMString(u8"calc(") + CSSOMString(depth, u8'(') + u8"1px" + CSSOMString(depth, u8')') + u8")";
    };

    CHECK(ParseValue(nested(99)));
    CHECK_FALSE(ParseValue(nested(101)));
  }

  TEST_CASE("TreeParser(Range)", "[CSS][Calc][TreeParser]")
  {
    // The function and the whitespace after it are consumed; the rest of the range is left.
    auto parsed = Parse(u8"calc(1px) 2px");
    CHECK(parsed.Result);
    CHECK_FALSE(parsed.AtEnd);

    parsed = Parse(u8"calc(1px)  ");
    CHECK(parsed.Result);
    CHECK(parsed.AtEnd);
  }

  TEST_CASE("TreeParser(Constants)", "[CSS][Calc][TreeParser]")
  {
    auto number = WithCategory(css::Category::Number);
    CHECK(Evaluate(u8"calc(pi)", number) == Catch::Approx(3.14159265358979));
    CHECK(Evaluate(u8"calc(e)", number) == Catch::Approx(2.71828182845905));
    CHECK(Serialise(u8"calc(infinity)", number) == "calc(infinity)");
    CHECK(Serialise(u8"calc(-infinity)", number) == "calc(-infinity)");
    CHECK(Serialise(u8"calc(NaN)", number) == "calc(NaN)");
    CHECK(Serialise(u8"calc(1px * infinity)") == "calc(infinity * 1px)");

    auto value = ParseValue(u8"calc(pi)", number);
    REQUIRE(value);
    CHECK_FALSE(value->RequiresConversionData());
  }

  TEST_CASE("TreeParser(RequiresConversionData)", "[CSS][Calc][TreeParser]")
  {
    auto em = ParseValue(u8"calc(1em + 1px)");
    REQUIRE(em);
    CHECK(em->RequiresConversionData());

    // Absolute lengths other than px are zoomed, so they need conversion data too.
    auto in = ParseValue(u8"calc(1in + 1px)");
    REQUIRE(in);
    CHECK(in->RequiresConversionData());

    auto px = ParseValue(u8"calc(1px + 1px)");
    REQUIRE(px);
    CHECK_FALSE(px->RequiresConversionData());

    auto angle = ParseValue(u8"calc(1turn + 1deg)", WithCategory(css::Category::Angle));
    REQUIRE(angle);
    CHECK_FALSE(angle->RequiresConversionData());
  }

  TEST_CASE("TreeParser(Symbols)", "[CSS][Calc][TreeParser]")
  {
    auto options = WithCategory(css::Category::Number);
    CHECK_FALSE(ParseValue(u8"calc(alpha * 2)", options));

    options.Symbols = css::AllowedSymbols {{{ValueId::Alpha, UnitType::Number}}};
    auto value = ParseValue(u8"calc(alpha * 2)", options);
    REQUIRE(value);
    CHECK(value->DoubleValue(NoConversionDataRequiredTag {},
                             SymbolTable {{ValueId::Alpha, UnitType::Number, 0.25}})
          == 0.5);

    // Constants are still available.
    CHECK(ParseValue(u8"calc(alpha * pi)", options));
  }

  TEST_CASE("TreeParser(MinMaxClamp)", "[CSS][Calc][TreeParser]")
  {
    CHECK(Serialise(u8"min(1px, 2px)") == "calc(1px)");
    CHECK(Serialise(u8"max(1px, 2px, 3px)") == "calc(3px)");
    CHECK(Serialise(u8"max(1px, 1em)") == "max(1px, 1em)");
    CHECK(Serialise(u8"calc(min(1px, 2px) + 1px)") == "calc(2px)");
    CHECK(Serialise(u8"clamp(1px, 5px, 3px)") == "calc(3px)");
    CHECK(Serialise(u8"clamp(none, 5px, 3px)") == "calc(3px)");
    CHECK(Serialise(u8"clamp(4px, 1px, none)") == "calc(4px)");
    CHECK(Serialise(u8"clamp(none, 2px, none)") == "calc(2px)");
    CHECK(Serialise(u8"clamp(1px, 1em, 3px)") == "clamp(1px, 1em, 3px)");

    CHECK_FALSE(ParseValue(u8"min()"));
    CHECK_FALSE(ParseValue(u8"min(1px 2px)"));
    CHECK_FALSE(ParseValue(u8"min(1px, 1s)"));
    CHECK_FALSE(ParseValue(u8"clamp(1px, 2px)"));
    CHECK_FALSE(ParseValue(u8"clamp(1px, none, 3px)"));
    CHECK_FALSE(ParseValue(u8"clamp(1px, 2px, 3px, 4px)"));
  }

  TEST_CASE("TreeParser(Round)", "[CSS][Calc][TreeParser]")
  {
    auto number = WithCategory(css::Category::Number);
    CHECK(Evaluate(u8"round(2.5)", number) == 3.0);
    CHECK(Serialise(u8"round(down, 7px, 2px)") == "calc(6px)");
    CHECK(Serialise(u8"round(up, 7px, 2px)") == "calc(8px)");
    CHECK(Serialise(u8"round(to-zero, -7px, 2px)") == "calc(-6px)");
    CHECK(Serialise(u8"round(nearest, 7px, 2px)") == "calc(8px)");
    CHECK(Serialise(u8"round(up, 1em, 2px)") == "round(up, 1em, 2px)");

    // A single argument must be a number.
    CHECK_FALSE(ParseValue(u8"round(7px)"));
    CHECK_FALSE(ParseValue(u8"round(up 7px, 2px)"));
    CHECK_FALSE(ParseValue(u8"round(7px, 2s)"));
    CHECK_FALSE(ParseValue(u8"round(up, 7px, 2px, 1px)"));
  }

  TEST_CASE("TreeParser(SteppedValueFunctions)", "[CSS][Calc][TreeParser]")
  {
    auto number = WithCategory(css::Category::Number);
    CHECK(Evaluate(u8"mod(7, 3)", number) == 1.0);
    CHECK(Evaluate(u8"rem(-7, 3)", number) == -1.0);
    CHECK(Evaluate(u8"mod(-7, 3)", number) == 2.0);
    CHECK(Serialise(u8"mod(7px, 3px)") == "calc(1px)");

    CHECK_FALSE(ParseValue(u8"mod(7px)"));
    CHECK_FALSE(ParseValue(u8"mod(7px, 3s)"));
    CHECK_FALSE(ParseValue(u8"rem(7px, 3px, 1px)"));
  }

  TEST_CASE("TreeParser(TrigonometricFunctions)", "[CSS][Calc][TreeParser]")
  {
    auto number = WithCategory(css::Category::Number);
    auto angle = WithCategory(css::Category::Angle);
    CHECK(Evaluate(u8"sin(90deg)", number) == Catch::Approx(1.0));
    CHECK(Evaluate(u8"cos(0)", number) == Catch::Approx(1.0));
    CHECK(Evaluate(u8"tan(45deg)", number) == Catch::Approx(1.0));
    CHECK(Evaluate(u8"asin(1)", angle) == Catch::Approx(90.0));
    CHECK(Evaluate(u8"acos(1)", angle) == Catch::Approx(0.0));
    CHECK(Evaluate(u8"atan(1)", angle) == Catch::Approx(45.0));
    CHECK(Evaluate(u8"atan2(1px, 1px)", angle) == Catch::Approx(45.0));

    CHECK_FALSE(ParseValue(u8"sin(1px)", number));
    CHECK_FALSE(ParseValue(u8"asin(1deg)", angle));
    CHECK_FALSE(ParseValue(u8"sin(90deg)", angle));
    CHECK_FALSE(ParseValue(u8"sin(1, 2)", number));
    CHECK_FALSE(ParseValue(u8"atan2(1px, 1s)", angle));
  }

  TEST_CASE("TreeParser(ExponentialFunctions)", "[CSS][Calc][TreeParser]")
  {
    auto number = WithCategory(css::Category::Number);
    CHECK(Evaluate(u8"pow(2, 3)", number) == 8.0);
    CHECK(Evaluate(u8"sqrt(16)", number) == 4.0);
    CHECK(Evaluate(u8"log(8, 2)", number) == Catch::Approx(3.0));
    CHECK(Evaluate(u8"log(e)", number) == Catch::Approx(1.0));
    CHECK(Evaluate(u8"exp(0)", number) == 1.0);
    CHECK(Serialise(u8"hypot(3px, 4px)") == "calc(5px)");

    CHECK_FALSE(ParseValue(u8"pow(2)", number));
    CHECK_FALSE(ParseValue(u8"pow(2px, 2)"));
    CHECK_FALSE(ParseValue(u8"sqrt(4px)"));
    CHECK_FALSE(ParseValue(u8"log(8, 2, 1)", number));
    CHECK_FALSE(ParseValue(u8"hypot(3px, 4s)"));
  }

  TEST_CASE("TreeParser(SignRelatedFunctions)", "[CSS][Calc][TreeParser]")
  {
    CHECK(Serialise(u8"abs(-2px)") == "calc(2px)");
    CHECK(Evaluate(u8"sign(-3px)", WithCategory(css::Category::Number)) == -1.0);
    CHECK(Serialise(u8"abs(1em)") == "calc(1em)");
    CHECK(Serialise(u8"abs(1em + 1px)") == "abs(1em + 1px)");
    CHECK_FALSE(ParseValue(u8"sign(-3px)"));
  }

  TEST_CASE("TreeParser(Progress)", "[CSS][Calc][TreeParser]")
  {
    auto options = WithCategory(css::Category::Number);
    CHECK_FALSE(ParseValue(u8"progress(5px, 0px, 10px)", options));

    options.Context.cssProgressFunctionEnabled = true;
    CHECK(Evaluate(u8"progress(5px, 0px, 10px)", options) == 0.5);
    CHECK_FALSE(ParseValue(u8"progress(5px, 0px)", options));
    CHECK_FALSE(ParseValue(u8"progress(5px, 0s, 10px)", options));
  }

  TEST_CASE("TreeParser(TreeCountingFunctions)", "[CSS][Calc][TreeParser]")
  {
    auto options = WithCategory(css::Category::Integer);
    CHECK_FALSE(ParseValue(u8"calc(sibling-index())", options));

    options.Context.cssTreeCountingFunctionsEnabled = true;
    CHECK(ParseValue(u8"calc(sibling-index())", options));
    CHECK(ParseValue(u8"calc(sibling-count() + 1)", options));
    CHECK_FALSE(ParseValue(u8"calc(sibling-index(1))", options));

    auto invalidProperty = options;
    invalidProperty.Property = PropertyId::Invalid;
    CHECK_FALSE(ParseValue(u8"calc(sibling-index())", invalidProperty));

    auto mediaRule = options;
    mediaRule.Rule = RuleType::Media;
    CHECK_FALSE(ParseValue(u8"calc(sibling-index())", mediaRule));

    auto keyframeRule = options;
    keyframeRule.Rule = RuleType::Keyframe;
    CHECK(ParseValue(u8"calc(sibling-index())", keyframeRule));
  }

  TEST_CASE("TreeParser(Random)", "[CSS][Calc][TreeParser]")
  {
    auto options = ParseOptions {};
    CHECK_FALSE(ParseValue(u8"random(1px, 10px)", options));

    options.Context.cssRandomFunctionEnabled = true;
    auto parsed = Parse(u8"random(1px, 10px)", options);
    REQUIRE(parsed.Result);
    CHECK(parsed.Result->RequiresConversionData());
    CHECK(parsed.RandomFunctionCount == 1);

    auto random = SwitchOn(
      parsed.Result->GetTree().Root, [](const IndirectNode<Random> &node) -> const Random *
      { return &*node; }, [](const auto &) -> const Random * { return nullptr; });
    REQUIRE(random);
    REQUIRE(HoldsAlternative<Random::SharingOptions>(random->Sharing));
    auto &sharing = std::get<Random::SharingOptions>(random->Sharing);
    CHECK(sharing.Identifier
          == Variant<Random::SharingOptions::Auto, CSSOMStringAtom> {
            Random::SharingOptions::Auto {.Property = PropertyId::Width, .Index = 0}});
    CHECK_FALSE(sharing.ElementShared);

    // Nested random() functions see an incremented index.
    parsed = Parse(u8"random(1px, random(2px, 3px))", options);
    REQUIRE(parsed.Result);
    CHECK(parsed.RandomFunctionCount == 2);

    CHECK(ParseValue(u8"random(1px, 10px, 1px)", options));
    CHECK(ParseValue(u8"random(auto, 1px, 10px)", options));
    CHECK(ParseValue(u8"random(--foo, 1px, 10px)", options));
    CHECK(ParseValue(u8"random(element-shared, 1px, 10px)", options));
    CHECK(ParseValue(u8"random(element-shared --foo, 1px, 10px)", options));
    CHECK(ParseValue(u8"random(fixed 0.5, 1px, 10px)", options));

    auto dashed = Parse(u8"random(--foo element-shared, 1px, 10px)", options);
    REQUIRE(dashed.Result);
    auto dashedRandom = SwitchOn(
      dashed.Result->GetTree().Root, [](const IndirectNode<Random> &node) -> const Random *
      { return &*node; }, [](const auto &) -> const Random * { return nullptr; });
    REQUIRE(dashedRandom);
    auto &dashedSharing = std::get<Random::SharingOptions>(dashedRandom->Sharing);
    CHECK(dashedSharing.Identifier
          == Variant<Random::SharingOptions::Auto, CSSOMStringAtom> {CSSOMStringAtom(u8"--foo")});
    CHECK(dashedSharing.ElementShared);

    CHECK_FALSE(ParseValue(u8"random(1px)", options));
    CHECK_FALSE(ParseValue(u8"random(1px, 10s)", options));
    CHECK_FALSE(ParseValue(u8"random(auto auto, 1px, 10px)", options));
    CHECK_FALSE(ParseValue(u8"random(foo, 1px, 10px)", options));
    CHECK_FALSE(ParseValue(u8"random(auto 1px, 10px)", options));
    CHECK_FALSE(ParseValue(u8"random(fixed 2, 1px, 10px)", options));
    CHECK_FALSE(ParseValue(u8"random(fixed, 1px, 10px)", options));
    CHECK_FALSE(ParseValue(u8"random(1px, 10px, 1px, 1px)", options));

    auto custom = options;
    custom.Property = PropertyId::Custom;
    CHECK_FALSE(ParseValue(u8"random(1px, 10px)", custom));

    auto invalidProperty = options;
    invalidProperty.Property = PropertyId::Invalid;
    CHECK_FALSE(ParseValue(u8"random(1px, 10px)", invalidProperty));

    auto mediaRule = options;
    mediaRule.Rule = RuleType::Media;
    CHECK_FALSE(ParseValue(u8"random(1px, 10px)", mediaRule));
  }

  TEST_CASE("TreeParser(Anchor)", "[CSS][Calc][TreeParser]")
  {
    auto options = WithCategory(css::Category::LengthPercentage);
    CHECK_FALSE(ParseValue(u8"anchor(--a top)", options));

    options.PropertyOptions.AnchorPolicy = AllowAnchor {true};
    CHECK_FALSE(ParseValue(u8"anchor(--a top)", options));

    options.Context.PropertySettings.cssAnchorPositioningEnabled = true;
    auto value = ParseValue(u8"anchor(--a top)", options);
    REQUIRE(value);
    CHECK(value->RequiresConversionData());

    auto anchor = SwitchOn(
      value->GetTree().Root, [](const IndirectNode<Anchor> &node) -> const Anchor * { return &*node; },
      [](const auto &) -> const Anchor * { return nullptr; });
    REQUIRE(anchor);
    CHECK(anchor->ElementName == CSSOMStringAtom(u8"--a"));
    CHECK(anchor->Side == AnchorSide {ValueId::Top});
    CHECK_FALSE(anchor->Fallback);

    CHECK(ParseValue(u8"anchor(top --a)", options));
    CHECK(ParseValue(u8"anchor(top)", options));
    CHECK(ParseValue(u8"anchor(--a 50%)", options));
    CHECK(ParseValue(u8"anchor(--a top, 10px)", options));
    CHECK(ParseValue(u8"anchor(--a top, 10%)", options));
    CHECK(ParseValue(u8"calc(anchor(--a top) + 10px)", options));

    auto noElement = ParseValue(u8"anchor(top, 10px)", options);
    REQUIRE(noElement);
    auto noElementAnchor = SwitchOn(
      noElement->GetTree().Root, [](const IndirectNode<Anchor> &node) -> const Anchor * { return &*node; },
      [](const auto &) -> const Anchor * { return nullptr; });
    REQUIRE(noElementAnchor);
    CHECK(noElementAnchor->ElementName == CSSOMStringAtom::Null());
    CHECK(noElementAnchor->Fallback);

    CHECK_FALSE(ParseValue(u8"anchor(--a)", options));
    CHECK_FALSE(ParseValue(u8"anchor(--a top, 1s)", options));
    CHECK_FALSE(ParseValue(u8"anchor(--a 10px)", options));
    CHECK_FALSE(ParseValue(u8"anchor(--a top 10px)", options));
  }

  TEST_CASE("TreeParser(AnchorSize)", "[CSS][Calc][TreeParser]")
  {
    auto options = WithCategory(css::Category::LengthPercentage);
    options.Context.PropertySettings.cssAnchorPositioningEnabled = true;
    CHECK_FALSE(ParseValue(u8"anchor-size(width)", options));

    options.PropertyOptions.AnchorSizePolicy = AllowAnchorSize {true};
    auto value = ParseValue(u8"anchor-size(--a height, 10px)", options);
    REQUIRE(value);
    CHECK(value->RequiresConversionData());

    auto anchorSize = SwitchOn(
      value->GetTree().Root, [](const IndirectNode<AnchorSize> &node) -> const AnchorSize *
      { return &*node; }, [](const auto &) -> const AnchorSize * { return nullptr; });
    REQUIRE(anchorSize);
    CHECK(anchorSize->ElementName == CSSOMStringAtom(u8"--a"));
    CHECK(anchorSize->Dimension == style::AnchorSizeDimension::Height);
    CHECK(anchorSize->Fallback);

    CHECK(ParseValue(u8"anchor-size()", options));
    CHECK(ParseValue(u8"anchor-size(width)", options));
    CHECK(ParseValue(u8"anchor-size(self-inline --a)", options));
    CHECK(ParseValue(u8"anchor-size(--a)", options));
    CHECK(ParseValue(u8"anchor-size(10px)", options));

    CHECK_FALSE(ParseValue(u8"anchor-size(1s)", options));
    CHECK_FALSE(ParseValue(u8"anchor-size(width, 1s)", options));
    CHECK_FALSE(ParseValue(u8"anchor-size(top)", options));
    CHECK_FALSE(ParseValue(u8"anchor-size(foo)", options));
  }
}
