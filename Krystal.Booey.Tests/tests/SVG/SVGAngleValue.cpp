#include "Krystal.Booey/SVG/SVGAngleValue.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include <numbers>

namespace krys::boo::svg::tests
{
  using Catch::Approx;

  TEST_CASE("SVGAngleValue: parsing and serialising", "[SVG][SVGAngleValue]")
  {
    SVGAngleValue angle;
    CHECK(angle.UnitType() == SVGAngleValue::SVG_ANGLETYPE_UNSPECIFIED);
    CHECK(angle.ValueAsString() == u8"0");

    REQUIRE_FALSE(angle.SetValueAsString(u8"90deg").HasException());
    CHECK(angle.UnitType() == SVGAngleValue::SVG_ANGLETYPE_DEG);
    CHECK(angle.Value() == 90);
    CHECK(angle.ValueAsString() == u8"90deg");

    REQUIRE_FALSE(angle.SetValueAsString(u8"100grad").HasException());
    CHECK(angle.UnitType() == SVGAngleValue::SVG_ANGLETYPE_GRAD);
    CHECK(angle.Value() == Approx(90));
    CHECK(angle.ValueAsString() == u8"100grad");

    REQUIRE_FALSE(angle.SetValueAsString(u8"0.25turn").HasException());
    CHECK(angle.Value() == Approx(90));
    CHECK(angle.ValueAsString() == u8"0.25turn");

    REQUIRE_FALSE(angle.SetValueAsString(u8"1.5rad").HasException());
    CHECK(angle.Value() == Approx(1.5 * 180 / std::numbers::pi));

    REQUIRE_FALSE(angle.SetValueAsString(u8"-45").HasException());
    CHECK(angle.UnitType() == SVGAngleValue::SVG_ANGLETYPE_UNSPECIFIED);
    CHECK(angle.ValueAsString() == u8"-45");

    // An empty string makes the unit unspecified and keeps the number.
    REQUIRE_FALSE(angle.SetValueAsString(u8"").HasException());
    CHECK(angle.UnitType() == SVGAngleValue::SVG_ANGLETYPE_UNSPECIFIED);
    CHECK(angle.ValueInSpecifiedUnits() == -45);

    // Spaces, other units and missing numbers are errors, and change nothing.
    CHECK(angle.SetValueAsString(u8"45 deg") == dom::ExceptionCode::SyntaxError);
    CHECK(angle.SetValueAsString(u8"45px") == dom::ExceptionCode::SyntaxError);
    CHECK(angle.SetValueAsString(u8"deg") == dom::ExceptionCode::SyntaxError);
    CHECK(angle.SetValueAsString(u8" 45deg") == dom::ExceptionCode::SyntaxError);
    CHECK(angle.ValueInSpecifiedUnits() == -45);
  }

  TEST_CASE("SVGAngleValue: units", "[SVG][SVGAngleValue]")
  {
    SVGAngleValue angle;
    REQUIRE_FALSE(angle.NewValueSpecifiedUnits(SVGAngleValue::SVG_ANGLETYPE_GRAD, 50).HasException());
    CHECK(angle.Value() == Approx(45));
    angle.SetValue(90);
    CHECK(angle.ValueInSpecifiedUnits() == Approx(100));

    // As upstream, the bindings can't choose turns, or an unknown unit.
    CHECK(angle.NewValueSpecifiedUnits(SVGAngleValue::SVG_ANGLETYPE_TURN, 1)
          == dom::ExceptionCode::NotSupportedError);
    CHECK(angle.NewValueSpecifiedUnits(SVGAngleValue::SVG_ANGLETYPE_UNKNOWN, 1)
          == dom::ExceptionCode::NotSupportedError);
    CHECK(angle.ConvertToSpecifiedUnits(SVGAngleValue::SVG_ANGLETYPE_TURN)
          == dom::ExceptionCode::NotSupportedError);

    REQUIRE_FALSE(angle.NewValueSpecifiedUnits(SVGAngleValue::SVG_ANGLETYPE_DEG, 180).HasException());
    REQUIRE_FALSE(angle.ConvertToSpecifiedUnits(SVGAngleValue::SVG_ANGLETYPE_RAD).HasException());
    CHECK(angle.UnitType() == SVGAngleValue::SVG_ANGLETYPE_RAD);
    CHECK(angle.ValueInSpecifiedUnits() == Approx(std::numbers::pi));
    REQUIRE_FALSE(angle.ConvertToSpecifiedUnits(SVGAngleValue::SVG_ANGLETYPE_GRAD).HasException());
    CHECK(angle.ValueInSpecifiedUnits() == Approx(200));

    REQUIRE_FALSE(angle.SetValueAsString(u8"0.5turn").HasException());
    REQUIRE_FALSE(angle.ConvertToSpecifiedUnits(SVGAngleValue::SVG_ANGLETYPE_UNSPECIFIED).HasException());
    CHECK(angle.UnitType() == SVGAngleValue::SVG_ANGLETYPE_UNSPECIFIED);
    CHECK(angle.ValueInSpecifiedUnits() == Approx(180));
  }
}
