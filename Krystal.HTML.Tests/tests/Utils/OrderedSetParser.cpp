#include "Krystal.HTML/Utils/OrderedSetParser.hpp"
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
}