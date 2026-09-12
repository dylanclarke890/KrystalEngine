#include "Krystal.Booey/DOM/Algorithms/OrderedSet.hpp"
#include <catch_all.hpp>

namespace krys::tests
{
  using namespace krys::boo;

  TEST_CASE("OrderedSet::Parser", "[HTML][OrderedSet]")
  {
    REQUIRE(OrderedSet::Parser(u8"") == List<dom::DOMString> {});
    REQUIRE(OrderedSet::Parser(u8"   ") == List<dom::DOMString> {});
    REQUIRE(OrderedSet::Parser(u8"foo") == List<dom::DOMString> {u8"foo"});
    REQUIRE(OrderedSet::Parser(u8"foo bar") == List<dom::DOMString> {u8"foo", u8"bar"});
    REQUIRE(OrderedSet::Parser(u8"foo bar foo") == List<dom::DOMString> {u8"foo", u8"bar"});
  }

  TEST_CASE("OrderedSet", "[HTML][OrderedSet]")
  {
    {
      auto input = List<dom::DOMString> {};
      REQUIRE(OrderedSet::Serializer(input) == u8"");
    }
    {
      auto input = List<dom::DOMString> {u8"foo"};
      REQUIRE(OrderedSet::Serializer(input) == u8"foo");
    }
    {
      auto input = List<dom::DOMString> {u8"foo", u8"bar"};
      REQUIRE(OrderedSet::Serializer(input) == u8"foo bar");
    }
  }
}