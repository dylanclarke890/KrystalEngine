#include "Krystal.HTML/Algorithms/OrderedSet.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("OrderedSet::Parser", "[HTML][OrderedSet]")
  {
    REQUIRE(OrderedSet::Parser(u8"") == List<DOMString> {});
    REQUIRE(OrderedSet::Parser(u8"   ") == List<DOMString> {});
    REQUIRE(OrderedSet::Parser(u8"foo") == List<DOMString> {u8"foo"});
    REQUIRE(OrderedSet::Parser(u8"foo bar") == List<DOMString> {u8"foo", u8"bar"});
    REQUIRE(OrderedSet::Parser(u8"foo bar foo") == List<DOMString> {u8"foo", u8"bar"});
  }

  TEST_CASE("OrderedSet", "[HTML][OrderedSet]")
  {
    {
      auto input = List<DOMString> {};
      REQUIRE(OrderedSet::Serializer(input) == u8"");
    }
    {
      auto input = List<DOMString> {u8"foo"};
      REQUIRE(OrderedSet::Serializer(input) == u8"foo");
    }
    {
      auto input = List<DOMString> {u8"foo", u8"bar"};
      REQUIRE(OrderedSet::Serializer(input) == u8"foo bar");
    }
  }
}