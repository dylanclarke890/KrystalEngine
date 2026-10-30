#include "Krystal.Booey/DOM/FragmentDirectiveParser.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"

namespace krys::boo::dom::tests
{
  using boo::tests::Str;

  namespace
  {
    KRYS_NODISCARD List<ParsedTextDirective> Parse(utf8_stringview directive) noexcept
    {
      FragmentDirectiveParser parser(directive);
      REQUIRE(parser.IsValid());
      return parser.ParsedTextDirectives();
    }
  }

  TEST_CASE("FragmentDirectiveParser", "[DOM][FragmentDirective]")
  {
    SECTION("start text, and a range from start to end")
    {
      auto directives = Parse(u8"text=hello");
      REQUIRE(directives.size() == 1);
      CHECK(Str(directives[0].StartText) == "hello");
      CHECK(directives[0].EndText.empty());

      directives = Parse(u8"text=from,to");
      REQUIRE(directives.size() == 1);
      CHECK(Str(directives[0].StartText) == "from");
      CHECK(Str(directives[0].EndText) == "to");
    }

    SECTION("a prefix ends with '-' and a suffix starts with one")
    {
      auto directives = Parse(u8"text=before-,match,-after");
      REQUIRE(directives.size() == 1);
      CHECK(Str(directives[0].Prefix) == "before");
      CHECK(Str(directives[0].StartText) == "match");
      CHECK(Str(directives[0].Suffix) == "after");

      directives = Parse(u8"text=a-,from,to,-z");
      REQUIRE(directives.size() == 1);
      CHECK(Str(directives[0].Prefix) == "a");
      CHECK(Str(directives[0].StartText) == "from");
      CHECK(Str(directives[0].EndText) == "to");
      CHECK(Str(directives[0].Suffix) == "z");
    }

    SECTION("the parts are percent-decoded")
    {
      auto directives = Parse(u8"text=caf%C3%A9%20au%20lait,%2C");
      REQUIRE(directives.size() == 1);
      CHECK(Str(directives[0].StartText) == "café au lait");
      CHECK(Str(directives[0].EndText) == ",");
    }

    SECTION("several directives, and the rest dropped")
    {
      auto directives = Parse(u8"text=one&other=thing&text=two");
      REQUIRE(directives.size() == 2);
      CHECK(Str(directives[0].StartText) == "one");
      CHECK(Str(directives[1].StartText) == "two");
    }

    SECTION("malformed directives are dropped")
    {
      CHECK(Parse(u8"text=").empty());
      CHECK(Parse(u8"text=a,,b").empty());
      CHECK(Parse(u8"text=a,b,c,d,e").empty());
      CHECK(Parse(u8"text=pre-").empty());
      CHECK(Parse(u8"text=a,b,c").empty());
      CHECK(Parse(u8"selector=x").empty());
    }
  }
}
