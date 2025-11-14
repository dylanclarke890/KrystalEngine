#include "Krystal.Lib/String/StringInterner.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Catch::Matchers;

  TEST_CASE("StringInterner::Intern", "[StringInterner]")
  {
    Krys::StringInterner interner;

    StringRef helloId = interner.Intern("hello");
    StringRef againId = interner.Intern("hello"); // same ID
    StringRef worldId = interner.Intern("world");

    REQUIRE(helloId == againId);
    REQUIRE(helloId != worldId);
    REQUIRE(interner.Get(worldId) == u8"world");
  }
}