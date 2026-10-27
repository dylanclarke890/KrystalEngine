#include "Krystal.Booey/SVG/SVGAnimateMotionElement.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/SVG/AnimationTestUtils.hpp"
#include "Krystal.Booey/SVG/SVGMPathElement.hpp"
#include "Krystal.Booey/SVG/SVGPathElement.hpp"
#include "Krystal.Booey/SVG/SVGRectElement.hpp"

namespace krys::boo::svg::tests
{
  using Catch::Approx;

  namespace
  {
    KRYS_NODISCARD AffineTransform MotionOf(const AnimationFixture &fixture, dom::DOMStringView id) noexcept
    {
      auto *transform = fixture.ById<SVGRectElement>(id).SupplementalTransform();
      return transform ? *transform : AffineTransform();
    }
  }

  TEST_CASE("SVGAnimateMotionElement: motion paths", "[SVG][SVGAnimateMotionElement]")
  {
    AnimationFixture fixture(
      u8R"html(<svg id=s><path id=mp d="M 0 0 H 40" display=none />)html"
      u8R"html(<rect id=ft><animateMotion from="0,0" to="100,50" dur=2s /></rect>)html"
      u8R"html(<rect id=by><animateMotion by="10,20" dur=2s /></rect>)html"
      u8R"html(<rect id=path><animateMotion path="M 0 0 L 100 0" dur=2s /></rect>)html"
      u8R"html(<rect id=auto><animateMotion path="M 0 0 L 0 100" rotate=auto dur=2s /></rect>)html"
      u8R"html(<rect id=reverse><animateMotion path="M 0 0 L 0 100" rotate=auto-reverse dur=2s /></rect>)html"
      u8R"html(<rect id=mpath><animateMotion id=a path="M 0 0 L 0 100" dur=2s><mpath id=m href=#mp />)html"
      u8R"html(</animateMotion></rect>)html"
      u8R"html(<rect id=keys><animateMotion path="M 0 0 H 100" keyPoints="0;0.5;1" keyTimes="0;0.75;1" )html"
      u8R"html(calcMode=linear dur=4s /></rect>)html"
      u8R"html(</svg>)html");
    fixture.Seek(1);

    // From, to and by animate a translation.
    auto fromTo = MotionOf(fixture, u8"ft");
    CHECK(fromTo.E() == Approx(50));
    CHECK(fromTo.F() == Approx(25));
    CHECK(MotionOf(fixture, u8"by").F() == Approx(10));

    // A path is followed by distance.
    CHECK(MotionOf(fixture, u8"path").E() == Approx(50));

    // rotate=auto turns with the path's direction.
    auto turned = MotionOf(fixture, u8"auto");
    CHECK(turned.F() == Approx(50));
    CHECK(turned.B() == Approx(1));
    CHECK(MotionOf(fixture, u8"reverse").B() == Approx(-1));

    // An <mpath> child overrides the path attribute.
    auto &mpath = fixture.ById<SVGMPathElement>(u8"m");
    REQUIRE(mpath.PathElement());
    CHECK(MotionOf(fixture, u8"mpath").E() == Approx(20));
    CHECK(MotionOf(fixture, u8"mpath").F() == Approx(0));

    // keyPoints map the keyTimes to distances along the path.
    fixture.Seek(1.5);
    CHECK(MotionOf(fixture, u8"keys").E() == Approx(25));

    // Changes to the referenced path's data update the motion path (a rendered path's data comes from its
    // style, so it's unrendered here).
    auto &referenced = fixture.ById<SVGPathElement>(u8"mp");
    REQUIRE_FALSE(referenced.SetAttribute(u8"d", u8"M 0 0 H 80").HasException());
    fixture.Seek(1);
    CHECK(MotionOf(fixture, u8"mpath").E() == Approx(40));

    // Without the <mpath>, the path attribute is used.
    mpath.Remove();
    fixture.Seek(1);
    CHECK(MotionOf(fixture, u8"mpath").E() == Approx(0).margin(1e-4));
    CHECK(MotionOf(fixture, u8"mpath").F() == Approx(50));

    // After the interval, the supplemental transform is reset.
    fixture.Seek(5);
    CHECK(MotionOf(fixture, u8"path").IsIdentity());
  }
}
