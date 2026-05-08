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
    REQUIRE(OrderedSet::Serializer(List<DOMString> {}) == u8"");
    REQUIRE(OrderedSet::Serializer(List<DOMString> {u8"foo"}) == u8"foo");
    REQUIRE(OrderedSet::Serializer(List<DOMString> {u8"foo", u8"bar"}) == u8"foo bar");
  }
}