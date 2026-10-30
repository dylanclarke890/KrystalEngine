#include "Krystal.Booey/SVG/SVGGradientElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/SVG/LinearGradientAttributes.hpp"
#include "Krystal.Booey/SVG/RadialGradientAttributes.hpp"
#include "Krystal.Booey/SVG/SVGLinearGradientElement.hpp"
#include "Krystal.Booey/SVG/SVGLocatable.hpp"
#include "Krystal.Booey/SVG/SVGPatternElement.hpp"
#include "Krystal.Booey/SVG/SVGRadialGradientElement.hpp"
#include "Krystal.Booey/SVG/SVGStopElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    template <typename T>
    KRYS_NODISCARD T &ElementById(dom::Document &document, dom::DOMStringView id) noexcept
    {
      auto element = document.GetElementById(id);
      krys_assert(element);
      return Downcast<T>(Downcast<SVGElement>(*element));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGGradientElement: attributes", "[SVG][SVGGradientElement]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><svg>)html"
      u8R"html(<linearGradient id=defaults />)html"
      u8R"html(<linearGradient id=l gradientUnits=userSpaceOnUse spreadMethod=reflect gradientTransform="translate(1 2)" x1=1 y1=2 x2=3 y2=4 />)html"
      u8R"html(<linearGradient id=invalid gradientUnits=bogus spreadMethod=bogus x2=junk />)html"
      u8R"html(<radialGradient id=r cx=1 cy=2 r=3 fx=4 fy=5 fr=-1 />)html"
      u8R"html(</svg>)html"};

    // The initial values: bounding box units, padding, no transform, a line from (0%, 0%) to (100%, 0%).
    auto &defaults = ElementById<SVGLinearGradientElement>(*fixture.Document, u8"defaults");
    CHECK(defaults.GradientUnits() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX);
    CHECK(defaults.SpreadMethod() == SVGSpreadMethodPad);
    CHECK(defaults.GradientTransform().Concatenate().IsIdentity());
    CHECK(defaults.X1().ValueAsString() == u8"0%");
    CHECK(defaults.X2().ValueAsString() == u8"100%");

    auto &l = ElementById<SVGLinearGradientElement>(*fixture.Document, u8"l");
    CHECK(l.GradientUnits() == SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE);
    CHECK(l.SpreadMethod() == SVGSpreadMethodReflect);
    CHECK(l.GradientTransform().Concatenate() == AffineTransform::MakeTranslation({1, 2}));
    CHECK(l.Y2().ValueAsString() == u8"4");
    CHECK(l.SpreadMethodAnimated().BaseVal() == SVGGradientElement::SVG_SPREADMETHOD_REFLECT);

    // Invalid enumerations keep their initial values; an invalid length its fallback.
    auto &invalid = ElementById<SVGLinearGradientElement>(*fixture.Document, u8"invalid");
    CHECK(invalid.GradientUnits() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX);
    CHECK(invalid.SpreadMethod() == SVGSpreadMethodPad);
    CHECK(invalid.X2().ValueAsString() == u8"100%");

    // The focal radius can't be negative.
    auto &r = ElementById<SVGRadialGradientElement>(*fixture.Document, u8"r");
    CHECK(r.Cx().ValueAsString() == u8"1");
    CHECK(r.Fy().ValueAsString() == u8"5");
    CHECK(r.Fr().ValueAsString() == u8"0%");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGGradientElement: stops", "[SVG][SVGGradientElement]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><svg><linearGradient id=g>)html"
      u8R"html(<stop id=a offset=0.5 stop-color=red stop-opacity=0.5 />)html"
      u8R"html(<stop id=b offset="25%" style="stop-color: currentcolor; color: blue" />)html"
      u8R"html(<stop id=c offset=junk />)html"
      u8R"html(<stop id=d offset=" 1.5" />)html"
      u8R"html(<g><stop offset=0.9 /></g>)html"
      u8R"html(</linearGradient></svg>)html"};
    (void)fixture.Document->UpdateLayout();

    // Offsets are numbers or percentages; anything else is 0.
    CHECK(ElementById<SVGStopElement>(*fixture.Document, u8"a").Offset() == 0.5f);
    CHECK(ElementById<SVGStopElement>(*fixture.Document, u8"b").Offset() == 0.25f);
    CHECK(ElementById<SVGStopElement>(*fixture.Document, u8"c").Offset() == 0);
    CHECK(ElementById<SVGStopElement>(*fixture.Document, u8"d").Offset() == 1.5f);

    // The stop's colour includes its opacity, and resolves currentcolor.
    CHECK(ElementById<SVGStopElement>(*fixture.Document, u8"a").StopColorIncludingOpacity()
          == Color(Color::red).ColorWithAlphaMultipliedBy(0.5f));
    CHECK(ElementById<SVGStopElement>(*fixture.Document, u8"b").StopColorIncludingOpacity()
          == Color(Color::blue));

    // The gradient's stops are its child stops, their offsets clamped to increase and to at most 1.
    auto stops = ElementById<SVGLinearGradientElement>(*fixture.Document, u8"g").BuildStops();
    REQUIRE(stops.Stops().size() == 4);
    CHECK(stops.Stops()[0].Offset == 0.5f);
    CHECK(stops.Stops()[1].Offset == 0.5f);
    CHECK(stops.Stops()[2].Offset == 0.5f);
    CHECK(stops.Stops()[3].Offset == 1);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGGradientElement: collected attributes",
                   "[SVG][SVGGradientElement]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><svg><defs>)html"
      u8R"html(<radialGradient id=base cx=0.1 r=0.2 spreadMethod=repeat><stop stop-color=red /></radialGradient>)html"
      u8R"html(<linearGradient id=linear href="#base" x1=0.3 />)html"
      u8R"html(<radialGradient id=radial href="#linear" cy=0.4><stop stop-color=blue /><stop stop-color=lime /></radialGradient>)html"
      u8R"html(</defs></svg>)html"};
    (void)fixture.Document->UpdateLayout();

    // A gradient takes what it doesn't have from the gradients it references, but only the attributes of
    // its own kind: the linear one's x1 doesn't reach the radial one, and only the common ones cross. Its
    // own stops win over those it references, and the focal point defaults to the centre.
    RadialGradientAttributes radial;
    CHECK(
      ElementById<SVGRadialGradientElement>(*fixture.Document, u8"radial").CollectGradientAttributes(radial));
    CHECK(radial.SpreadMethod() == SVGSpreadMethodRepeat);
    CHECK(radial.Cx().ValueAsString() == u8"0.1");
    CHECK(radial.Cy().ValueAsString() == u8"0.4");
    CHECK(radial.R().ValueAsString() == u8"0.2");
    CHECK(radial.Fx().ValueAsString() == u8"0.1");
    CHECK(radial.Stops().Stops().size() == 2);

    LinearGradientAttributes linear;
    CHECK(
      ElementById<SVGLinearGradientElement>(*fixture.Document, u8"linear").CollectGradientAttributes(linear));
    CHECK(linear.X1().ValueAsString() == u8"0.3");
    CHECK(linear.SpreadMethod() == SVGSpreadMethodRepeat);
    CHECK(linear.Stops().Stops().size() == 1);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGPatternElement: attributes", "[SVG][SVGPatternElement]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><svg>)html"
      u8R"html(<pattern id=defaults />)html"
      u8R"html(<pattern id=p x=1 y=2 width=3 height=4 patternUnits=userSpaceOnUse patternContentUnits=objectBoundingBox patternTransform="scale(2)" viewBox="0 0 10 10" />)html"
      u8R"html(</svg>)html"};

    auto &defaults = ElementById<SVGPatternElement>(*fixture.Document, u8"defaults");
    CHECK(defaults.PatternUnits() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX);
    CHECK(defaults.PatternContentUnits() == SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE);

    auto &p = ElementById<SVGPatternElement>(*fixture.Document, u8"p");
    CHECK(p.X().ValueAsString() == u8"1");
    CHECK(p.Height().ValueAsString() == u8"4");
    CHECK(p.PatternUnits() == SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE);
    CHECK(p.PatternContentUnits() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX);
    CHECK(p.LocalCoordinateSpaceTransform(CTMScope::NearestViewportScope)
          == AffineTransform::MakeScale({2, 2}));
    CHECK(p.ViewBox() == FloatRect(0, 0, 10, 10));
  }
}
