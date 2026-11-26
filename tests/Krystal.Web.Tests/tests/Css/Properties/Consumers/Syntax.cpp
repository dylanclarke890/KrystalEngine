#include "Krystal.Web/Css/Properties/Consumers/Syntax.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/ConsumerTestUtils.hpp"

namespace krys::web::css::parse_property::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("parse_property(ConsumeDeclarationValue)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"foo bar", ConsumeDeclarationValue) == "foo bar");
    CHECK(ConsumeToText(u8"var(--x)", ConsumeDeclarationValue) == "var(--x)");

    // Bad tokens and unmatched closing brackets aren't allowed in <declaration-value>.
    CHECK(ConsumeToText(u8"foo )", ConsumeDeclarationValue) == "<null>");
  }
}
