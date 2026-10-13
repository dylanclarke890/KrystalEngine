#include "Krystal.Booey/CSS/Properties/Consumers/Syntax.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumeDeclarationValue)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"foo bar", ConsumeDeclarationValue) == "foo bar");
    CHECK(ConsumeToText(u8"var(--x)", ConsumeDeclarationValue) == "var(--x)");

    // Bad tokens and unmatched closing brackets aren't allowed in <declaration-value>.
    CHECK(ConsumeToText(u8"foo )", ConsumeDeclarationValue) == "<null>");
  }
}
