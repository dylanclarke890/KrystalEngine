#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"

// Only the Builder-free half: resolution needs `style::Builder`.
namespace krys::boo::css::tests
{
  namespace
  {
    Ref<VariableData> MakeData(utf8_stringview css, const ParserContext &context = StrictParserContext())
    {
      InputStream input {CSSOMString(css)};
      Tokenizer tokenizer {input};
      REQUIRE(tokenizer.PumpTokenizer());
      return VariableData::Create(tokenizer.Tokens(), context);
    }

    Ref<VariableReferenceValue> MakeReference(utf8_stringview css)
    {
      return VariableReferenceValue::Create(MakeData(css));
    }
  }

  TEST_CASE_METHOD(ValueFixture, "VariableReferenceValue::Create", "[CSS][Values][VariableReferenceValue]")
  {
    SECTION("From data")
    {
      auto data = MakeData(u8"var(--x, 1px) solid");
      auto *dataPointer = data.get();
      auto value = VariableReferenceValue::Create(krys::move(data));
      CHECK(value->IsVariableReferenceValue());
      CHECK(value->HasVariableReferences());
      CHECK(&value->Data() == dataPointer);
      CHECK(&value->Context() == &value->Data().Context());
    }

    SECTION("From a token range and context")
    {
      ParserContext context {ParserMode::HTMLQuirks};
      InputStream input {CSSOMString(u8"env(safe-area-inset-top)")};
      Tokenizer tokenizer {input};
      REQUIRE(tokenizer.PumpTokenizer());

      auto value = VariableReferenceValue::Create(css::TokenRange(tokenizer.Tokens()), context);
      CHECK(value->Context() == context);
      CHECK(value->Data().Tokens().size() == tokenizer.Tokens().Size());
      CHECK(CssText(*value) == "env(safe-area-inset-top)");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "VariableReferenceValue serialisation",
                   "[CSS][Values][VariableReferenceValue]")
  {
    for (auto css :
         {u8"var(--x)", u8"var(--x, 1px) solid", u8"env(safe-area-inset-top)", u8"calc(var(--a) * 2)"})
    {
      auto narrow = ToNarrow(css);
      INFO(narrow);
      auto value = MakeReference(css);
      CHECK(CssText(*value) == narrow);
      // The text is cached; serialising again gives the same result.
      CHECK(CssText(*value) == narrow);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "VariableReferenceValue equality and hashing",
                   "[CSS][Values][VariableReferenceValue]")
  {
    auto a = MakeReference(u8"var(--x)");
    auto b = MakeReference(u8"var(--x)");

    CHECK(*a == *b);
    CHECK_FALSE(*a == *MakeReference(u8"var(--y)"));
    CHECK_FALSE(*a == *MakeReference(u8"var(--x, red)"));

    // Values sharing their data are equal.
    auto data = MakeData(u8"var(--z)");
    auto shared1 = VariableReferenceValue::Create(Ref<VariableData> {data});
    auto shared2 = VariableReferenceValue::Create(Ref<VariableData> {data});
    CHECK(*shared1 == *shared2);

    // `VariableReferenceValue` has no hash of its own.
    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "VariableReferenceValue visitation", "[CSS][Values][VariableReferenceValue]")
  {
    CHECK(Children(*MakeReference(u8"var(--x, 1px)")).empty());
  }
}
