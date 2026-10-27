#include "Krystal.Booey/SVG/SVGAnimationElement.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/SVG/AnimationTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/Style/Extractor.hpp"
#include "Krystal.Booey/SVG/SVGPathElement.hpp"
#include "Krystal.Booey/SVG/SVGRectElement.hpp"
#include "Krystal.Booey/SVG/SVGSetElement.hpp"

namespace krys::boo::svg::tests
{
  using Catch::Approx;

  namespace
  {
    KRYS_NODISCARD float X(const AnimationFixture &fixture, dom::DOMStringView id, float seconds) noexcept
    {
      fixture.Seek(seconds);
      return fixture.ById<SVGRectElement>(id).X().ValueInSpecifiedUnits();
    }
  }

  TEST_CASE("SVGAnimationElement: values and calcMode", "[SVG][SVGAnimationElement]")
  {
    AnimationFixture fixture(
      u8R"html(<svg id=s>)html"
      u8R"html(<rect id=l><animate attributeName=x values="0;10;30" dur=4s /></rect>)html"
      u8R"html(<rect id=k><animate attributeName=x values="0;10;30" keyTimes="0;0.75;1" dur=4s /></rect>)html"
      u8R"html(<rect id=d><animate attributeName=x values="0;10;30" calcMode=discrete dur=3s /></rect>)html"
      u8R"html(<rect id=p><animate attributeName=x values="0;10;30" calcMode=paced dur=3s /></rect>)html"
      u8R"html(<rect id=sp><animate attributeName=x values="0;100" keyTimes="0;1" calcMode=spline )html"
      u8R"html(keySplines="0.5 0 0.5 1" dur=2s /></rect>)html"
      u8R"html(<rect id=bad x=7><animate attributeName=x values="0;10;30" keyTimes="0;1" dur=2s /></rect>)html"
      u8R"html(</svg>)html");

    // Linear: the values are spread evenly over the duration.
    CHECK(X(fixture, u8"l", 1) == Approx(5));
    CHECK(X(fixture, u8"l", 3) == Approx(20));

    // keyTimes place each value.
    CHECK(X(fixture, u8"k", 1.5) == Approx(5));
    CHECK(X(fixture, u8"k", 3.5) == Approx(20));

    // Discrete: each value holds for its share.
    CHECK(X(fixture, u8"d", 0.5) == Approx(0));
    CHECK(X(fixture, u8"d", 1.5) == Approx(10));
    CHECK(X(fixture, u8"d", 2.5) == Approx(30));

    // Paced: the values are spread by distance (10, then 20).
    CHECK(X(fixture, u8"p", 0.5) == Approx(5));
    CHECK(X(fixture, u8"p", 2) == Approx(20));

    // Spline: eased, symmetric about the middle.
    CHECK(X(fixture, u8"sp", 1) == Approx(50).margin(0.5));
    CHECK(X(fixture, u8"sp", 0.5) < 25);
    CHECK(X(fixture, u8"sp", 1.5) > 75);

    // keyTimes that don't match the values make the animation invalid.
    CHECK(X(fixture, u8"bad", 1) == Approx(7));
  }

  TEST_CASE("SVGAnimationElement: animation modes", "[SVG][SVGAnimationElement]")
  {
    AnimationFixture fixture(
      u8R"html(<svg id=s>)html"
      u8R"html(<rect id=to x=10><animate attributeName=x to=30 dur=2s /></rect>)html"
      u8R"html(<rect id=by x=10><animate attributeName=x by=20 dur=2s /></rect>)html"
      u8R"html(<rect id=fromBy x=10><animate attributeName=x from=5 by=20 dur=2s /></rect>)html"
      u8R"html(<rect id=sum x=10><animate attributeName=x from=0 to=20 additive=sum dur=2s /></rect>)html"
      u8R"html(<rect id=acc><animate attributeName=x from=0 to=10 accumulate=sum repeatCount=3 dur=1s />)html"
      u8R"html(</rect><rect id=set x=10><set id=se attributeName=x to=50 begin=1s dur=2s /></rect>)html"
      u8R"html(<rect id=none x=10><animate attributeName=x dur=2s /></rect>)html"
      u8R"html(</svg>)html");

    // A to-animation starts from the underlying value.
    CHECK(X(fixture, u8"to", 1) == Approx(20));

    // A by-animation is additive; a from-by animation isn't.
    CHECK(X(fixture, u8"by", 1) == Approx(20));
    CHECK(X(fixture, u8"fromBy", 1) == Approx(15));

    // additive=sum adds to the underlying value.
    CHECK(X(fixture, u8"sum", 1) == Approx(20));

    // accumulate=sum builds on each repeat's end value.
    CHECK(X(fixture, u8"acc", 2.5) == Approx(25));

    // <set> holds its value for the interval.
    CHECK(X(fixture, u8"set", 0.5) == Approx(10));
    CHECK(X(fixture, u8"set", 2) == Approx(50));
    CHECK(X(fixture, u8"set", 4) == Approx(10));
    CHECK(fixture.ById<SVGSetElement>(u8"se").GetAnimationMode() == AnimationMode::To);

    // Without values, to or by, there's nothing to animate.
    CHECK(X(fixture, u8"none", 1) == Approx(10));
  }

