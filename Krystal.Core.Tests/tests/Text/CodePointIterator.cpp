#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::text::tests
{
  namespace
  {
    List<char32> Decode(utf8_stringview text)
    {
      List<char32> result;
      for (CodePointIterator it {text}; !it.AtEnd(); ++it)
      {
        result.push_back(*it);
      }
      return result;
    }

    utf8_string Bytes(std::initializer_list<unsigned char> bytes)
    {
      utf8_string result;
      for (auto b : bytes)
      {
        result.push_back(static_cast<char8>(b));
      }
      return result;
    }
  }

  TEST_CASE("CodePointIterator(valid)", "[Text]")
  {
    CHECK(Decode(u8"").empty());
    CHECK(Decode(u8"ab") == List<char32> {U'a', U'b'});
    CHECK(Decode(u8"é€\U0001F600") == List<char32> {0xE9, 0x20AC, 0x1F600});
  }

  TEST_CASE("CodePointIterator(invalid)", "[Text]")
  {
    // A lone continuation byte, an overlong '/', a surrogate, a truncated sequence and a byte above F4.
    CHECK(Decode(Bytes({0x80, 'a'})) == List<char32> {UnicodeReplacementChar, U'a'});
    CHECK(Decode(Bytes({0xC0, 0xAF})) == List<char32> {UnicodeReplacementChar, UnicodeReplacementChar});
    CHECK(Decode(Bytes({0xED, 0xA0, 0x80})).size() == 3);
    CHECK(Decode(Bytes({0xE2, 0x82})) == List<char32> {UnicodeReplacementChar, UnicodeReplacementChar});
    CHECK(Decode(Bytes({0xF5, 'a'})) == List<char32> {UnicodeReplacementChar, U'a'});
  }

  TEST_CASE("CodePointIterator(ranges and positions)", "[Text]")
  {
    utf8_stringview text = u8"aéb";
    CodePointIterator begin {text};
    auto it = begin;
    ++it;
    CHECK(it.CodeUnitsSince(begin) == 1);
    ++it;
    CHECK(it.CodeUnitsSince(begin) == 3);
    CHECK(*it == U'b');

    CodePointIterator middle {begin, it};
    CHECK(*middle == U'a');
    ++middle;
    CHECK(*middle == 0xE9);
    ++middle;
    CHECK(middle.AtEnd());
    CHECK(CodePointIterator {} == CodePointIterator {});
  }

  TEST_CASE("IsValidUTF8", "[Text]")
  {
    CHECK(IsValidUTF8(u8"plain"));
    CHECK(IsValidUTF8(u8"é\U0001F600"));
    CHECK_FALSE(IsValidUTF8(Bytes({0xC3})));
    CHECK_FALSE(IsValidUTF8(Bytes({0xED, 0xBF, 0xBF})));
    CHECK_FALSE(IsValidUTF8(Bytes({0xF4, 0x90, 0x80, 0x80})));
  }
}
