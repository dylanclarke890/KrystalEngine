#include "Krystal.Booey/CSS/Values/VariableData.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD string ToNarrow(utf8_stringview str) noexcept
    {
      return string(reinterpret_cast<const char *>(str.data()), str.size());
    }

    Ref<VariableData> MakeData(utf8_stringview css, const ParserContext &context = StrictParserContext())
    {
      if (css.empty())
      {
        return VariableData::Create(css::TokenRange(Span<const Token> {}), context);
      }

      InputStream input {CSSOMString(css)};
      Tokenizer tokenizer {input};
      REQUIRE(tokenizer.PumpTokenizer());
      return VariableData::Create(tokenizer.Tokens(), context);
    }
  }

  TEST_CASE("VariableData(Serialise)", "[CSS][Values][VariableData]")
  {
    for (auto css : {u8"var(--x)", u8"var(--x, 1px) solid", u8"env(safe-area-inset-top)",
                     u8"\"a string\" url(x) #fff", u8""})
    {
      INFO(ToNarrow(css));
      CHECK(ToNarrow(MakeData(css)->Serialise()) == ToNarrow(css));
    }

    // A URL token's value is serialised as an identifier.
    CHECK(ToNarrow(MakeData(u8"url(x.png)")->Serialise()) == "url(x\\.png)");
  }

  TEST_CASE("VariableData(Tokens)", "[CSS][Values][VariableData]")
  {
    // The tokens are copied, so they outlive the tokenizer and the input.
    auto data = MakeData(u8"--name red");
    REQUIRE(data->Tokens().size() == 3uz);
    CHECK(data->Tokens()[0].Type() == TokenType::Ident);
    CHECK(data->Tokens()[0].IdentCodePoints() == u8"--name");
    CHECK(data->Tokens()[1].Type() == TokenType::Whitespace);
    CHECK(data->Tokens()[2].ValueId() == ValueId::Red);

    auto range = data->TokenRange();
    CHECK(range.Tokens().size() == 3uz);
    CHECK(range.Tokens().data() == data->Tokens().data());

    CHECK(MakeData(u8"")->Tokens().empty());
  }

  TEST_CASE("VariableData(Equality)", "[CSS][Values][VariableData]")
  {
    CHECK(*MakeData(u8"var(--x) 1px") == *MakeData(u8"var(--x) 1px"));
    CHECK(*MakeData(u8"var(--x) 1px") != *MakeData(u8"var(--y) 1px"));
    CHECK(*MakeData(u8"var(--x) 1px") != *MakeData(u8"var(--x) 2px"));
    CHECK(*MakeData(u8"var(--x)") != *MakeData(u8"var(--x) "));
    CHECK(*MakeData(u8"") == *MakeData(u8""));
  }

  TEST_CASE("VariableData(Context)", "[CSS][Values][VariableData]")
  {
    CHECK(MakeData(u8"red")->Context().Mode == ParserMode::HTMLStandard);

    ParserContext quirks {ParserMode::HTMLQuirks};
    CHECK(MakeData(u8"red", quirks)->Context().Mode == ParserMode::HTMLQuirks);
  }
}
