#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathByteStream.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathUtilities.hpp"

namespace krys::boo::svg::tests
{
  namespace
  {
    struct ParseResult
    {
      bool Ok;
      dom::DOMString Path;
    };

    // Parses `d` into a byte stream and writes it back out.
    ParseResult RoundTrip(dom::DOMStringView d, PathParsingMode mode = PathParsingMode::UnalteredParsing)
    {
      SVGPathByteStream stream;
      bool ok = BuildSVGPathByteStreamFromString(d, stream, mode);
      dom::DOMString result;
      REQUIRE(BuildStringFromByteStream(stream, result, PathParsingMode::UnalteredParsing));
      return {ok, result};
    }

    dom::DOMString Unaltered(dom::DOMStringView d)
    {
      auto result = RoundTrip(d);
      CHECK(result.Ok);
      return result.Path;
    }

    dom::DOMString Normalised(dom::DOMStringView d)
    {
      auto result = RoundTrip(d, PathParsingMode::NormalisedParsing);
      CHECK(result.Ok);
      return result.Path;
    }

    Maybe<float> Number(dom::DOMStringView string, dom::DOMStringView remaining)
    {
      auto result = ParseNumber(string);
      CHECK(string == remaining);
      return result;
    }
  }

  TEST_CASE("SVGParserUtilities: numbers", "[SVG][SVGParserUtilities]")
  {
    CHECK(Number(u8"10", u8"") == 10.0f);
    CHECK(Number(u8"+10", u8"") == 10.0f);
    CHECK(Number(u8"-10.5", u8"") == -10.5f);
    CHECK(Number(u8".5", u8"") == 0.5f);
    CHECK(Number(u8"1e2", u8"") == 100.0f);
    CHECK(Number(u8"1E-2", u8"") == 0.01f);
    CHECK(Number(u8"1.5e+1", u8"") == 15.0f);

    // The parser stops at the first character that can't continue the number, and skips a following
    // delimiter.
    CHECK(Number(u8"10 20", u8"20") == 10.0f);
    CHECK(Number(u8"10 , 20", u8"20") == 10.0f);
    CHECK(Number(u8"10,20", u8"20") == 10.0f);
    CHECK(Number(u8"10-20", u8"-20") == 10.0f);
    CHECK(Number(u8".5.5", u8".5") == 0.5f);
    // "ex" and "em" are units, not exponents.
    CHECK(Number(u8"10ex", u8"ex") == 10.0f);
    CHECK(Number(u8"10em", u8"em") == 10.0f);

    CHECK_FALSE(Number(u8"", u8""));
    CHECK_FALSE(Number(u8"-", u8""));
    CHECK_FALSE(Number(u8"abc", u8"abc"));
    CHECK_FALSE(Number(u8"1.", u8""));
    // An exponent needs at least one more character, so this is 1 followed by "e".
    CHECK(Number(u8"1e", u8"e") == 1.0f);
    CHECK_FALSE(Number(u8"1e+", u8""));
    // Too large for a float.
    CHECK_FALSE(Number(u8"1e39", u8""));
    CHECK_FALSE(Number(u8"1e400", u8""));
    // Tiny exponents round to zero.
    CHECK(Number(u8"1e-50", u8"") == 0.0f);

    dom::DOMStringView noSkip = u8"10 20";
    CHECK(ParseNumber(noSkip, SuffixSkippingPolicy::DontSkip) == 10.0f);
    CHECK(noSkip == u8" 20");

    CHECK(ParseNumberFromString(u8"10") == 10.0f);
    CHECK(ParseNumberFromString(u8"10 ") == 10.0f);
    CHECK_FALSE(ParseNumberFromString(u8"10px"));
    CHECK_FALSE(ParseNumberFromString(u8"10 ", SuffixSkippingPolicy::DontSkip));
  }

  TEST_CASE("SVGParserUtilities: flags, points and rects", "[SVG][SVGParserUtilities]")
  {
    dom::DOMStringView flags = u8"01 1";
    CHECK(ParseArcFlag(flags) == false);
    CHECK(ParseArcFlag(flags) == true);
    CHECK(ParseArcFlag(flags) == true);
    CHECK(flags.empty());
    dom::DOMStringView badFlag = u8"2";
    CHECK_FALSE(ParseArcFlag(badFlag));

    dom::DOMStringView points = u8"1,2 3 4";
    CHECK(ParseFloatPoint(points) == FloatPoint(1, 2));
    CHECK(ParseFloatPoint(points) == FloatPoint(3, 4));
    CHECK_FALSE(ParseFloatPoint(points));

    CHECK(ParsePoint(u8"  1, 2  ") == FloatPoint(1, 2));
    CHECK_FALSE(ParsePoint(u8""));
    CHECK_FALSE(ParsePoint(u8"   "));
    CHECK_FALSE(ParsePoint(u8"1"));

    CHECK(ParseRect(u8" 1 2 3 4") == FloatRect(1, 2, 3, 4));
    CHECK(ParseRect(u8"1,2,3,4") == FloatRect(1, 2, 3, 4));
    CHECK_FALSE(ParseRect(u8"1 2 3"));

    CHECK(ParseNumberOptionalNumber(u8"5") == std::make_pair(5.0f, 5.0f));
    CHECK(ParseNumberOptionalNumber(u8"5 6") == std::make_pair(5.0f, 6.0f));
    CHECK_FALSE(ParseNumberOptionalNumber(u8""));
    CHECK_FALSE(ParseNumberOptionalNumber(u8"5 6 7"));
    CHECK_FALSE(ParseNumberOptionalNumber(u8"5 6 "));

    CHECK(IsSVGSpaceOrComma(','));
    CHECK(IsSVGSpaceOrComma(' '));
    CHECK_FALSE(IsSVGSpaceOrComma('a'));
  }

