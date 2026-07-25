#include "Krystal.HTML/HTML/Parser/HTMLInputStream.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("HTMLInputStream: basic iteration", "[HTML][InputStream]")
  {
    HTMLInputStream stream;
    stream.Append(u8"abc", IsEOF(true));

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
    REQUIRE(stream.NextInputCharacter() == EOFMarker);
  }

  TEST_CASE("HTMLInputStream: CR normalizes to LF", "[HTML][InputStream]")
  {
    HTMLInputStream stream;
    stream.Append(u8"a\rb", IsEOF(true));

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
    stream.Append(u8"a\r\nb", IsEOF(true));

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
    stream.Append(u8"a\nb", IsEOF(true));

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
    stream.Append(u8"ab");

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'a');
    stream.Advance();

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'b');
    stream.Advance();

    REQUIRE_FALSE(stream.Peek());

    stream.Append(u8"cd", IsEOF(true));

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'c');
    stream.Advance();

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'd');
  }

  TEST_CASE("HTMLInputStream: Peek does not consume", "[HTML][InputStream]")
  {
    HTMLInputStream stream;
    stream.Append(u8"x", IsEOF(true));

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'x');

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'x');

    stream.Advance();
    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == EOFMarker);
  }

  TEST_CASE("HTMLInputStream: Advance past advances the correct amount of characters", "[HTML][InputStream]")
  {
    HTMLInputStream stream;
    stream.Append(u8"aBcDeF", IsEOF(true));

    auto result = stream.AdvancePast<false>(Krys::Text::ASCIILiteral::From("aBc"));
    REQUIRE(result == HTMLInputStream::MatchResult::Matched);
    REQUIRE(stream.NextInputCharacter() == U'D');

    result = stream.AdvancePast<>(Krys::Text::ASCIILiteral::From("dE"));
    REQUIRE(result == HTMLInputStream::MatchResult::Matched);
    REQUIRE(stream.NextInputCharacter() == U'F');

    result = stream.AdvancePast<>(Krys::Text::ASCIILiteral::From("XYZ"));
    REQUIRE(result == HTMLInputStream::MatchResult::NotEnoughCharacters);
    REQUIRE(stream.NextInputCharacter() == U'F');

    result = stream.AdvancePast<>(Krys::Text::ASCIILiteral::From("G"));
    REQUIRE(result == HTMLInputStream::MatchResult::DidNotMatch);
    REQUIRE(stream.NextInputCharacter() == U'F');
  }

  TEST_CASE("HTMLInputStream: SourceLocation tracking", "[HTML][InputStream]")
  {
    HTMLInputStream stream;

    stream.Append(u8"a\nb\rc\r\ndtext", IsEOF(true));
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
    (void)stream.AdvancePast<false>(Krys::Text::ASCIILiteral::From("text")); // 'text'
    {
      const auto &loc = stream.GetCurrentLocation();
      REQUIRE(loc.Line == 4uz);
      REQUIRE(loc.Column == 6uz);
    }
  }
}
