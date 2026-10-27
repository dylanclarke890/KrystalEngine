#include "Krystal.Booey/SVG/SVGAnimateTransformElement.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/SVG/AnimationTestUtils.hpp"
#include "Krystal.Booey/SVG/SVGRectElement.hpp"

namespace krys::boo::svg::tests
{
  using Catch::Approx;

  namespace
  {
    KRYS_NODISCARD AffineTransform TransformOf(const AnimationFixture &fixture,
                                               dom::DOMStringView id) noexcept
    {
      return fixture.ById<SVGRectElement>(id).Transform().Concatenate();
    }
  }

  TEST_CASE("SVGAnimateTransformElement: transform types", "[SVG][SVGAnimateTransformElement]")
  {
    AnimationFixture fixture(
      u8R"html(<svg id=s>)html"
      u8R"html(<rect id=t><animateTransform id=a attributeName=transform type=translate from="0 0" to="10 20" )html"
      u8R"html(dur=2s /></rect>)html"
      u8R"html(<rect id=r><animateTransform attributeName=transform type=rotate from=0 to=90 dur=2s /></rect>)html"
      u8R"html(<rect id=sc><animateTransform attributeName=transform type=scale from=1 to=3 dur=2s /></rect>)html"
      u8R"html(<rect id=sum transform="translate(100 0)"><animateTransform attributeName=transform type=scale )html"
      u8R"html(from=1 to=3 additive=sum dur=2s /></rect>)html"
      u8R"html(<rect id=m transform="translate(7 0)"><animateTransform attributeName=transform type=matrix )html"
      u8R"html(from="1 0 0 1 0 0" to="1 0 0 1 10 0" dur=2s /></rect>)html"
      u8R"html(<rect id=css transform="translate(7 0)"><animateTransform attributeName=transform )html"
      u8R"html(attributeType=CSS type=translate from=0 to=10 dur=2s /></rect>)html"
      u8R"html(</svg>)html");
    fixture.Seek(1);

    CHECK(fixture.ById<SVGAnimateTransformElement>(u8"a").TransformType()
          == SVGTransformValue::SVG_TRANSFORM_TRANSLATE);

    auto translate = TransformOf(fixture, u8"t");
    CHECK(translate.E() == Approx(5));
    CHECK(translate.F() == Approx(10));

    auto rotate = TransformOf(fixture, u8"r");
    CHECK(rotate.A() == Approx(std::cos(std::numbers::pi / 4)));
    CHECK(rotate.B() == Approx(std::sin(std::numbers::pi / 4)));

    auto scale = TransformOf(fixture, u8"sc");
    CHECK(scale.A() == Approx(2));
    CHECK(scale.D() == Approx(2));

    // additive=sum appends to the base transform list.
    auto sum = TransformOf(fixture, u8"sum");
    CHECK(sum.A() == Approx(2));
    CHECK(sum.E() == Approx(100));

    // type=matrix isn't animatable, and transform isn't a CSS attribute here.
    CHECK(TransformOf(fixture, u8"m").E() == Approx(7));
    CHECK(TransformOf(fixture, u8"css").E() == Approx(7));

    // After the interval, the base transform list is back.
    fixture.Seek(3);
    CHECK(TransformOf(fixture, u8"t").IsIdentity());
    CHECK(TransformOf(fixture, u8"sum").A() == Approx(1));
  }
}