  TEST_CASE("SVGPathByteStream: round trips unaltered", "[SVG][SVGPathByteStream]")
  {
    CHECK(Unaltered(u8"") == u8"");
    CHECK(Unaltered(u8"   ") == u8"");
    CHECK(Unaltered(u8"M 10 20 L 30 40 Z") == u8"M 10 20 L 30 40 Z");
    CHECK(Unaltered(u8"m 10 20 l 30 40 z") == u8"m 10 20 l 30 40 Z");
    CHECK(Unaltered(u8"M10,20L30-40") == u8"M 10 20 L 30 -40");
    CHECK(Unaltered(u8"  M 0.1 0.25  ") == u8"M 0.1 0.25");

    CHECK(Unaltered(u8"M 0 0 H 10 h 5 V 10 v 5") == u8"M 0 0 H 10 h 5 V 10 v 5");
    CHECK(Unaltered(u8"M 0 0 C 1 2 3 4 5 6 c 1 2 3 4 5 6") == u8"M 0 0 C 1 2 3 4 5 6 c 1 2 3 4 5 6");
    CHECK(Unaltered(u8"M 0 0 S 1 2 3 4 s 1 2 3 4") == u8"M 0 0 S 1 2 3 4 s 1 2 3 4");
    CHECK(Unaltered(u8"M 0 0 Q 1 2 3 4 q 1 2 3 4") == u8"M 0 0 Q 1 2 3 4 q 1 2 3 4");
    CHECK(Unaltered(u8"M 0 0 T 1 2 t 3 4") == u8"M 0 0 T 1 2 t 3 4");
    CHECK(Unaltered(u8"M 0 0 A 10 20 30 1 0 40 50 a 1 2 3 0 1 4 5")
          == u8"M 0 0 A 10 20 30 1 0 40 50 a 1 2 3 0 1 4 5");

    // Arc flags don't need separators.
    CHECK(Unaltered(u8"M0 0a10 10 0 0110 10") == u8"M 0 0 a 10 10 0 0 1 10 10");

    // Repeated coordinates repeat the command; after a moveto they're linetos.
    CHECK(Unaltered(u8"M 0 0 10 10 20 20") == u8"M 0 0 L 10 10 L 20 20");
    CHECK(Unaltered(u8"m 0 0 10 10") == u8"m 0 0 l 10 10");
    CHECK(Unaltered(u8"M 0 0 L 1 1 2 2") == u8"M 0 0 L 1 1 L 2 2");

    // Several subpaths.
    CHECK(Unaltered(u8"M 0 0 Z M 1 1 Z") == u8"M 0 0 Z M 1 1 Z");

    // Numbers are written with six significant figures.
    CHECK(Unaltered(u8"M 1.23456789 1e10") == u8"M 1.23457 1e+10");
  }

  TEST_CASE("SVGPathByteStream: parse errors", "[SVG][SVGPathByteStream]")
  {
    // A path must start with a moveto.
    auto noMove = RoundTrip(u8"L 10 10");
    CHECK_FALSE(noMove.Ok);
    CHECK(noMove.Path == u8"");

    // Segments up to the error are kept.
    auto truncated = RoundTrip(u8"M 0 0 L 10 10 L 20");
    CHECK_FALSE(truncated.Ok);
    CHECK(truncated.Path == u8"M 0 0 L 10 10");

    auto unknown = RoundTrip(u8"M 0 0 X 1 1");
    CHECK_FALSE(unknown.Ok);
    CHECK(unknown.Path == u8"M 0 0");

    auto badFlag = RoundTrip(u8"M 0 0 A 1 1 0 2 0 1 1");
    CHECK_FALSE(badFlag.Ok);
    CHECK(badFlag.Path == u8"M 0 0");

    auto closeThenNumber = RoundTrip(u8"M 0 0 Z 10 10");
    CHECK_FALSE(closeThenNumber.Ok);
    CHECK(closeThenNumber.Path == u8"M 0 0 Z");
  }

