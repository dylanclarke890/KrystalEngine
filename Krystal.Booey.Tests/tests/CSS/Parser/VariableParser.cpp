#include "Krystal.Booey/CSS/Parser/VariableParser.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    /// @brief Keeps the tokens alive for the range.
    struct Tokens
    {
      InputStream Input;
      Tokenizer Tokenizer;

      explicit Tokens(utf8_stringview css) : Input(CSSOMString(css)), Tokenizer(Input)
      {
        Tokenizer.PumpTokenizer();
      }

      KRYS_NODISCARD TokenRange Range() const noexcept
      {
        return Tokenizer.Tokens();
      }
    };

    bool ContainsValidVariableReferences(utf8_stringview css,
                                         const ParserContext &context = StrictParserContext())
    {
      Tokens tokens(css);
      return VariableParser::ContainsValidVariableReferences(tokens.Range(), context);
    }

    RefPtr<CustomPropertyValue> ParseDeclarationValue(utf8_stringview css)
    {
      Tokens tokens(css);
      return VariableParser::ParseDeclarationValue(CSSOMStringAtom(u8"--x"), tokens.Range(),
                                                   StrictParserContext());
    }
  }

  TEST_CASE("VariableParser::IsValidVariableName", "[CSS][Parser][VariableParser]")
  {
    auto isValid = [](utf8_stringview css)
    {
      Tokens tokens(css);
      return VariableParser::IsValidVariableName(tokens.Range().Peek());
    };

    CHECK(isValid(u8"--x"));
    CHECK(isValid(u8"--foo-bar"));
    CHECK_FALSE(isValid(u8"--"));
    CHECK_FALSE(isValid(u8"-x"));
    CHECK_FALSE(isValid(u8"x"));
    CHECK_FALSE(isValid(u8"--x("));
    CHECK_FALSE(isValid(u8"'--x'"));
  }

  TEST_CASE("VariableParser::ContainsValidVariableReferences", "[CSS][Parser][VariableParser]")
  {
    SECTION("References")
    {
      for (auto css :
           {u8"var(--x)", u8" var( --x ) ", u8"var(--x,)", u8"var(--x, 1px)", u8"var(--x, var(--y))",
            u8"calc(var(--x) + 1px)", u8"1px var(--x)", u8"env(safe-area-inset-top)", u8"env(foo, 1px)",
            u8"VAR(--x)", u8"var(--x, {a})", u8"{var(--x)}"})
      {
        CAPTURE(ToNarrow(css));
        CHECK(ContainsValidVariableReferences(css));
      }
    }

    SECTION("No references")
    {
      for (auto css : {u8"", u8"1px", u8"red blue", u8"initial", u8"--x", u8"foo(bar)", u8"{a}"})
      {
        CAPTURE(ToNarrow(css));
        CHECK_FALSE(ContainsValidVariableReferences(css));
      }
    }

    SECTION("Invalid references")
    {
      for (auto css : {u8"var()", u8"var(x)", u8"var(--x y)", u8"var(--x;)", u8"var(1px)", u8"env()",
                       u8"env(1px)", u8"var(--x, ]", u8"var(--x, !)", u8"var(--x, a; b)"})
      {
        CAPTURE(ToNarrow(css));
        CHECK_FALSE(ContainsValidVariableReferences(css));
      }
    }

    SECTION("Invalid at the top level")
    {
      for (auto css : {u8"var(--x) !", u8"var(--x);", u8"var(--x) )", u8"var(--x) ]", u8"var(--x) }",
                       u8"var(--x) 'a\n'", u8"var(--x) url(a b)"})
      {
        CAPTURE(ToNarrow(css));
        CHECK_FALSE(ContainsValidVariableReferences(css));
      }
    }

    SECTION("Top-level brace blocks must be alone")
    {
      CHECK(ContainsValidVariableReferences(u8"{var(--x)}"));
      CHECK(ContainsValidVariableReferences(u8" {var(--x)} "));
      CHECK_FALSE(ContainsValidVariableReferences(u8"{var(--x)} a"));
      CHECK_FALSE(ContainsValidVariableReferences(u8"{var(--x)} {}"));
      // Nested blocks don't count.
      CHECK(ContainsValidVariableReferences(u8"a (b {var(--x)} c)"));
    }

    SECTION("Dashed functions")
    {
      ParserContext context(ParserMode::HTMLStandard);
      CHECK_FALSE(ContainsValidVariableReferences(u8"--f(1px)", context));

      context.PropertySettings.cssFunctionAtRuleEnabled = true;
      for (auto css : {u8"--f()", u8"--f(1px)", u8"--f(1px, red)", u8"--f({a, b}, c)"})
      {
        CAPTURE(ToNarrow(css));
        CHECK(ContainsValidVariableReferences(css, context));
      }
      for (auto css : {u8"--f(1px,)", u8"--f(,1px)", u8"--f({a} b)", u8"--f({})", u8"--f(;)"})
      {
        CAPTURE(ToNarrow(css));
        CHECK_FALSE(ContainsValidVariableReferences(css, context));
      }
    }
  }

  TEST_CASE_METHOD(ValueFixture, "VariableParser::ParseDeclarationValue", "[CSS][Parser][VariableParser]")
  {
    SECTION("Global keyword")
    {
      auto value = ParseDeclarationValue(u8" inherit ");
      REQUIRE(value);
      CHECK(value->Name() == CSSOMStringAtom(u8"--x"));
      CHECK(value->TryGlobalKeyword() == GlobalKeyword::Inherit);
    }

    SECTION("A keyword followed by more tokens is data")
    {
      auto value = ParseDeclarationValue(u8"inherit 1px");
      REQUIRE(value);
      CHECK_FALSE(value->IsGlobalKeyword());
      CHECK(value->IsVariableData());
    }

    SECTION("References")
    {
      auto value = ParseDeclarationValue(u8"var(--y) 1px");
      REQUIRE(value);
      CHECK(value->IsVariableReference());
      CHECK(CssText(*value) == "var(--y) 1px");
    }

    SECTION("Data")
    {
      auto value = ParseDeclarationValue(u8"1px red");
      REQUIRE(value);
      CHECK(value->IsVariableData());
      CHECK(CssText(*value) == "1px red");

      auto empty = ParseDeclarationValue(u8"");
      REQUIRE(empty);
      CHECK(empty->IsVariableData());
    }

    SECTION("A brace block mixed with other values is still data")
    {
      auto value = ParseDeclarationValue(u8"{a} b");
      REQUIRE(value);
      CHECK(value->IsVariableData());
    }

    SECTION("Invalid")
    {
      for (auto css : {u8"!", u8"a;b", u8")", u8"var(x)", u8"'a\n'"})
      {
        CAPTURE(ToNarrow(css));
        CHECK_FALSE(ParseDeclarationValue(css));
      }
    }
  }
}
