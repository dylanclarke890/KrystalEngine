#include "Krystal.Booey/HTML/Parser/InputStream.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE("InputStream: basic iteration", "[HTML][InputStream]")
  {
    InputStream stream;
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

  TEST_CASE("InputStream: CR normalizes to LF", "[HTML][InputStream]")
  {
    InputStream stream;
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

  TEST_CASE("InputStream: CRLF collapses to single LF", "[HTML][InputStream]")
  {
    InputStream stream;
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

  TEST_CASE("InputStream: LF preserved", "[HTML][InputStream]")
  {
    InputStream stream;
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

  TEST_CASE("InputStream: incremental append", "[HTML][InputStream]")
  {
    InputStream stream;
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

  TEST_CASE("InputStream: Peek does not consume", "[HTML][InputStream]")
  {
    InputStream stream;
    stream.Append(u8"x", IsEOF(true));

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'x');

    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == U'x');

    stream.Advance();
    REQUIRE(stream.Peek());
    REQUIRE(stream.NextInputCharacter() == EOFMarker);
  }

  TEST_CASE("InputStream: Advance past advances the correct amount of characters", "[HTML][InputStream]")
  {
    InputStream stream;
    stream.Append(u8"aBcDeF", IsEOF(true));

    auto result = stream.AdvancePast<false>("aBc");
    REQUIRE(result == InputStream::MatchResult::Matched);
    REQUIRE(stream.NextInputCharacter() == U'D');

    result = stream.AdvancePast<>("dE");
    REQUIRE(result == InputStream::MatchResult::Matched);
    REQUIRE(stream.NextInputCharacter() == U'F');

    result = stream.AdvancePast<>("XYZ");
    REQUIRE(result == InputStream::MatchResult::NotEnoughCharacters);
    REQUIRE(stream.NextInputCharacter() == U'F');

    result = stream.AdvancePast<>("G");
    REQUIRE(result == InputStream::MatchResult::DidNotMatch);
    REQUIRE(stream.NextInputCharacter() == U'F');
  }

  TEST_CASE("InputStream: SourceLocation tracking", "[HTML][InputStream]")
  {
    InputStream stream;

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
    (void)stream.AdvancePast<false>("text");
    {
      const auto &loc = stream.GetCurrentLocation();
      REQUIRE(loc.Line == 4uz);
      REQUIRE(loc.Column == 6uz);
    }
  }
}
