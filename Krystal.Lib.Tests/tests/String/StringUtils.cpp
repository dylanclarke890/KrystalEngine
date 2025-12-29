#include "Krystal.Lib/String/StringUtils.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("StringUtils::Split", "[StringUtils]")
  {
    const string TestString = "This is a test string.";
    const List<string> Expected({"This", "is", "a", "test", "string."});
    const List<string> Actual = StringUtils::Split(TestString, " ");

    REQUIRE(Expected.size() == Actual.size());
    for (auto i = 0; i < Expected.size(); i++)
    {
      REQUIRE(Expected[i] == Actual[i]);
    }
  }
}