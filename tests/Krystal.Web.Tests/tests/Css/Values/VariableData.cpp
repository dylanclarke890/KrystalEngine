#include "Krystal.Web/Css/Values/VariableData.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web.Tests/Css/VariableTestUtils.hpp"
#include "Krystal.Web/Css/Parser/InputStream.hpp"
#include "Krystal.Web/Css/Parser/Tokenizer.hpp"

namespace krys::web::css::tests
{
  TEST_CASE("VariableData(Serialise)", "[CSS][Values][VariableData]")
  {
    for (auto css : {u8"var(--x)", u8"var(--x, 1px) solid", u8"env(safe-area-inset-top)",
                     u8"\"a string\" url(x) #fff", u8""})
    {
      INFO(ToNarrow(css));
      CHECK(ToNarrow(MakeVariableData(css)->Serialise()) == ToNarrow(css));
    }

    // A URL token's value is serialised as an identifier.
    CHECK(ToNarrow(MakeVariableData(u8"url(x.png)")->Serialise()) == "url(x\\.png)");
  }

  TEST_CASE("VariableData(Tokens)", "[CSS][Values][VariableData]")
  {
    // The tokens are copied, so they outlive the tokenizer and the input.
    auto data = MakeVariableData(u8"--name red");
    REQUIRE(data->Tokens().size() == 3uz);
    CHECK(data->Tokens()[0].Type() == TokenType::Ident);
    CHECK(data->Tokens()[0].IdentCodePoints() == u8"--name");
    CHECK(data->Tokens()[1].Type() == TokenType::Whitespace);
    CHECK(data->Tokens()[2].ValueId() == ValueId::Red);

    auto range = data->TokenRange();
    CHECK(range.Tokens().size() == 3uz);
    CHECK(range.Tokens().data() == data->Tokens().data());

    CHECK(MakeVariableData(u8"")->Tokens().empty());
  }

  TEST_CASE("VariableData(Equality)", "[CSS][Values][VariableData]")
  {
    CHECK(*MakeVariableData(u8"var(--x) 1px") == *MakeVariableData(u8"var(--x) 1px"));
    CHECK(*MakeVariableData(u8"var(--x) 1px") != *MakeVariableData(u8"var(--y) 1px"));
    CHECK(*MakeVariableData(u8"var(--x) 1px") != *MakeVariableData(u8"var(--x) 2px"));
    CHECK(*MakeVariableData(u8"var(--x)") != *MakeVariableData(u8"var(--x) "));
    CHECK(*MakeVariableData(u8"") == *MakeVariableData(u8""));
  }

  TEST_CASE("VariableData(Context)", "[CSS][Values][VariableData]")
  {
    CHECK(MakeVariableData(u8"red")->Context().Mode == ParserMode::HTMLStandard);

    ParserContext quirks {ParserMode::HTMLQuirks};
    CHECK(MakeVariableData(u8"red", quirks)->Context().Mode == ParserMode::HTMLQuirks);
  }
}
