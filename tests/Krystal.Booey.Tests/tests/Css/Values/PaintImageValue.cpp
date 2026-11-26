#include "Krystal.Booey/Css/Values/PaintImageValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Booey/Css/Parser/InputStream.hpp"
#include "Krystal.Booey/Css/Parser/ParserContext.hpp"
#include "Krystal.Booey/Css/Parser/Tokenizer.hpp"
#include "Krystal.Booey/Css/Values/VariableData.hpp"

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
    CHECK(Is<PaintImageValue>(*value));
    CHECK(value->IsImageGeneratorValue());
    CHECK(value->IsImage());
    CHECK(value->Name() == u8"checkerboard");
  }

  TEST_CASE_METHOD(ValueFixture, "PaintImageValue serialisation", "[CSS][Values][PaintImageValue]")
  {
    CHECK(CssText(*PaintImageValue::Create(u8"checkerboard", Arguments(u8""))) == "paint(checkerboard)");

    CHECK(CssText(*PaintImageValue::Create(u8"checkerboard", Arguments(u8"red, 10px")))
          == "paint(checkerboard, red, 10px)");
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
