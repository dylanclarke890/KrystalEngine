#include "Krystal.Core/Text/TextStream.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::tests
{
  TEST_CASE("TextStream: values", "[Text][TextStream]")
  {
    TextStream ts;
    ts << "a" << ' ' << u8"b" << ' ' << 42 << ' ' << -7 << ' ' << 3u << ' ' << true << false;
    CHECK(ts.Release() == u8"a b 42 -7 3 10");
    CHECK(ts.IsEmpty());

    ts << 1.5 << ' ' << 2.0f << ' ' << 0.125 << ' ' << -0.001 << ' ' << 1234.567;
    CHECK(ts.Release() == u8"1.50 2.00 0.13 0.00 1234.57");

    // Exact ties round away from zero, as ECMAScript's toFixed does; rounding carries into the integer part.
    ts << 0.375 << ' ' << -0.375 << ' ' << 9.996 << ' ' << -99.999 << ' ' << 0.0;
    CHECK(ts.Release() == u8"0.38 -0.38 10.00 -100.00 0.00");

    ts << TextStream::FormatNumberRespectingIntegers(2.0) << ' '
       << TextStream::FormatNumberRespectingIntegers(2.5);
    CHECK(ts.Release() == u8"2 2.50");

    TextStream integers {TextStream::LineMode::MultipleLine,
                         {TextStream::Formatting::NumberRespectingIntegers}};
    integers << 3.0 << ' ' << 3.25f;
    CHECK(integers.Release() == u8"3 3.25");

    ts << TextStream::Repeat(3, '-');
    CHECK(ts.Release() == u8"---");
  }

  TEST_CASE("TextStream: indentation and groups", "[Text][TextStream]")
  {
    TextStream ts;
    ts << Indent << "root\n";
    {
      TextStream::IndentScope scope(ts);
      ts << Indent << "child\n";
      {
        TextStream::IndentScope inner(ts, 2);
        ts << Indent << "grandchild\n";
      }
    }
    ts << Indent << "end";
    CHECK(ts.Release() == u8"root\n  child\n      grandchild\nend");

    ts.DumpProperty(u8"width", 10);
    CHECK(ts.Release() == u8"\n(width 10)");

    TextStream singleLine {TextStream::LineMode::SingleLine};
    singleLine << "a";
    singleLine.IncreaseIndent();
    singleLine << Indent;
    singleLine.DumpProperty(u8"b", 1);
    singleLine.NextLine();
    singleLine << "c";
    CHECK(singleLine.Release() == u8"a (b 1) c");
  }

  TEST_CASE("TextStream: containers", "[Text][TextStream]")
  {
    TextStream ts;
    ts << List<int32> {1, 2, 3} << ' ' << Maybe<int32> {} << ' ' << Maybe<int32> {4} << ' '
       << Pair<int32, int32> {5, 6};
    CHECK(ts.Release() == u8"[1, 2, 3] nullopt 4 [5, 6]");

    TextStream limited {TextStream::LineMode::MultipleLine, {}, 2};
    limited << List<int32> {1, 2, 3};
    CHECK(limited.Release() == u8"[1, 2, ...]");

    int32 value = 7;
    ts << ValueOrNull<int32>(&value) << ' ' << ValueOrNull<int32>(nullptr);
    CHECK(ts.Release() == u8"7 null");
  }
}
