#include "Krystal.Booey/SVG/SVGMarkerElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/SVG/SVGAngle.hpp"
#include "Krystal.Booey/SVG/SVGClipPathElement.hpp"
#include "Krystal.Booey/SVG/SVGMaskElement.hpp"
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

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGMarkerElement: attributes", "[SVG][SVGMarkerElement]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><svg><marker id=defaults /><marker id=k refX=1 refY=2 markerWidth=5 markerHeight=6 markerUnits=userSpaceOnUse orient=45deg />)html"
      u8R"html(<marker id=auto orient=auto /><marker id=reverse orient=auto-start-reverse /><marker id=invalid markerUnits=bogus orient=bogus /></svg>)html"};

    auto &defaults = ElementById<SVGMarkerElement>(*fixture.Document, u8"defaults");
    CHECK(defaults.RefX().ValueAsString() == u8"0");
    CHECK(defaults.MarkerWidth().ValueAsString() == u8"3");
    CHECK(defaults.MarkerHeight().ValueAsString() == u8"3");
    CHECK(defaults.MarkerUnits() == SVGMarkerUnitsType::StrokeWidth);
    CHECK(defaults.OrientType() == SVGMarkerOrientAngle);
    CHECK(defaults.OrientAngle().Value() == 0);

    auto &k = ElementById<SVGMarkerElement>(*fixture.Document, u8"k");
    CHECK(k.RefX().ValueAsString() == u8"1");
    CHECK(k.RefY().ValueAsString() == u8"2");
    CHECK(k.MarkerWidth().ValueAsString() == u8"5");
    CHECK(k.MarkerHeight().ValueAsString() == u8"6");
    CHECK(k.MarkerUnits() == SVGMarkerUnitsType::UserSpaceOnUse);
    CHECK(k.OrientType() == SVGMarkerOrientAngle);
    CHECK(k.OrientAngle().Value() == 45);

    CHECK(ElementById<SVGMarkerElement>(*fixture.Document, u8"auto").OrientType() == SVGMarkerOrientAuto);
    CHECK(ElementById<SVGMarkerElement>(*fixture.Document, u8"reverse").OrientType()
          == SVGMarkerOrientAutoStartReverse);
    auto &invalid = ElementById<SVGMarkerElement>(*fixture.Document, u8"invalid");
    CHECK(invalid.MarkerUnits() == SVGMarkerUnitsType::StrokeWidth);
    CHECK(invalid.OrientAngle().Value() == 0);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGMarkerElement: orient", "[SVG][SVGMarkerElement]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><svg><marker id=k /></svg>)html"};
    auto &k = ElementById<SVGMarkerElement>(*fixture.Document, u8"k");
    CHECK(k.Orient().View().empty());

    k.SetOrient(dom::DOMStringAtom(u8"auto"));
    CHECK(k.Orient().View() == u8"auto");
    CHECK(k.OrientType() == SVGMarkerOrientAuto);

    k.SetOrientToAutoStartReverse();
    CHECK(k.OrientType() == SVGMarkerOrientAutoStartReverse);

    k.SetOrientToAuto();
    CHECK(k.OrientType() == SVGMarkerOrientAuto);

    SVGAngleValue angle;
    angle.SetValue(30);
    k.SetOrientToAngle(*SVGAngle::Create(angle));
    CHECK(k.OrientAngle().Value() == 30);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGMarkerElement: the view box transform",
                   "[SVG][SVGMarkerElement]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><svg><marker id=k viewBox="0 0 2 4" /></svg>)html"};
    auto &k = ElementById<SVGMarkerElement>(*fixture.Document, u8"k");

    // The view box is fitted into the marker's viewport, keeping its aspect ratio and centred.
    CHECK(k.ViewBoxToViewTransform(4, 4) == AffineTransform(1, 0, 0, 1, 1, 0));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGClipPathElement and SVGMaskElement: attributes",
                   "[SVG][SVGClipPathElement][SVGMaskElement]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><svg><clipPath id=c /><clipPath id=bbox clipPathUnits=objectBoundingBox />)html"
      u8R"html(<mask id=defaults /><mask id=m x=1 y=2 width=3 height=4 maskUnits=userSpaceOnUse maskContentUnits=objectBoundingBox /></svg>)html"};

    CHECK(ElementById<SVGClipPathElement>(*fixture.Document, u8"c").ClipPathUnits()
          == SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE);
    CHECK(ElementById<SVGClipPathElement>(*fixture.Document, u8"bbox").ClipPathUnits()
          == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX);

    auto &defaults = ElementById<SVGMaskElement>(*fixture.Document, u8"defaults");
    CHECK(defaults.X().ValueAsString() == u8"-10%");
    CHECK(defaults.Y().ValueAsString() == u8"-10%");
    CHECK(defaults.Width().ValueAsString() == u8"120%");
    CHECK(defaults.Height().ValueAsString() == u8"120%");
    CHECK(defaults.MaskUnits() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX);
    CHECK(defaults.MaskContentUnits() == SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE);

    auto &m = ElementById<SVGMaskElement>(*fixture.Document, u8"m");
    CHECK(m.X().ValueAsString() == u8"1");
    CHECK(m.Height().ValueAsString() == u8"4");
    CHECK(m.MaskUnits() == SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE);
    CHECK(m.MaskContentUnits() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX);
  }
}
