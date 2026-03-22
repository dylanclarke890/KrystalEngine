#include "Krystal.HTML/Algorithms/OrderedSetAlgorithms.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("OrderedSetParser", "[HTML][OrderedSetParser]")
  {
    REQUIRE(OrderedSetParser(u8"") == List<DOMString> {});
    REQUIRE(OrderedSetParser(u8"   ") == List<DOMString> {});
    REQUIRE(OrderedSetParser(u8"foo") == List<DOMString> {u8"foo"});
    REQUIRE(OrderedSetParser(u8"foo bar") == List<DOMString> {u8"foo", u8"bar"});
    REQUIRE(OrderedSetParser(u8"foo bar foo") == List<DOMString> {u8"foo", u8"bar"});
  }

  TEST_CASE("OrderedSetSerializer", "[HTML][OrderedSetSerializer]")
  {
    REQUIRE(OrderedSetSerializer(List<DOMString> {}) == u8"");
    REQUIRE(OrderedSetSerializer(List<DOMString> {u8"foo"}) == u8"foo");
    REQUIRE(OrderedSetSerializer(List<DOMString> {u8"foo", u8"bar"}) == u8"foo bar");
  }
}