  TEST_CASE("SVGAnimationElement: changing the animation", "[SVG][SVGAnimationElement]")
  {
    AnimationFixture fixture(
      u8R"html(<svg id=s><rect id=r x=10><animate id=a attributeName=x from=0 to=100 dur=2s /></rect>)html"
      u8R"html(<rect id=w width=10><animate id=b attributeName=x from=0 to=100 dur=2s /></rect></svg>)html");
    auto &animate = fixture.ById<SVGAnimationElement>(u8"a");

    CHECK(X(fixture, u8"r", 1) == Approx(50));

    // A changed attribute restarts the interval with the new values.
    REQUIRE_FALSE(animate.SetAttribute(u8"to", u8"200").HasException());
    CHECK(X(fixture, u8"r", 1) == Approx(100));

    // So does a changed target attribute.
    auto &other = fixture.ById<SVGAnimationElement>(u8"b");
    REQUIRE_FALSE(other.SetAttribute(u8"attributeName", u8"width").HasException());
    fixture.Seek(1);
    auto &target = fixture.ById<SVGRectElement>(u8"w");
    CHECK(target.Width().ValueInSpecifiedUnits() == Approx(50));
    CHECK(target.X().ValueInSpecifiedUnits() == Approx(0));

    // Removing the animation restores the base value.
    animate.Remove();
    CHECK(X(fixture, u8"r", 1) == Approx(10));
  }

  TEST_CASE("SVGAnimationElement: CSS properties and path data", "[SVG][SVGAnimationElement]")
  {
    AnimationFixture fixture(
      u8R"html(<svg id=s><rect id=r opacity=1 width=10 height=10>)html"
      u8R"html(<animate attributeName=opacity from=0 to=1 dur=2s /></rect>)html"
      u8R"html(<path id=p d="M 0 0 L 10 0"><animate attributeName=d from="M 0 0 L 10 0" to="M 0 0 L 30 0" )html"
      u8R"html(dur=2s /></path></svg>)html");
    fixture.Seek(1);

    // Presentation attributes animate the property, in the cascade's SMIL level.
    auto &rect = fixture.ById<SVGRectElement>(u8"r");
    CHECK(style::Extractor(&rect).PropertyValueSerialisation(css::PropertyId::Opacity,
                                                             css::DefaultSerialisationContext())
          == u8"0.5");
    CHECK(rect.GetAttribute(u8"opacity") == u8"1");

    // Path data animates as a byte stream.
    auto &path = fixture.ById<SVGPathElement>(u8"p");
    CHECK(path.GetTotalLength() == Approx(20));
    fixture.Seek(3);
    CHECK(path.GetTotalLength() == Approx(10));
  }

  TEST_CASE("SVGAnimationElement: the DOM interface", "[SVG][SVGAnimationElement]")
  {
    AnimationFixture fixture(
      u8R"html(<svg id=s><rect id=r><animate id=a attributeName=x from=0 to=10 begin=1s dur=2s /></rect>)html"
      u8R"html(<rect><animate id=i attributeName=x from=0 to=10 begin=indefinite /></rect></svg>)html");
    auto &animate = fixture.ById<SVGAnimationElement>(u8"a");
    auto &indefinite = fixture.ById<SVGAnimationElement>(u8"i");

    fixture.Seek(1.5);
    CHECK(animate.GetStartTime().Value() == Approx(1));
    CHECK(animate.GetSimpleDuration().Value() == Approx(2));
    CHECK(animate.GetCurrentTime() == Approx(1.5));
    CHECK(animate.TargetElement() == &fixture.ById<SVGRectElement>(u8"r"));

    // Without a resolved begin or a determined duration, the queries throw.
    CHECK(indefinite.GetStartTime().HasException());
    CHECK(indefinite.GetSimpleDuration().HasException());
  }
}
