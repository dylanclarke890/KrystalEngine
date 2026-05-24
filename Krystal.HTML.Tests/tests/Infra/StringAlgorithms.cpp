#include "Krystal.HTML/Infra/StringAlgorithms.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("StringAlgorithms::StripLeadingAndTrailingWhitespace", "[HTML][StringAlgorithms]")
  {
    REQUIRE(StringAlgorithms::StripLeadingAndTrailingWhitespace(u8"   foo bar baz   ") == u8"foo bar baz");
    REQUIRE(StringAlgorithms::StripLeadingAndTrailingWhitespace(u8"foo bar baz") == u8"foo bar baz");
    REQUIRE(StringAlgorithms::StripLeadingAndTrailingWhitespace(u8"   foo bar baz") == u8"foo bar baz");
    REQUIRE(StringAlgorithms::StripLeadingAndTrailingWhitespace(u8"foo bar baz   ") == u8"foo bar baz");
  }

  TEST_CASE("StringAlgorithms::CollectCodePointSequence", "[HTML][StringAlgorithms]")
  {
    auto condition = [](char32 current)
    {
      return current != u8' ';
    };

    DOMString input = u8"foo bar baz";
    DOMStringView inputView(input);

    auto position = inputView.begin();

    REQUIRE(StringAlgorithms::CollectCodePointSequence(inputView, position, condition) == u8"foo");
    ++position; // skip the space

    REQUIRE(StringAlgorithms::CollectCodePointSequence(inputView, position, condition) == u8"bar");
    ++position; // skip the space

    REQUIRE(StringAlgorithms::CollectCodePointSequence(inputView, position, condition) == u8"baz");
    REQUIRE(position == inputView.end());
  }

  TEST_CASE("StringAlgorithms::IsASCIIWhitespace", "[HTML][StringAlgorithms]")
  {
    REQUIRE(StringAlgorithms::IsASCIIWhitespace(u8' '));
    REQUIRE(StringAlgorithms::IsASCIIWhitespace(u8'\t'));
    REQUIRE(StringAlgorithms::IsASCIIWhitespace(u8'\n'));
    REQUIRE(StringAlgorithms::IsASCIIWhitespace(u8'\r'));
    REQUIRE(StringAlgorithms::IsASCIIWhitespace(u8'\f'));
    REQUIRE_FALSE(StringAlgorithms::IsASCIIWhitespace(u8'a'));
    REQUIRE_FALSE(StringAlgorithms::IsASCIIWhitespace(u8'1'));
    REQUIRE_FALSE(StringAlgorithms::IsASCIIWhitespace(u8'-'));
  }

  TEST_CASE("StringAlgorithms::StrictlySplit", "[HTML][StringAlgorithms]")
  {
    REQUIRE(StringAlgorithms::StrictlySplit(u8"foo bar baz", u8' ')
            == List<DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::StrictlySplit(u8"foo:bar", u8':') == List<DOMString> {u8"foo", u8"bar"});
    REQUIRE(StringAlgorithms::StrictlySplit(u8"foo:", u8':') == List<DOMString> {u8"foo", u8""});
  }

  TEST_CASE("StringAlgorithms::SkipWhitespace", "[HTML][StringAlgorithms]")
  {
    DOMString input = u8"   foo bar baz";
    DOMStringView inputView(input);

    auto position = inputView.begin();
    StringAlgorithms::SkipWhitespace(inputView, position);
    REQUIRE(position == inputView.begin() + 3);
  }

  TEST_CASE("StringAlgorithms::SplitOnWhitespace", "[HTML][StringAlgorithms]")
  {
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"   foo   bar baz  ")
            == List<DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"foo bar baz")
            == List<DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"   foo bar baz")
            == List<DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"foo bar baz   ")
            == List<DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"      ") == List<DOMString> {});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"") == List<DOMString> {});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t\n\r\f") == List<DOMString> {});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t foo \n bar \r baz \f")
            == List<DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t foo \n  \r  baz \f")
            == List<DOMString> {u8"foo", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t foo \n  \r    \f") == List<DOMString> {u8"foo"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t    \n  \r    \f") == List<DOMString> {});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t foo") == List<DOMString> {u8"foo"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t foo ") == List<DOMString> {u8"foo"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t foo  ") == List<DOMString> {u8"foo"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t foo  bar") == List<DOMString> {u8"foo", u8"bar"});
  }

  TEST_CASE("StringAlgorithms::SplitOnComma", "[HTML][StringAlgorithms]")
  {
    REQUIRE(StringAlgorithms::SplitOnComma(u8"   foo,   bar baz,qux  ")
            == List<DOMString> {u8"foo", u8"bar baz", u8"qux"});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"foo,bar,baz") == List<DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"   foo,bar,baz")
            == List<DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"foo,bar,baz   ")
            == List<DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"      ") == List<DOMString> {u8""});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"") == List<DOMString> {});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"\t\n\r\f") == List<DOMString> {u8""});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"\t foo \n bar \r baz \f")
            == List<DOMString> {u8"foo \n bar \r baz"});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"\t foo ,\n  bar \r  baz \f")
            == List<DOMString> {u8"foo", u8"bar \r  baz"});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"\t foo ,\n  bar \r    \f")
            == List<DOMString> {u8"foo", u8"bar"});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"\t foo ,\n    \r    \f") == List<DOMString> {u8"foo", u8""});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"\t foo,") == List<DOMString> {u8"foo"});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"\t foo, ") == List<DOMString> {u8"foo", u8""});
  }

  TEST_CASE("StringAlgorithms::Concatenate", "[HTML][StringAlgorithms]")
  {
    {
      auto input = List<DOMString> {u8"foo", u8"bar", u8"baz"};
      REQUIRE(StringAlgorithms::Concatenate(input, std::nullopt) == u8"foobarbaz");
    }
    {
      auto input = List<DOMString> {};
      REQUIRE(StringAlgorithms::Concatenate(input, std::nullopt) == u8"");
    }
    {
      auto input = List<DOMString> {u8""};
      REQUIRE(StringAlgorithms::Concatenate(input, std::nullopt) == u8"");
    }
    {
      auto input = List<DOMString> {u8"foo", u8"", u8"bar"};
      REQUIRE(StringAlgorithms::Concatenate(input, std::nullopt) == u8"foobar");
    }
    {
      auto input = List<DOMString> {u8"foo", u8"bar", u8"baz"};
      REQUIRE(StringAlgorithms::Concatenate(input, u8", ") == u8"foo, bar, baz");
    }
  }
}