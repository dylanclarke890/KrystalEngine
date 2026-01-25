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
    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == HTMLInputStream::EOFMarker);
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
    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == HTMLInputStream::EOFMarker);
  }

  TEST_CASE("HTMLInputStream: Advance past advances the correct amount of characters", "[HTML][InputStream]")
  {
    HTMLInputStream stream;
    stream.Append(ToUTF32("aBcDeF"), IsEOF(true));

    auto result = stream.AdvancePast<false>(Text::ASCIILiteral::From("aBc"));
    REQUIRE(result == HTMLInputStream::MatchResult::Matched);
    REQUIRE(stream.NextInputCharacter() == U'D');

    result = stream.AdvancePast<>(Text::ASCIILiteral::From("dE"));
    REQUIRE(result == HTMLInputStream::MatchResult::Matched);
    REQUIRE(stream.NextInputCharacter() == U'F');

    result = stream.AdvancePast<>(Text::ASCIILiteral::From("XYZ"));
    REQUIRE(result == HTMLInputStream::MatchResult::NotEnoughCharacters);
    REQUIRE(stream.NextInputCharacter() == U'F');

    result = stream.AdvancePast<>(Text::ASCIILiteral::From("G"));
    REQUIRE(result == HTMLInputStream::MatchResult::DidNotMatch);
    REQUIRE(stream.NextInputCharacter() == U'F');
  }

  TEST_CASE("HTMLInputStream: SourceLocation tracking", "[HTML][InputStream]")
  {
    HTMLInputStream stream;

    stream.Append(ToUTF32("a\nb\rc\r\ndtext"), IsEOF(true));
    {
      const auto &loc = stream.GetCurrentLocation();
      REQUIRE(loc.Line == 1uz);
      REQUIRE(loc.Column == 1uz);
    }

    stream.Peek();
    stream.Advance(); // 'a'
    {
      const auto &loc = stream.GetCurrentLocation();
      REQUIRE(loc.Line == 1uz);
      REQUIRE(loc.Column == 2uz);
    }

    stream.Peek();
    stream.Advance(); // '\n'
    {
      const auto &loc = stream.GetCurrentLocation();
      REQUIRE(loc.Line == 2uz);
      REQUIRE(loc.Column == 1uz);
    }

    stream.Peek();
    stream.Advance(); // 'b'
    {
      const auto &loc = stream.GetCurrentLocation();
      REQUIRE(loc.Line == 2uz);
      REQUIRE(loc.Column == 2uz);
    }

    stream.Peek();
    stream.Advance(); // '\n' (from '\r')
    {
      const auto &loc = stream.GetCurrentLocation();
      REQUIRE(loc.Line == 3uz);
      REQUIRE(loc.Column == 1uz);
    }

    stream.Peek();
    stream.Advance(); // 'c'
    {
      const auto &loc = stream.GetCurrentLocation();
      REQUIRE(loc.Line == 3uz);
      REQUIRE(loc.Column == 2uz);
    }

    stream.Peek();
    stream.Advance(); // '\n' (from '\r\n')
    {
      const auto &loc = stream.GetCurrentLocation();
      REQUIRE(loc.Line == 4uz);
      REQUIRE(loc.Column == 1uz);
    }

    stream.Peek();
    stream.Advance(); // 'd'
    {
      const auto &loc = stream.GetCurrentLocation();
      REQUIRE(loc.Line == 4uz);
      REQUIRE(loc.Column == 2uz);
    }

    stream.Peek();
    (void)stream.AdvancePast<false>(Text::ASCIILiteral::From("text")); // 'text'
    {
      const auto &loc = stream.GetCurrentLocation();
      REQUIRE(loc.Line == 4uz);
      REQUIRE(loc.Column == 6uz);
    }
  }
}
