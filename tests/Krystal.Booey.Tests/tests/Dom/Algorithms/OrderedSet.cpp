#include "Krystal.Booey/Dom/Algorithms/OrderedSet.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE("Ordered set parser: splits on ASCII whitespace, keeping the first of each token",
            "[HTML][OrderedSet][dom:concept-ordered-set-parser]")
  {
    REQUIRE(OrderedSet::Parser(u8"").empty());
    REQUIRE(OrderedSet::Parser(u8"   ").empty());
    REQUIRE(OrderedSet::Parser(u8"foo") == List<DOMString> {u8"foo"});
    REQUIRE(OrderedSet::Parser(u8"foo bar") == List<DOMString> {u8"foo", u8"bar"});
    REQUIRE(OrderedSet::Parser(u8"foo bar foo") == List<DOMString> {u8"foo", u8"bar"});

    SECTION("Every kind of ASCII whitespace separates tokens, and leading and trailing runs are dropped")
    {
      REQUIRE(OrderedSet::Parser(u8" \t\n\f\ra \t\n\f\rb \t\n\f\r") == List<DOMString> {u8"a", u8"b"});
    }

    SECTION("Other whitespace is part of a token")
    {
      REQUIRE(OrderedSet::Parser(u8"a\u00A0b\u3000c") == List<DOMString> {u8"a\u00A0b\u3000c"});
      REQUIRE(OrderedSet::Parser(u8"a\vb") == List<DOMString> {u8"a\vb"});
    }

    SECTION("A repeated token stays where it first appeared")
    {
      REQUIRE(OrderedSet::Parser(u8"b a b c a") == List<DOMString> {u8"b", u8"a", u8"c"});
    }

    SECTION("Tokens are compared case-sensitively")
    {
      REQUIRE(OrderedSet::Parser(u8"a A a") == List<DOMString> {u8"a", u8"A"});
    }
  }

  TEST_CASE("Ordered set serializer: the tokens joined with single spaces",
            "[HTML][OrderedSet][dom:concept-ordered-set-serializer]")
  {
    {
      auto input = List<DOMString> {};
      REQUIRE(OrderedSet::Serialiser(input).empty());
    }
    {
      auto input = List<DOMString> {u8"foo"};
      REQUIRE(OrderedSet::Serialiser(input) == u8"foo");
    }
    {
      auto input = List<DOMString> {u8"foo", u8"bar"};
      REQUIRE(OrderedSet::Serialiser(input) == u8"foo bar");
    }
  }
}
