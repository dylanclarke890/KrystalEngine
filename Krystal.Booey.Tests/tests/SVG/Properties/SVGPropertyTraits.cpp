#include "Krystal.Booey/SVG/Properties/SVGPropertyTraits.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGMarkerTypes.hpp"
#include "Krystal.Booey/SVG/SVGUnitTypes.hpp"
#include "Krystal.Booey/SVG/SVGZoomAndPanType.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGPropertyTraits: primitives", "[SVG][SVGPropertyTraits]")
  {
    auto document = ResolverFixture::Parse(u8"<svg id=s></svg>");
    auto &element = Downcast<SVGElement>(*document->GetElementById(u8"s"));

    CHECK(SVGPropertyTraits<bool>::FromString(element, u8"true"));
    CHECK_FALSE(SVGPropertyTraits<bool>::FromString(element, u8"TRUE"));
    CHECK(SVGPropertyTraits<bool>::ToString(false) == u8"false");

    CHECK(SVGPropertyTraits<int32>::FromString(element, u8" 12 ") == 12);
    CHECK(SVGPropertyTraits<int32>::FromString(element, u8"-3") == -3);
    CHECK(SVGPropertyTraits<int32>::FromString(element, u8"1.5") == 0);
    CHECK(SVGPropertyTraits<int32>::ToString(-7) == u8"-7");
    CHECK(SVGPropertyTraits<uint32>::ToString(7) == u8"7");
    CHECK(SVGPropertyTraits<std::pair<int32, int32>>::FromString(element, u8"2.6 3.4") == std::pair(3, 3));
    CHECK(SVGPropertyTraits<std::pair<int32, int32>>::FromString(element, u8"x") == std::pair(0, 0));

    CHECK(SVGPropertyTraits<float>::FromString(element, u8"1.5") == 1.5f);
    CHECK(SVGPropertyTraits<float>::FromString(element, u8"abc") == 0);
    dom::QualifiedName name(dom::DOMStringAtom::Null(), dom::DOMStringAtom::Null(),
                            dom::DOMStringAtom(u8"x"));
    CHECK_FALSE(SVGPropertyTraits<float>::Parse(name, u8"abc"));
    CHECK(SVGPropertyTraits<float>::Parse(name, u8"2") == 2.f);
    CHECK(SVGPropertyTraits<float>::ToString(0.25f) == u8"0.25");
    // A single number is used for both.
    CHECK(SVGPropertyTraits<std::pair<float, float>>::FromString(element, u8"4") == std::pair(4.f, 4.f));
    CHECK(SVGPropertyTraits<std::pair<float, float>>::FromString(element, u8"4, 5") == std::pair(4.f, 5.f));

    CHECK(SVGPropertyTraits<FloatPoint>::FromString(element, u8"1,2") == FloatPoint(1, 2));
    CHECK(SVGPropertyTraits<FloatPoint>::ToString(FloatPoint(1.5f, -2)) == u8"1.5 -2");
    CHECK(SVGPropertyTraits<FloatRect>::FromString(element, u8"0 0 100 50") == FloatRect(0, 0, 100, 50));
    CHECK(SVGPropertyTraits<FloatRect>::FromString(element, u8"0 0 100") == FloatRect());
    CHECK(SVGPropertyTraits<FloatRect>::ToString(FloatRect(1, 2, 3, 4)) == u8"1 2 3 4");
    CHECK(SVGPropertyTraits<dom::DOMString>::FromString(element, u8" a ") == u8" a ");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGPropertyTraits: colours", "[SVG][SVGPropertyTraits]")
  {
    auto document = ResolverFixture::Parse(u8"<svg id=s></svg>");
    auto &element = Downcast<SVGElement>(*document->GetElementById(u8"s"));

    CHECK(SVGPropertyTraits<Color>::FromString(element, u8"red") == Color::red);
    CHECK(SVGPropertyTraits<Color>::FromString(element, u8"  #00ff00\n") == Color::green);
    CHECK(SVGPropertyTraits<Color>::FromString(element, u8"color-mix(in srgb, red, blue)").IsValid());
    CHECK_FALSE(SVGPropertyTraits<Color>::FromString(element, u8"nonsense").IsValid());
    // The current colour comes from the element's renderer, which it hasn't got.
    CHECK_FALSE(SVGPropertyTraits<Color>::FromString(element, u8"currentcolor").IsValid());
    CHECK(SVGPropertyTraits<Color>::ToString(Color::red) == u8"#ff0000");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGPropertyTraits: enumerations", "[SVG][SVGPropertyTraits]")
  {
    auto document = ResolverFixture::Parse(u8"<svg id=s></svg>");
    auto &element = Downcast<SVGElement>(*document->GetElementById(u8"s"));

    using Units = SVGPropertyTraits<SVGUnitTypes::SVGUnitType>;
    CHECK(Units::FromString(element, u8"objectBoundingBox") == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX);
    CHECK(Units::FromString(element, u8"userspaceonuse") == SVGUnitTypes::SVG_UNIT_TYPE_UNKNOWN);
    CHECK(Units::ToString(SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE) == u8"userSpaceOnUse");
    CHECK(Units::HighestEnumValue() == 2);

    using MarkerUnits = SVGPropertyTraits<SVGMarkerUnitsType>;
    CHECK(MarkerUnits::FromString(element, u8"strokeWidth") == SVGMarkerUnitsType::StrokeWidth);
    CHECK(MarkerUnits::ToString(SVGMarkerUnitsType::UserSpaceOnUse) == u8"userSpaceOnUse");

    using Orient = SVGPropertyTraits<std::pair<SVGAngleValue, SVGMarkerOrientType>>;
    CHECK(Orient::FromString(element, u8"auto").second == SVGMarkerOrientAuto);
    CHECK(Orient::FromString(element, u8"auto-start-reverse").second == SVGMarkerOrientAutoStartReverse);
    auto angle = Orient::FromString(element, u8"45deg");
    CHECK(angle.second == SVGMarkerOrientAngle);
    CHECK(angle.first.Value() == 45);
    CHECK(Orient::FromString(element, u8"sideways").second == SVGMarkerOrientUnknown);
    CHECK(SVGPropertyTraits<SVGMarkerOrientType>::ToString(SVGMarkerOrientAutoStartReverse)
          == u8"auto-start-reverse");

    using ZoomAndPan = SVGPropertyTraits<SVGZoomAndPanType>;
    CHECK(ZoomAndPan::InitialValue() == SVGZoomAndPanMagnify);
    CHECK(ZoomAndPan::FromString(element, u8"disable") == SVGZoomAndPanDisable);
    CHECK(ZoomAndPan::FromString(element, u8"zoom") == SVGZoomAndPanUnknown);
  }
}
