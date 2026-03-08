#include "Krystal.HTML/Utils/StringAlgorithms.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("StringAlgorithms::StrictlySplit", "[HTML][StringAlgorithms]")
  {
    REQUIRE(StringAlgorithms::StrictlySplit(u8"foo bar baz", u8' ')
            == List<DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::StrictlySplit(u8"foo:bar", u8':') == List<DOMString> {u8"foo", u8"bar"});
    REQUIRE(StringAlgorithms::StrictlySplit(u8"foo:", u8':') == List<DOMString> {u8"foo", u8""});
  }

  TEST_CASE("StringAlgorithms::CollectCodePointSequence", "[HTML][StringAlgorithms]")
  {
    auto condition = [](char32 current)
    {
      return current != u8' ';
    };

    DOMString input = u8"foo bar baz";
    auto position = input.begin();

    REQUIRE(StringAlgorithms::CollectCodePointSequence(input, position, condition) == u8"foo");
    ++position; // skip the space

    REQUIRE(StringAlgorithms::CollectCodePointSequence(input, position, condition) == u8"bar");
    ++position; // skip the space

    REQUIRE(StringAlgorithms::CollectCodePointSequence(input, position, condition) == u8"baz");
    REQUIRE(position == input.end());
  }
}