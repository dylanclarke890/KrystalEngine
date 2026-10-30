#include "Krystal.Booey/CSS/Properties/Consumers/List.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::CssText;
  using css::tests::ValueFixture;

  namespace
  {
    struct Tokens
    {
      InputStream Input;
      Tokenizer Tokenizer;

      explicit Tokens(utf8_stringview css) : Input(CSSOMString(css)), Tokenizer(Input)
      {
        REQUIRE(Tokenizer.PumpTokenizer());
      }
    };

    constexpr auto ConsumeIdentValue = [](TokenRange &tokens)
    {
      return ConsumeIdent(tokens);
    };

    template <char Separator, ListBounds Bounds, ListOptimization Optimization = ListOptimization::None>
    std::string ConsumeList(utf8_stringview css, bool expectAtEnd = true)
    {
      Tokens tokens {css};
      auto range = tokens.Tokenizer.Tokens();
      auto list = ConsumeListSeparatedBy<Separator, Bounds, Optimization>(range, ConsumeIdentValue);
      if (!list)
      {
        return "<null>";
      }

      CHECK(range.IsAtEnd() == expectAtEnd);
      return CssText(*list);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "PropertyParserHelpers(ConsumeListSeparatedBy)",
                   "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeList<',', OneOrMore>(u8"a, b ,c") == "a, b, c");
    CHECK(ConsumeList<'/', OneOrMore>(u8"a / b") == "a / b");
    CHECK(ConsumeList<' ', OneOrMore>(u8"a b c") == "a b c");

    // A failed element fails a comma- or slash-separated list, but ends a space-separated one.
    CHECK(ConsumeList<',', OneOrMore>(u8"a, 5") == "<null>");
    CHECK(ConsumeList<'/', OneOrMore>(u8"a / 5") == "<null>");
    CHECK(ConsumeList<' ', OneOrMore>(u8"a b 5", false) == "a b");
    CHECK(ConsumeList<' ', OneOrMore>(u8"5", false) == "<null>");
    CHECK(ConsumeList<' ', ZeroOrMore>(u8"5", false) == "");
  }

  TEST_CASE_METHOD(ValueFixture, "PropertyParserHelpers(ConsumeListSeparatedBy bounds)",
                   "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeList<',', ListBounds::Exactly(2)>(u8"a, b") == "a, b");
    CHECK(ConsumeList<',', ListBounds::Exactly(2)>(u8"a") == "<null>");
    CHECK(ConsumeList<',', ListBounds::Exactly(2)>(u8"a, b, c") == "<null>");
    CHECK(ConsumeList<',', ListBounds::MinimumOf(2)>(u8"a, b, c") == "a, b, c");
    CHECK(ConsumeList<' ', ListBounds {1, 2}>(u8"a b c") == "<null>");
  }

  TEST_CASE_METHOD(ValueFixture, "PropertyParserHelpers(ConsumeListSeparatedBy single value)",
                   "[CSS][Properties][Consumers]")
  {
    Tokens tokens {u8"a"};
    auto range = tokens.Tokenizer.Tokens();
    auto single =
      ConsumeListSeparatedBy<',', OneOrMore, ListOptimization::SingleValue>(range, ConsumeIdentValue);
    REQUIRE(single);
    CHECK(single->IsPrimitiveValue());

    Tokens many {u8"a, b"};
    range = many.Tokenizer.Tokens();
    auto list =
      ConsumeListSeparatedBy<',', OneOrMore, ListOptimization::SingleValue>(range, ConsumeIdentValue);
    REQUIRE(list);
    CHECK(list->IsValueListValue());
    CHECK(CssText(*list) == "a, b");
  }
}
