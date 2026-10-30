#include "Krystal.Booey/CSS/Counter/CounterStyle.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleRegistry.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Rules/CounterStyleRule.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/Style/UserAgentStyle.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    /// @brief A registry with the author counter styles of @p text, over the user agent ones.
    struct CounterStyleFixture : ValueFixture
    {
      CounterStyleRegistry Registry;

      CounterStyleFixture()
      {
        style::UserAgentStyle::InitDefaultStyleSheet();
      }

      void Add(utf8_stringview text)
      {
        auto sheet = StyleSheetContents::Create(ParserContext(ParserMode::HTMLStandard));
        sheet->ParseString(CSSOMString(text));
        for (auto &rule : sheet->ChildRules())
        {
          auto *counterStyleRule = DynamicDowncast<CounterStyleRule>(*rule);
          REQUIRE(counterStyleRule);
          Registry.AddCounterStyle(counterStyleRule->Descriptors());
        }
      }

      std::string Text(utf8_stringview name, int32 value, WritingMode writingMode = {})
      {
        return ToNarrow(Registry.ResolvedCounterStyle(CSSOMStringAtom(name))->Text(value, writingMode));
      }
    };
  }

  TEST_CASE_METHOD(CounterStyleFixture, "CounterStyle: predefined counter styles", "[CSS][CounterStyle]")
  {
    REQUIRE(Text(u8"decimal", 5) == "5");
    REQUIRE(Text(u8"decimal", -3) == "-3");
    REQUIRE(Text(u8"decimal", 0) == "0");

    REQUIRE(Text(u8"lower-roman", 4) == "iv");
    REQUIRE(Text(u8"upper-roman", 1'994) == "MCMXCIV");
    // Out of range: falls back to decimal.
    REQUIRE(Text(u8"lower-roman", 0) == "0");
    REQUIRE(Text(u8"lower-roman", 4'000) == "4000");

    REQUIRE(Text(u8"lower-alpha", 1) == "a");
    REQUIRE(Text(u8"lower-alpha", 27) == "aa");
    REQUIRE(Text(u8"lower-alpha", 0) == "0");
    REQUIRE(Text(u8"lower-alpha", -1) == "-1");

    REQUIRE(Text(u8"disc", 7) == "\xE2\x80\xA2");
    REQUIRE(Text(u8"decimal-leading-zero", 5) == "05");
    REQUIRE(Text(u8"decimal-leading-zero", -5) == "-5");
    REQUIRE(Text(u8"decimal-leading-zero", 123) == "123");

    REQUIRE(Text(u8"cjk-decimal", 10) == "\xE4\xB8\x80\xE3\x80\x87");
    REQUIRE(Text(u8"simp-chinese-informal", 0) == "\xE9\x9B\xB6");
    REQUIRE(Text(u8"simp-chinese-informal", 12) == "\xE5\x8D\x81\xE4\xBA\x8C");
    REQUIRE(Text(u8"simp-chinese-informal", 20) == "\xE4\xBA\x8C\xE5\x8D\x81");
    REQUIRE(Text(u8"simp-chinese-informal", 105) == "\xE4\xB8\x80\xE7\x99\xBE\xE9\x9B\xB6\xE4\xBA\x94");
    REQUIRE(Text(u8"trad-chinese-formal", -1) == "\xE8\xB2\xA0\xE5\xA3\xB9");
    REQUIRE(Text(u8"ethiopic-numeric", 1) == "\xE1\x8D\xA9");
    REQUIRE(Text(u8"ethiopic-numeric", 100) == "\xE1\x8D\xBB");

    REQUIRE(Text(u8"disclosure-open", 1) == "\xE2\x96\xBE");
    REQUIRE(Text(u8"disclosure-closed", 1) == "\xE2\x96\xB8");

    SECTION("Unknown names resolve to decimal")
    {
      REQUIRE(Text(u8"nonsense", 3) == "3");
      REQUIRE(Registry.ResolvedCounterStyle(CSSOMStringAtom(u8"nonsense")).get()
              == CounterStyleRegistry::DecimalCounter().get());
    }
  }

  TEST_CASE_METHOD(CounterStyleFixture, "CounterStyle: author counter styles", "[CSS][CounterStyle]")
  {
    Add(u8"@counter-style fixed3 { system: fixed 3; symbols: a b c; }"
        u8"@counter-style stars { system: symbolic; symbols: '*' x; }"
        u8"@counter-style binary { system: numeric; symbols: '0' '1'; negative: '(' ')'; }"
        u8"@counter-style roman { system: additive; additive-symbols: 5 V, 1 I, 0 N; }"
        u8"@counter-style cycle { system: cyclic; symbols: x; range: 2 3; fallback: lower-alpha; }"
        u8"@counter-style padded { system: extends decimal; pad: 3 '0'; }"
        u8"@counter-style parens { system: extends lower-alpha; prefix: '('; }"
        u8"@counter-style loop-a { system: extends loop-b; }"
        u8"@counter-style loop-b { system: extends loop-a; }"
        u8"@counter-style chain { system: extends padded; }");

    REQUIRE(Registry.HasAuthorCounterStyles());

    SECTION("Systems")
    {
      REQUIRE(Text(u8"fixed3", 3) == "a");
      REQUIRE(Text(u8"fixed3", 5) == "c");
      REQUIRE(Text(u8"fixed3", 6) == "6");
      REQUIRE(Text(u8"fixed3", 2) == "2");

      REQUIRE(Text(u8"stars", 1) == "*");
      REQUIRE(Text(u8"stars", 4) == "xx");

      REQUIRE(Text(u8"binary", 5) == "101");
      REQUIRE(Text(u8"binary", 0) == "0");
      REQUIRE(Text(u8"binary", -2) == "(10)");

      REQUIRE(Text(u8"roman", 0) == "N");
      REQUIRE(Text(u8"roman", 7) == "VII");
    }

    SECTION("Ranges and fallbacks")
    {
      REQUIRE(Text(u8"cycle", 2) == "x");
      REQUIRE(Text(u8"cycle", 1) == "a");
      REQUIRE(Text(u8"cycle", 4) == "d");
    }

    SECTION("Extends")
    {
      REQUIRE(Text(u8"padded", 7) == "007");
      REQUIRE(Text(u8"padded", -7) == "-07");
      REQUIRE(Text(u8"padded", 1'234) == "1234");
      REQUIRE(Text(u8"chain", 7) == "007");

      // The prefix isn't part of the representation, but is inherited from the descriptors.
      auto parens = Registry.ResolvedCounterStyle(CSSOMStringAtom(u8"parens"));
      REQUIRE(ToNarrow(parens->Text(2, {})) == "b");
      REQUIRE(ToNarrow(parens->Prefix().Text) == "(");
      REQUIRE(parens->System() == CounterStyleSystem::Alphabetic);

      // A cycle of extends is treated as extending decimal.
      REQUIRE(Text(u8"loop-a", 12) == "12");
      REQUIRE(Text(u8"loop-b", -12) == "-12");
    }

    SECTION("Clearing the author counter styles")
    {
      Registry.ClearAuthorCounterStyles();
      REQUIRE_FALSE(Registry.HasAuthorCounterStyles());
      REQUIRE(Text(u8"fixed3", 3) == "3");
    }

    SECTION("Registries compare their counter styles")
    {
      CounterStyleRegistry other;
      REQUIRE_FALSE(other == Registry);
      REQUIRE(Registry == Registry);
    }
  }
}
