#include "Krystal.Core/Text/StringBuilder.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Text/HexNumber.hpp"
#include <limits>

namespace krys::tests
{
  namespace
  {
    template <typename... T>
    utf8_string Build(const T &...values)
    {
      StringBuilder builder;
      builder.Append(values...);
      return krys::move(builder).ToString();
    }

    enum class Colour : uint8
    {
      Red = 3
    };

    struct Bracketed
    {
      utf8_stringview Inner;

      void WriteTo(StringBuilder &builder) const noexcept
      {
        builder.Append(u8'[', Inner, u8']');
      }
    };
  }

  TEST_CASE("StringBuilder: strings and characters", "[Text][StringBuilder]")
  {
    StringBuilder builder;
    CHECK(builder.IsEmpty());
    builder.Append(u8"abc", utf8_string {u8"def"}, utf8_stringview {u8"g"});
    builder.Append("hi", u8'j', 'k');
    builder.Append(StringAtom {u8"lm"});
    CHECK(builder.ToString() == u8"abcdefghijklm");
    CHECK(builder.Length() == 13);
    CHECK(builder[1] == u8'b');
    CHECK(builder == u8"abcdefghijklm");
    CHECK(builder.View().starts_with(u8"abc"));
    CHECK(builder.ContainsOnlyASCII());

    SECTION("Code points are encoded as UTF-8")
    {
      builder.Clear();
      builder.Append(U'a', U'\u00E9', U'\u4E00', U'\U0001F600');
      CHECK(builder.ToString() == u8"a\u00E9\u4E00\U0001F600");
      CHECK(builder.Length() == 10);
      CHECK_FALSE(builder.ContainsOnlyASCII());
    }

    SECTION("Substrings, repeats and shrinking")
    {
      builder.Clear();
      builder.AppendSubstring(u8"abcdef", 2, 3);
      builder.AppendSubstring(u8"xyz", 1);
      builder.AppendRepeated(u8'-', 3);
      CHECK(builder.ToString() == u8"cdeyz---");
      builder.Shrink(2);
      CHECK(builder.ToString() == u8"cd");
      builder.ReserveCapacity(100);
      CHECK(builder.Capacity() >= 100);
      builder.ShrinkToFit();
      CHECK(builder.ToString() == u8"cd");
    }

    SECTION("Builders append to and swap with builders")
    {
      StringBuilder other;
      other.Append(u8"xy");
      builder.Clear();
      builder.Append(u8'<', other, u8'>');
      CHECK(builder.ToString() == u8"<xy>");
      builder.Swap(other);
      CHECK(builder.ToString() == u8"xy");
      CHECK(other.ToString() == u8"<xy>");
      CHECK(other.ToAtomString() == StringAtom {u8"<xy>"});
    }

    SECTION("Moving the string out")
    {
      auto string = krys::move(builder).ToString();
      CHECK(string == u8"abcdefghijklm");
    }
  }

  TEST_CASE("StringBuilder: numbers", "[Text][StringBuilder]")
  {
    SECTION("Integers and enums")
    {
      CHECK(Build(0, -12, 345u, int64 {-9'000'000'000}) == u8"0-12345-9000000000");
      CHECK(Build(std::numeric_limits<uint64>::max()) == u8"18446744073709551615");
      CHECK(Build(uint8 {200}, int16 {-7}) == u8"200-7");
      CHECK(Build(Colour::Red) == u8"3");
    }

    SECTION("Floating-point numbers are written as ECMAScript writes them")
    {
      CHECK(Build(1.5) == u8"1.5");
      CHECK(Build(-0.25) == u8"-0.25");
      CHECK(Build(100.0) == u8"100");
      CHECK(Build(0.1) == u8"0.1");
      CHECK(Build(0.0) == u8"0");
      CHECK(Build(-0.0) == u8"0");
      CHECK(Build(123456789.125) == u8"123456789.125");
      CHECK(Build(0.000001) == u8"0.000001");
      CHECK(Build(0.0000001) == u8"1e-7");
      CHECK(Build(1.5e-10) == u8"1.5e-10");
      CHECK(Build(1e20) == u8"100000000000000000000");
      CHECK(Build(1e21) == u8"1e+21");
      CHECK(Build(-1.25e300) == u8"-1.25e+300");
      CHECK(Build(std::numeric_limits<double>::infinity()) == u8"Infinity");
      CHECK(Build(-std::numeric_limits<double>::infinity()) == u8"-Infinity");
      CHECK(Build(std::numeric_limits<double>::quiet_NaN()) == u8"NaN");
    }

    SECTION("Floats are written with float precision")
    {
      CHECK(Build(0.1f) == u8"0.1");
      CHECK(Build(1.0f / 3) == u8"0.33333334");
      CHECK(Build(1.0 / 3) == u8"0.3333333333333333");
    }

    SECTION("Mixed")
    {
      CHECK(Build(u8"width: ", 12.5, "px; z: ", -3) == u8"width: 12.5px; z: -3");
    }
  }

  TEST_CASE("StringBuilder: writers", "[Text][StringBuilder]")
  {
    CHECK(Build(u8"a", Bracketed {u8"b"}, u8"c") == u8"a[b]c");
  }

  TEST_CASE("StringBuilder: hexadecimal numbers", "[Text][StringBuilder]")
  {
    CHECK(Build(Hex(0)) == u8"0");
    CHECK(Build(Hex(255)) == u8"FF");
    CHECK(Build(Hex(255, HexConversionMode::Lowercase)) == u8"ff");
    CHECK(Build(Hex(0x1F600u, 6)) == u8"01F600");
    CHECK(Build(Hex(uint8 {0xA}, 2, HexConversionMode::Lowercase)) == u8"0a");
    CHECK(Build(Hex(U'\u00E9')) == u8"E9");
    // Negative numbers are written as their two's complement.
    CHECK(Build(Hex(int8 {-1})) == u8"FF");
    CHECK(Build(Hex(std::numeric_limits<uint64>::max())) == u8"FFFFFFFFFFFFFFFF");
    CHECK(Build(u8"U+", Hex(0x41), u8'-', Hex(0x5A)) == u8"U+41-5A");
  }
}
