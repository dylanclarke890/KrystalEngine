#include "Krystal.Booey/CSS/Values/PaintImageValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    Ref<VariableData> Arguments(utf8_stringview css)
    {
      if (css.empty())
      {
        return VariableData::Create(css::TokenRange(Span<const Token> {}), StrictParserContext());
      }

      InputStream input {CSSOMString(css)};
      Tokenizer tokenizer {input};
      REQUIRE(tokenizer.PumpTokenizer());
      return VariableData::Create(tokenizer.Tokens(), StrictParserContext());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "PaintImageValue::Create", "[CSS][Values][PaintImageValue]")
  {
    auto value = PaintImageValue::Create(u8"checkerboard", Arguments(u8""));
    CHECK(value->IsPaintImageValue());
    CHECK(value->IsImageGeneratorValue());
    CHECK(value->IsImage());
    CHECK(value->Name() == u8"checkerboard");
  }

  TEST_CASE_METHOD(ValueFixture, "PaintImageValue serialisation", "[CSS][Values][PaintImageValue]")
  {
    CHECK(CssText(*PaintImageValue::Create(u8"checkerboard", Arguments(u8""))) == "paint(checkerboard)");

    // The arguments aren't serialised yet (see the FIXME in PaintImageValue.cpp).
    CHECK(CssText(*PaintImageValue::Create(u8"checkerboard", Arguments(u8"red, 10px")))
          == "paint(checkerboard)");
  }

  TEST_CASE_METHOD(ValueFixture, "PaintImageValue equality and children", "[CSS][Values][PaintImageValue]")
  {
    auto a = PaintImageValue::Create(u8"a", Arguments(u8"1px"));
    CHECK(*a == *PaintImageValue::Create(u8"a", Arguments(u8"1px")));
    CHECK_FALSE(*a == *PaintImageValue::Create(u8"b", Arguments(u8"1px")));

    // Only the name is compared.
    CHECK(*a == *PaintImageValue::Create(u8"a", Arguments(u8"2px")));

    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
    CHECK(Children(*a).empty());
    CHECK_FALSE(a->MayDependOnBaseUrl());
  }
}
