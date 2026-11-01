#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::infra::tests
{
  using namespace krys::boo;

  TEST_CASE("Strip leading and trailing ASCII whitespace",
            "[Infra][StringAlgorithms][infra:strip-leading-and-trailing-ascii-whitespace]")
  {
    REQUIRE(StringAlgorithms::StripLeadingAndTrailingWhitespace(u8"   foo bar baz   ") == u8"foo bar baz");
    REQUIRE(StringAlgorithms::StripLeadingAndTrailingWhitespace(u8"foo bar baz") == u8"foo bar baz");
    REQUIRE(StringAlgorithms::StripLeadingAndTrailingWhitespace(u8"   foo bar baz") == u8"foo bar baz");
    REQUIRE(StringAlgorithms::StripLeadingAndTrailingWhitespace(u8"foo bar baz   ") == u8"foo bar baz");
  }

  TEST_CASE("Strip and collapse ASCII whitespace: runs become one space",
            "[Infra][StringAlgorithms][infra:strip-and-collapse-ascii-whitespace]")
  {
    // Leading and trailing whitespace is stripped
    REQUIRE(StringAlgorithms::StripAndCollapseASCIIWhitespace(
              u8"\t\n  Hello   \t  beautiful   world\n\r  from \t C++   \n")
            == u8"Hello beautiful world from C++");
    // No whitespace — unchanged
    REQUIRE(StringAlgorithms::StripAndCollapseASCIIWhitespace(u8"foo") == u8"foo");
    // Already single spaces — unchanged
    REQUIRE(StringAlgorithms::StripAndCollapseASCIIWhitespace(u8"foo bar baz") == u8"foo bar baz");
    // Multiple spaces between words collapsed to one
    REQUIRE(StringAlgorithms::StripAndCollapseASCIIWhitespace(u8"foo   bar   baz") == u8"foo bar baz");
    // Only whitespace — collapses to empty string
    REQUIRE(StringAlgorithms::StripAndCollapseASCIIWhitespace(u8"   ") == u8"");
    // Empty string — unchanged
    REQUIRE(StringAlgorithms::StripAndCollapseASCIIWhitespace(u8"") == u8"");
    // Mixed whitespace characters collapsed
    REQUIRE(StringAlgorithms::StripAndCollapseASCIIWhitespace(u8"\t\n\r\f") == u8"");
    // Leading whitespace stripped
    REQUIRE(StringAlgorithms::StripAndCollapseASCIIWhitespace(u8"   foo") == u8"foo");
    // Trailing whitespace stripped
    REQUIRE(StringAlgorithms::StripAndCollapseASCIIWhitespace(u8"foo   ") == u8"foo");
    // Internal mixed whitespace collapsed to single space
    REQUIRE(StringAlgorithms::StripAndCollapseASCIIWhitespace(u8"foo\t\n\r bar") == u8"foo bar");
    // Single word with surrounding mixed whitespace
    REQUIRE(StringAlgorithms::StripAndCollapseASCIIWhitespace(u8"\t foo \n") == u8"foo");
  }

  TEST_CASE(
    "Collect a sequence of code points: while the condition holds",
    "[Infra][StringAlgorithms][infra:collect-a-sequence-of-code-points][infra:string-position-variable]")
  {
    auto condition = [](char32 current)
    {
      return current != u8' ';
    };

    dom::DOMString input = u8"foo bar baz";
    dom::DOMStringView inputView(input);

    auto position = inputView.begin();

    REQUIRE(StringAlgorithms::CollectCodePointSequence(inputView, position, condition) == u8"foo");
    ++position; // skip the space

    REQUIRE(StringAlgorithms::CollectCodePointSequence(inputView, position, condition) == u8"bar");
    ++position; // skip the space

    REQUIRE(StringAlgorithms::CollectCodePointSequence(inputView, position, condition) == u8"baz");
    REQUIRE(position == inputView.end());
  }

  TEST_CASE("ASCII whitespace: tab, LF, FF, CR and space",
            "[Infra][StringAlgorithms][infra:ascii-whitespace]")
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

  TEST_CASE("Strictly split a string: every delimiter separates two tokens",
            "[Infra][StringAlgorithms][infra:strictly-split]")
  {
    REQUIRE(StringAlgorithms::StrictlySplit(u8"foo bar baz", u8' ')
            == List<dom::DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::StrictlySplit(u8"foo:bar", u8':') == List<dom::DOMString> {u8"foo", u8"bar"});
    REQUIRE(StringAlgorithms::StrictlySplit(u8"foo:", u8':') == List<dom::DOMString> {u8"foo", u8""});
  }

  TEST_CASE("Skip ASCII whitespace", "[Infra][StringAlgorithms][infra:skip-ascii-whitespace]")
  {
    dom::DOMString input = u8"   foo bar baz";
    dom::DOMStringView inputView(input);

    auto position = inputView.begin();
    StringAlgorithms::SkipWhitespace(inputView, position);
    REQUIRE(position == inputView.begin() + 3);
  }

  TEST_CASE("Split a string on ASCII whitespace: no empty tokens",
            "[Infra][StringAlgorithms][infra:split-on-ascii-whitespace]")
  {
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"   foo   bar baz  ")
            == List<dom::DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"foo bar baz")
            == List<dom::DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"   foo bar baz")
            == List<dom::DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"foo bar baz   ")
            == List<dom::DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"      ") == List<dom::DOMString> {});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"") == List<dom::DOMString> {});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t\n\r\f") == List<dom::DOMString> {});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t foo \n bar \r baz \f")
            == List<dom::DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t foo \n  \r  baz \f")
            == List<dom::DOMString> {u8"foo", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t foo \n  \r    \f") == List<dom::DOMString> {u8"foo"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t    \n  \r    \f") == List<dom::DOMString> {});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t foo") == List<dom::DOMString> {u8"foo"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t foo ") == List<dom::DOMString> {u8"foo"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t foo  ") == List<dom::DOMString> {u8"foo"});
    REQUIRE(StringAlgorithms::SplitOnWhitespace(u8"\t foo  bar") == List<dom::DOMString> {u8"foo", u8"bar"});
  }

  TEST_CASE("Split a string on commas: each token stripped of ASCII whitespace",
            "[Infra][StringAlgorithms][infra:split-on-commas]")
  {
    REQUIRE(StringAlgorithms::SplitOnComma(u8"   foo,   bar baz,qux  ")
            == List<dom::DOMString> {u8"foo", u8"bar baz", u8"qux"});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"foo,bar,baz")
            == List<dom::DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"   foo,bar,baz")
            == List<dom::DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"foo,bar,baz   ")
            == List<dom::DOMString> {u8"foo", u8"bar", u8"baz"});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"      ") == List<dom::DOMString> {u8""});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"") == List<dom::DOMString> {});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"\t\n\r\f") == List<dom::DOMString> {u8""});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"\t foo \n bar \r baz \f")
            == List<dom::DOMString> {u8"foo \n bar \r baz"});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"\t foo ,\n  bar \r  baz \f")
            == List<dom::DOMString> {u8"foo", u8"bar \r  baz"});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"\t foo ,\n  bar \r    \f")
            == List<dom::DOMString> {u8"foo", u8"bar"});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"\t foo ,\n    \r    \f")
            == List<dom::DOMString> {u8"foo", u8""});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"\t foo,") == List<dom::DOMString> {u8"foo"});
    REQUIRE(StringAlgorithms::SplitOnComma(u8"\t foo, ") == List<dom::DOMString> {u8"foo", u8""});
  }

  TEST_CASE("Concatenate: the strings separated by the separator",
            "[Infra][StringAlgorithms][infra:string-concatenate]")
  {
    {
      auto input = List<dom::DOMString> {u8"foo", u8"bar", u8"baz"};
      REQUIRE(StringAlgorithms::Concatenate(input, std::nullopt) == u8"foobarbaz");
    }
    {
      auto input = List<dom::DOMString> {};
      REQUIRE(StringAlgorithms::Concatenate(input, std::nullopt) == u8"");
    }
    {
      auto input = List<dom::DOMString> {u8""};
      REQUIRE(StringAlgorithms::Concatenate(input, std::nullopt) == u8"");
    }
    {
      auto input = List<dom::DOMString> {u8"foo", u8"", u8"bar"};
      REQUIRE(StringAlgorithms::Concatenate(input, std::nullopt) == u8"foobar");
    }
    {
      auto input = List<dom::DOMString> {u8"foo", u8"bar", u8"baz"};
      REQUIRE(StringAlgorithms::Concatenate(input, u8", ") == u8"foo, bar, baz");
    }
  }

  TEST_CASE("Code unit less than: by UTF-16 code units, where a surrogate pair sorts before U+E000 to U+FFFF",
            "[Infra][StringAlgorithms][infra:code-unit-less-than]")
  {
    CHECK(StringAlgorithms::CodeUnitLessThan(u8"a", u8"b"));
    CHECK(StringAlgorithms::CodeUnitLessThan(u8"a", u8"ab"));
    CHECK_FALSE(StringAlgorithms::CodeUnitLessThan(u8"ab", u8"a"));
    CHECK_FALSE(StringAlgorithms::CodeUnitLessThan(u8"ab", u8"ab"));
    // U+1F600 is a surrogate pair in UTF-16, which sorts before U+FF21 and U+E000 but after U+D7FF.
    CHECK(StringAlgorithms::CodeUnitLessThan(u8"\U0001F600", u8"Ａ"));
    CHECK_FALSE(StringAlgorithms::CodeUnitLessThan(u8"Ａ", u8"\U0001F600"));
    CHECK(StringAlgorithms::CodeUnitLessThan(u8"x\U0001F600", u8"x"));
    CHECK(StringAlgorithms::CodeUnitLessThan(u8"퟿", u8"\U0001F600"));
    // Within the same range, by code point.
    CHECK(StringAlgorithms::CodeUnitLessThan(u8"\U0001F600", u8"\U0001F601"));
    CHECK(StringAlgorithms::CodeUnitLessThan(u8"é", u8"ê"));
  }

  TEST_CASE("Normalize newlines: CR LF, then any other CR, becomes LF",
            "[Infra][StringAlgorithms][infra:normalize-newlines]")
  {
    CHECK(StringAlgorithms::NormalizeNewlines(u8"a\r\nb\rc\nd\r\r\n") == u8"a\nb\nc\nd\n\n");
    CHECK(StringAlgorithms::NormalizeNewlines(u8"\n\r") == u8"\n\n");
    CHECK(StringAlgorithms::NormalizeNewlines(u8"") == u8"");
  }

  TEST_CASE("ASCII case-insensitive: only ASCII upper and lower alphas match each other",
            "[Infra][StringAlgorithms][infra:ascii-case-insensitive]")
  {
    CHECK(StringAlgorithms::ASCIICaseInsensitiveMatch(u8"HeLLo", u8"hEllO"));
    CHECK_FALSE(StringAlgorithms::ASCIICaseInsensitiveMatch(u8"hello", u8"hell"));
    // Code points 0x20 apart that aren't letters.
    CHECK_FALSE(StringAlgorithms::ASCIICaseInsensitiveMatch(u8"@", u8"`"));
    CHECK_FALSE(StringAlgorithms::ASCIICaseInsensitiveMatch(u8"[", u8"{"));
    CHECK_FALSE(StringAlgorithms::ASCIICaseInsensitiveMatch(u8"\x10", u8"0"));
    // Non-ASCII code points, even with their own case, must be identical.
    CHECK_FALSE(StringAlgorithms::ASCIICaseInsensitiveMatch(u8"é", u8"É"));
    CHECK(StringAlgorithms::ASCIICaseInsensitiveMatch(u8"é", u8"é"));
  }

  TEST_CASE("ASCII string: only ASCII code points", "[Infra][StringAlgorithms][infra:ascii-string]")
  {
    CHECK(StringAlgorithms::IsASCIIString(u8"abc\x7F"));
    CHECK(StringAlgorithms::IsASCIIString(u8""));
    CHECK_FALSE(StringAlgorithms::IsASCIIString(u8"abé"));
  }

  TEST_CASE("ASCII lowercase and uppercase: only ASCII alphas change",
            "[Infra][StringAlgorithms][infra:ascii-lowercase][infra:ascii-uppercase]")
  {
    CHECK(krys::text::ToASCIILower(dom::DOMStringView(u8"AbC@[É")) == u8"abc@[É");
    CHECK(krys::text::ToASCIIUpper(dom::DOMStringView(u8"aBc`{é")) == u8"ABC`{é");
  }

  TEST_CASE("Code points: the ASCII classes",
            "[Infra][StringAlgorithms][infra:code-points][infra:ascii-code-point][infra:ascii-digit]"
            "[infra:ascii-hex-digit][infra:ascii-upper-alpha][infra:ascii-lower-alpha][infra:ascii-alpha]"
            "[infra:ascii-alphanumeric]")
  {
    for (char32 c = 0; c < 0x100; ++c)
    {
      bool upper = c >= U'A' && c <= U'Z';
      bool lower = c >= U'a' && c <= U'z';
      bool digit = c >= U'0' && c <= U'9';
      CHECK(krys::text::IsASCII(c) == (c <= 0x7F));
      CHECK(krys::text::IsASCIIUpper(c) == upper);
      CHECK(krys::text::IsASCIILower(c) == lower);
      CHECK(krys::text::IsASCIIAlpha(c) == (upper || lower));
      CHECK(krys::text::IsASCIIDigit(c) == digit);
      CHECK(krys::text::IsASCIIAlphanumeric(c) == (upper || lower || digit));
      CHECK(krys::text::IsASCIIHexDigit(c)
            == (digit || (c >= U'A' && c <= U'F') || (c >= U'a' && c <= U'f')));
    }
  }

  TEST_CASE("Namespaces",
            "[Infra][Namespaces][infra:namespaces][infra:html-namespace][infra:mathml-namespace]"
            "[infra:svg-namespace][infra:xlink-namespace][infra:xml-namespace][infra:xmlns-namespace]")
  {
    STATIC_CHECK(Namespaces::HTML == u8"http://www.w3.org/1999/xhtml");
    STATIC_CHECK(Namespaces::MathML == u8"http://www.w3.org/1998/Math/MathML");
    STATIC_CHECK(Namespaces::SVG == u8"http://www.w3.org/2000/svg");
    STATIC_CHECK(Namespaces::XLink == u8"http://www.w3.org/1999/xlink");
    STATIC_CHECK(Namespaces::XML == u8"http://www.w3.org/XML/1998/namespace");
    STATIC_CHECK(Namespaces::XMLNS == u8"http://www.w3.org/2000/xmlns/");
  }
}