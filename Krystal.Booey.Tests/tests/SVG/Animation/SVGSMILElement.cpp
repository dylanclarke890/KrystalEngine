#include "Krystal.Booey/SVG/Animation/SVGSMILElement.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/SVG/AnimationTestUtils.hpp"
#include "Krystal.Booey/SVG/Animation/SMILTimeContainer.hpp"
#include "Krystal.Booey/SVG/SVGAnimationElement.hpp"
#include "Krystal.Booey/SVG/SVGRectElement.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  using Catch::Approx;

  namespace
  {
    /// @brief The rect's animated x at @p seconds into the document timeline.
    KRYS_NODISCARD float XAt(SVGSvgElement &svg, SVGRectElement &rect, float seconds) noexcept
    {
      svg.SetCurrentTime(seconds);
      return rect.X().ValueInSpecifiedUnits();
    }
  }

  TEST_CASE("SVGSMILElement: clock values", "[SVG][SVGSMILElement]")
  {
    auto clock = [](dom::DOMStringView value)
    {
      return SVGSMILElement::ParseClockValue(value);
    };

    // Full and partial clock values.
    CHECK(clock(u8"02:30:03") == SMILTime(2 * 3'600 + 30 * 60 + 3));
    CHECK(clock(u8"50:00:10.25") == SMILTime(50 * 3'600 + 10.25));
    CHECK(clock(u8"02:33") == SMILTime(2 * 60 + 33));
    CHECK(clock(u8"00:10.5") == SMILTime(10.5));

    // Timecount values, with and without a metric.
    CHECK(clock(u8"3.2h") == SMILTime(3.2 * 3'600));
    CHECK(clock(u8"45min") == SMILTime(45 * 60));
    CHECK(clock(u8"30s") == SMILTime(30));
    CHECK(clock(u8"5ms") == SMILTime(0.005));
    CHECK(clock(u8"12.467") == SMILTime(12.467));
    CHECK(clock(u8" 2s ") == SMILTime(2));

    // Indefinite, and the errors.
    CHECK(clock(u8"indefinite").IsIndefinite());
    CHECK(clock(u8"").IsUnresolved());
    CHECK(SVGSMILElement::ParseClockValue(std::nullopt).IsUnresolved());
    CHECK(clock(u8"2x").IsUnresolved());

    // Offsets may be negative.
    CHECK(SVGSMILElement::ParseOffsetValue(u8"-2.5s") == SMILTime(-2.5));
    CHECK(SVGSMILElement::ParseOffsetValue(u8"+1s") == SMILTime(1));
  }

  TEST_CASE("SVGSMILElement: intervals", "[SVG][SVGSMILElement]")
  {
    SECTION("begin, dur and fill")
    {
      AnimationFixture fixture(
        u8R"html(<svg id=s><rect id=r x=7><animate id=a attributeName=x from=0 to=100 begin=2s dur=4s />)html"
        u8R"html(</rect><rect id=f x=7><animate attributeName=x from=0 to=100 dur=4s fill=freeze />)html"
        u8R"html(</rect></svg>)html");
      auto &svg = fixture.Svg();
      auto &rect = fixture.ById<SVGRectElement>(u8"r");
      auto &frozen = fixture.ById<SVGRectElement>(u8"f");
      auto &animate = fixture.ById<SVGAnimationElement>(u8"a");

      // Before the interval begins, the base value shows.
      CHECK(XAt(svg, rect, 1) == Approx(7));
      CHECK(animate.GetStartTime().Value() == Approx(2));
      CHECK(animate.GetSimpleDuration().Value() == Approx(4));

      // During it, the value is interpolated.
      CHECK(XAt(svg, rect, 3) == Approx(25));
      CHECK(XAt(svg, rect, 5) == Approx(75));
      CHECK(svg.GetCurrentTime() == Approx(5));
      CHECK(animate.GetCurrentTime() == Approx(5));

      // After it, the animation is removed, unless it's frozen.
      CHECK(XAt(svg, rect, 7) == Approx(7));
      CHECK(frozen.X().ValueInSpecifiedUnits() == Approx(100));

      // Seeking back restarts it.
      CHECK(XAt(svg, rect, 4) == Approx(50));
    }

    SECTION("repeatCount, repeatDur and min/max")
    {
      AnimationFixture fixture(
        u8R"html(<svg id=s><rect id=r><animate attributeName=x from=0 to=10 dur=2s repeatCount=2.5 /></rect>)html"
        u8R"html(<rect id=d><animate attributeName=x from=0 to=10 dur=2s repeatDur=3s fill=freeze /></rect>)html"
        u8R"html(<rect id=m><animate attributeName=x from=0 to=10 dur=2s min=4s fill=freeze /></rect>)html"
        u8R"html(</svg>)html");
      auto &svg = fixture.Svg();
      auto &rect = fixture.ById<SVGRectElement>(u8"r");
      auto &repeatDur = fixture.ById<SVGRectElement>(u8"d");
      auto &min = fixture.ById<SVGRectElement>(u8"m");

      CHECK(XAt(svg, rect, 1) == Approx(5));
      CHECK(XAt(svg, rect, 3) == Approx(5));
      CHECK(XAt(svg, rect, 4.5) == Approx(2.5));
      CHECK(XAt(svg, rect, 6) == Approx(0));

      svg.SetCurrentTime(10);
      // Frozen at the active end, part way through the second repeat.
      CHECK(repeatDur.X().ValueInSpecifiedUnits() == Approx(5));
      // The active duration is at least min; the value freezes at the simple duration's end.
      CHECK(min.X().ValueInSpecifiedUnits() == Approx(10));
    }

    SECTION("syncbase and script begins")
    {
      AnimationFixture fixture(
        u8R"html(<svg id=s><rect id=r><animate id=a attributeName=x from=0 to=10 begin=1s dur=2s /></rect>)html"
        u8R"html(<rect id=q><animate attributeName=x from=0 to=10 begin=a.end+1s dur=2s /></rect>)html"
        u8R"html(<rect id=i x=3><animate id=b attributeName=x from=0 to=10 begin=indefinite dur=2s /></rect>)html"
        u8R"html(</svg>)html");
      auto &svg = fixture.Svg();
      auto &rect = fixture.ById<SVGRectElement>(u8"q");
      auto &indefinite = fixture.ById<SVGRectElement>(u8"i");
      auto &script = fixture.ById<SVGAnimationElement>(u8"b");

      // a ends at 3s, so the dependent begins at 4s.
      CHECK(XAt(svg, rect, 3.5) == Approx(0));
      CHECK(XAt(svg, rect, 5) == Approx(5));

      // An indefinite begin waits for beginElement.
      CHECK(XAt(svg, indefinite, 1) == Approx(3));
      CHECK(script.GetStartTime().HasException());
    }
  }
}
