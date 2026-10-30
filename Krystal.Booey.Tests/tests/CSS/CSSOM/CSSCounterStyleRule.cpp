#include "Krystal.Booey/CSS/CSSOM/CSSCounterStyleRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSCounterStyleRule", "[CSS][CSSOM][CSSCounterStyleRule]")
  {
    auto sheet =
      SheetFromText(u8"@counter-style thumbs { system: fixed 3; symbols: a \"b\"; suffix: \")\"; "
                    u8"range: 1 10; pad: 2 \"0\"; fallback: disc; negative: \"(\" \")\"; prefix: \"[\"; } "
                    u8"@counter-style plain { system: additive; additive-symbols: 5 \"V\", 1 \"I\"; }");
    auto &rule = RuleAt<CSSCounterStyleRule>(*sheet, 0);

    CHECK(rule.StyleRuleType() == RuleType::CounterStyle);
    CHECK(ToNarrow(rule.Name()) == "thumbs");
    CHECK(ToNarrow(rule.System()) == "fixed 3");
    CHECK(ToNarrow(rule.Symbols()) == "a \"b\"");
    CHECK(ToNarrow(rule.Suffix()) == "\")\"");
    CHECK(ToNarrow(rule.Prefix()) == "\"[\"");
    CHECK(ToNarrow(rule.Range()) == "1 10");
    CHECK(ToNarrow(rule.Pad()) == "2 \"0\"");
    CHECK(ToNarrow(rule.Fallback()) == "disc");
    CHECK(ToNarrow(rule.Negative()) == "\"(\" \")\"");
    CHECK(rule.SpeakAs().empty());
    CHECK(
      RuleText(rule)
      == "@counter-style thumbs { system: fixed 3; symbols: a \"b\"; negative: \"(\" \")\"; prefix: \"[\"; "
         "suffix: \")\"; pad: 2 \"0\"; range: 1 10; fallback: disc; }");

    CHECK(RuleText(*sheet->Item(1))
          == "@counter-style plain { system: additive; additive-symbols: 5 \"V\", 1 \"I\"; }");

    SECTION("setters")
    {
      rule.SetSymbols(u8"x y z");
      CHECK(ToNarrow(rule.Symbols()) == "x y z");
      rule.SetPrefix(u8"\"<\"");
      CHECK(ToNarrow(rule.Prefix()) == "\"<\"");
      rule.SetSuffix(u8"\">\"");
      CHECK(ToNarrow(rule.Suffix()) == "\">\"");
      rule.SetNegative(u8"\"-\"");
      // The suffix is always written, even when it is empty.
      CHECK(ToNarrow(rule.Negative()) == "\"-\" \"\"");
      rule.SetRange(u8"auto");
      CHECK(ToNarrow(rule.Range()) == "auto");
      rule.SetPad(u8"3 \"x\"");
      CHECK(ToNarrow(rule.Pad()) == "3 \"x\"");
      rule.SetFallback(u8"decimal");
      CHECK(ToNarrow(rule.Fallback()) == "decimal");
      rule.SetSpeakAs(u8"bullets");
      CHECK(rule.SpeakAs().empty());

      // Invalid values are ignored.
      rule.SetSymbols(u8"");
      CHECK(ToNarrow(rule.Symbols()) == "x y z");
      rule.SetPad(u8"-1 \"x\"");
      CHECK(ToNarrow(rule.Pad()) == "3 \"x\"");
    }

    SECTION("setName")
    {
      rule.SetName(u8"hands");
      CHECK(ToNarrow(rule.Name()) == "hands");
      CHECK(RuleText(rule).starts_with("@counter-style hands {"));

      // Names that can't name a counter style are ignored.
      rule.SetName(u8"none");
      rule.SetName(u8"decimal");
      rule.SetName(u8"two words");
      CHECK(ToNarrow(rule.Name()) == "hands");
    }

    SECTION("setSystem only changes an aspect of the algorithm")
    {
      rule.SetSystem(u8"fixed 7");
      CHECK(ToNarrow(rule.System()) == "fixed 7");
      // The first symbol value defaults to 1.
      rule.SetSystem(u8"fixed");
      CHECK(ToNarrow(rule.System()) == "fixed 1");

      // A different algorithm is ignored.
      rule.SetSystem(u8"cyclic");
      CHECK(ToNarrow(rule.System()) == "fixed 1");
      rule.SetSystem(u8"nonsense");
      CHECK(ToNarrow(rule.System()) == "fixed 1");
    }

    SECTION("additive symbols")
    {
      auto &additive = RuleAt<CSSCounterStyleRule>(*sheet, 1);
      additive.SetAdditiveSymbols(u8"10 \"X\", 1 \"I\"");
      CHECK(ToNarrow(additive.AdditiveSymbols()) == "10 \"X\", 1 \"I\"");
    }
  }
}