  TEST_CASE("SVGPathByteStream: normalised parsing", "[SVG][SVGPathByteStream]")
  {
    // Relative commands become absolute, and H/V become L.
    CHECK(Normalised(u8"m 10 10 l 5 5 h 10 v 10 z") == u8"M 10 10 L 15 15 L 25 15 L 25 25 Z");
    // A close moves the current point back to the start of the subpath.
    CHECK(Normalised(u8"M 10 10 L 20 20 Z l 5 5") == u8"M 10 10 L 20 20 Z L 15 15");

    // Quadratic curves become cubics with control points two thirds of the way to the quadratic one.
    CHECK(Normalised(u8"M 0 0 Q 3 3 6 0") == u8"M 0 0 C 2 2 4 2 6 0");
    CHECK(Normalised(u8"M 0 0 q 3 3 6 0") == u8"M 0 0 C 2 2 4 2 6 0");

    // Smooth curves reflect the previous control point about the current point.
    CHECK(Normalised(u8"M 0 0 C 0 10 10 10 10 0 S 20 -10 20 0")
          == u8"M 0 0 C 0 10 10 10 10 0 C 10 -10 20 -10 20 0");
    CHECK(Normalised(u8"M 0 0 Q 3 3 6 0 T 12 0") == u8"M 0 0 C 2 2 4 2 6 0 C 8 -2 10 -2 12 0");
    // With no previous curve, the control point is the current point.
    CHECK(Normalised(u8"M 0 0 S 10 10 20 0") == u8"M 0 0 C 0 0 10 10 20 0");

    // Arcs with a zero radius, or that end where they start, are lines.
    CHECK(Normalised(u8"M 0 0 A 0 10 0 0 1 20 0") == u8"M 0 0 L 20 0");
    CHECK(Normalised(u8"M 5 5 A 10 10 0 0 1 5 5") == u8"M 5 5 L 5 5");
    CHECK(Normalised(u8"M 5 5 a 10 10 0 0 1 0 0") == u8"M 5 5 L 5 5");
  }

  TEST_CASE("SVGPathByteStream: normalised arcs", "[SVG][SVGPathByteStream]")
  {
    // A half circle of radius 10 from (0, 0) to (20, 0) becomes two quarter circle cubics through (10, -10)
    // or (10, 10) depending on the sweep.
    auto clockwise = Normalised(u8"M 0 0 A 10 10 0 0 1 20 0");
    // The end point picks up float noise (y is about 1.7e-6), so check the curves rather than the exact end.
    // 5.52285 is the quarter circle cubic constant (0.552285) times the radius.
    CHECK(clockwise.starts_with(u8"M 0 0 C "));
    CHECK(clockwise.find(u8" 10 -10 C 15.5228 -10 20 -5.52285 20 ") != dom::DOMString::npos);

    auto anticlockwise = Normalised(u8"M 0 0 A 10 10 0 0 0 20 0");
    CHECK(anticlockwise.find(u8" 10 10 C ") != dom::DOMString::npos);

    // Radii too small to reach are scaled up, so this is the same half circle.
    CHECK(Normalised(u8"M 0 0 A 1 1 0 0 1 20 0") == clockwise);

    // Relative arcs are made absolute.
    CHECK(Normalised(u8"M 0 0 a 10 10 0 0 1 20 0") == clockwise);

    // A large arc on a circle bigger than needed takes more than two quarters.
    auto large = Normalised(u8"M 0 0 A 20 20 0 1 1 20 0");
    size_t curves = 0;
    for (size_t at = large.find(u8'C'); at != dom::DOMString::npos; at = large.find(u8'C', at + 1))
    {
      ++curves;
    }
    CHECK(curves == 4);
  }

  TEST_CASE("SVGPathByteStream: API", "[SVG][SVGPathByteStream]")
  {
    SVGPathByteStream fromString(u8"M 1 2");
    CHECK_FALSE(fromString.IsEmpty());
    CHECK(fromString.Size() == 1 + 2 * sizeof(float));
    CHECK(fromString == SVGPathByteStream(u8"M1,2"));
    CHECK(fromString != SVGPathByteStream(u8"M 1 3"));
    CHECK(SVGPathByteStream::Empty().IsEmpty());
    CHECK(SVGPathByteStream(u8"") == SVGPathByteStream::Empty());

    auto copy = fromString.Copy();
    CHECK(*copy == fromString);

    SVGPathByteStream appended(u8"M 1 2");
    appended.Append(SVGPathByteStream(u8"M 0 0 L 3 4"));
    dom::DOMString result;
    REQUIRE(BuildStringFromByteStream(appended, result, PathParsingMode::UnalteredParsing,
                                      /* checkForInitialMoveTo */ false));
    CHECK(result == u8"M 1 2 M 0 0 L 3 4");

    SVGPathByteStream bytes(SVGPathByteStream::Bytes(fromString.begin(), fromString.end()));
    CHECK(bytes == fromString);

    appended.Clear();
    CHECK(appended.IsEmpty());
    CHECK(appended.Size() == 0);

    // Building a string from an empty stream succeeds with an empty string.
    dom::DOMString empty;
    CHECK(BuildStringFromByteStream(SVGPathByteStream(), empty, PathParsingMode::UnalteredParsing));
    CHECK(empty.empty());
  }
}
