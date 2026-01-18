#include "Krystal.HTML/Parser/HTMLInputStream.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  static utf32_string ToUTF32(const char *s)
  {
    utf32_string out;
    while (*s)
      out.push_back(static_cast<char32>(*s++));
    return out;
  }

  TEST_CASE("HTMLInputStream: basic iteration", "[HTML][InputStream]")
  {
    HTMLInputStream stream;
    stream.Append(ToUTF32("abc"), IsEOF(true));

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'a');

    stream.Advance();
    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'b');

    stream.Advance();
    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'c');

    stream.Advance();
    REQUIRE_FALSE(stream.Peek());
    REQUIRE(stream.IsAtEOF());
  }

  TEST_CASE("HTMLInputStream: CR normalizes to LF", "[HTML][InputStream]")
  {
    HTMLInputStream stream;
    stream.Append(ToUTF32("a\rb"), IsEOF(true));

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'a');
    stream.Advance();

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'\n'); // CR → LF
    stream.Advance();

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'b');
  }

  TEST_CASE("HTMLInputStream: CRLF collapses to single LF", "[HTML][InputStream]")
  {
    HTMLInputStream stream;
    stream.Append(ToUTF32("a\r\nb"), IsEOF(true));

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'a');
    stream.Advance();

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'\n');
    stream.Advance();

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'b');
  }

  TEST_CASE("HTMLInputStream: LF preserved", "[HTML][InputStream]")
  {
    HTMLInputStream stream;
    stream.Append(ToUTF32("a\nb"), IsEOF(true));

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'a');
    stream.Advance();

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'\n');
    stream.Advance();

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'b');
  }

  TEST_CASE("HTMLInputStream: NULL replaced with U+FFFD", "[HTML][InputStream]")
  {
    HTMLInputStream stream;
    utf32_string data = {U'a', U'\0', U'b'};
    stream.Append(std::move(data), IsEOF(true));

    REQUIRE(stream.Peek(IgnoreNullCharacters(false)));
    REQUIRE(stream.NextInputCharacter() == U'a');
    stream.Advance(IgnoreNullCharacters(false));

    REQUIRE(stream.Peek(IgnoreNullCharacters(false)));
    REQUIRE(stream.NextInputCharacter() == U'\uFFFD');
    stream.Advance(IgnoreNullCharacters(false));

    REQUIRE(stream.Peek(IgnoreNullCharacters(false)));
    REQUIRE(stream.NextInputCharacter() == U'b');
  }

  TEST_CASE("HTMLInputStream: NULL ignored when requested", "[HTML][InputStream]")
  {
    HTMLInputStream stream;
    utf32_string data = {U'a', U'\0', U'b'};
    stream.Append(std::move(data), IsEOF(true));

    REQUIRE(stream.Peek(IgnoreNullCharacters(true)));
    REQUIRE(stream.NextInputCharacter() == U'a');
    stream.Advance(IgnoreNullCharacters(true));

    REQUIRE(stream.Peek(IgnoreNullCharacters(true)));
    REQUIRE(stream.NextInputCharacter() == U'b');
  }

  TEST_CASE("HTMLInputStream: incremental append", "[HTML][InputStream]")
  {
    HTMLInputStream stream;
    stream.Append(ToUTF32("ab"));

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'a');
    stream.Advance();

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'b');
    stream.Advance();

    REQUIRE_FALSE(stream.Peek());

    stream.Append(ToUTF32("cd"), IsEOF(true));

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'c');
    stream.Advance();

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'd');
  }

  TEST_CASE("HTMLInputStream: EOF only after final chunk", "[HTML][InputStream]")
  {
    HTMLInputStream stream;
    stream.Append(ToUTF32("a"));

    REQUIRE(stream.Peek());
    stream.Advance();

    REQUIRE_FALSE(stream.IsAtEOF());

    stream.Append({}, IsEOF(true));
    REQUIRE(stream.IsAtEOF());
  }

  TEST_CASE("HTMLInputStream: Peek does not consume", "[HTML][InputStream]")
  {
    HTMLInputStream stream;
    stream.Append(ToUTF32("x"), IsEOF(true));

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'x');

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'x');

    stream.Advance();
    REQUIRE_FALSE(stream.Peek());
  }
}
