#include "Krystal.Booey/CSS/Properties/Consumers/Transform.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumeTranslate)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumeTranslate) == "none");
    CHECK(ConsumeToText(u8"10px", ConsumeTranslate) == "10px");
    CHECK(ConsumeToText(u8"10px 20%", ConsumeTranslate) == "10px 20%");
    CHECK(ConsumeToText(u8"1px 2px 3px", ConsumeTranslate) == "1px 2px 3px");

    // A zero y is dropped unless it's a percentage or a non-zero z follows; a zero z is dropped.
    CHECK(ConsumeToText(u8"10px 0", ConsumeTranslate) == "10px");
    CHECK(ConsumeToText(u8"10px 0%", ConsumeTranslate) == "10px 0%");
    CHECK(ConsumeToText(u8"1px 0px 3px", ConsumeTranslate) == "1px 0px 3px");
    CHECK(ConsumeToText(u8"1px 0 0", ConsumeTranslate) == "1px");
    CHECK(ConsumeToText(u8"1px 2px 0", ConsumeTranslate) == "1px 2px");

    CHECK(ConsumeToText(u8"1px 2px 3%", ConsumeTranslate) == "<null>");
    CHECK(ConsumeToText(u8"1px auto", ConsumeTranslate) == "<null>");
    CHECK(ConsumeToText(u8"1deg", ConsumeTranslate) == "<null>");
    CHECK(ConsumeToText(u8"1px 2px 3px 4px", ConsumeTranslate) == "<partial>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeScale)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumeScale) == "none");
    CHECK(ConsumeToText(u8"2", ConsumeScale) == "2");
    CHECK(ConsumeToText(u8"50%", ConsumeScale) == "0.5");
    CHECK(ConsumeToText(u8"2 3", ConsumeScale) == "2 3");
    CHECK(ConsumeToText(u8"1 1 2", ConsumeScale) == "1 1 2");

    // Redundant values are dropped: y equal to x, and z of 1.
    CHECK(ConsumeToText(u8"2 2", ConsumeScale) == "2");
    CHECK(ConsumeToText(u8"2 200%", ConsumeScale) == "2");
    CHECK(ConsumeToText(u8"2 2 1", ConsumeScale) == "2");
    CHECK(ConsumeToText(u8"2 3 1", ConsumeScale) == "2 3");

    CHECK(ConsumeToText(u8"1px", ConsumeScale) == "<null>");
    CHECK(ConsumeToText(u8"1 none", ConsumeScale) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeRotate)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumeRotate) == "none");
    CHECK(ConsumeToText(u8"45deg", ConsumeRotate) == "45deg");
    CHECK(ConsumeToText(u8"x 45deg", ConsumeRotate) == "x 45deg");
    CHECK(ConsumeToText(u8"45deg y", ConsumeRotate) == "y 45deg");

    // The z axis is the default, so it isn't kept; axis vectors become identifiers.
    CHECK(ConsumeToText(u8"z 45deg", ConsumeRotate) == "45deg");
    CHECK(ConsumeToText(u8"1 0 0 45deg", ConsumeRotate) == "x 45deg");
    CHECK(ConsumeToText(u8"0 2 0 45deg", ConsumeRotate) == "y 45deg");
    CHECK(ConsumeToText(u8"0 0 1 45deg", ConsumeRotate) == "45deg");
    CHECK(ConsumeToText(u8"1 1 0 45deg", ConsumeRotate) == "1 1 0 45deg");
    CHECK(ConsumeToText(u8"45deg 1 1 0", ConsumeRotate) == "1 1 0 45deg");

    CHECK(ConsumeToText(u8"x", ConsumeRotate) == "<null>");
    CHECK(ConsumeToText(u8"x y 45deg", ConsumeRotate) == "<null>");
    CHECK(ConsumeToText(u8"1 2 45deg", ConsumeRotate) == "<null>");
    CHECK(ConsumeToText(u8"1 2 3 4 45deg", ConsumeRotate) == "<null>");
    CHECK(ConsumeToText(u8"45deg 90deg", ConsumeRotate) == "<null>");
    CHECK(ConsumeToText(u8"x 1 0 0 45deg", ConsumeRotate) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeRotate3dFunction)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"rotate3d(1, 0, 0, 45deg)", ConsumeRotate3dFunction) == "rotate3d(1, 0, 0, 45deg)");

    // A unitless zero angle is allowed.
    CHECK(ConsumeToText(u8"rotate3d(1, 2, 3, 0)", ConsumeRotate3dFunction) == "rotate3d(1, 2, 3, 0deg)");

    CHECK(ConsumeToText(u8"rotate3d(1, 0, 0)", ConsumeRotate3dFunction) == "<null>");
    CHECK(ConsumeToText(u8"rotate3d(1 0 0 45deg)", ConsumeRotate3dFunction) == "<null>");
    CHECK(ConsumeToText(u8"rotate3d(1, 0, 0, 45deg, 1)", ConsumeRotate3dFunction) == "<null>");
    CHECK(ConsumeToText(u8"rotate3d()", ConsumeRotate3dFunction) == "<null>");
    CHECK(ConsumeToText(u8"rotate(45deg)", ConsumeRotate3dFunction) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeTranslateFunction and ConsumeTranslate3dFunction)",
            "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"translate(10px)", ConsumeTranslateFunction) == "translate(10px)");
    CHECK(ConsumeToText(u8"translate(10px, 20%)", ConsumeTranslateFunction) == "translate(10px, 20%)");

    // A second argument of zero is the same as none.
    CHECK(ConsumeToText(u8"translate(10px, 0)", ConsumeTranslateFunction) == "translate(10px)");

    CHECK(ConsumeToText(u8"translate()", ConsumeTranslateFunction) == "<null>");
    CHECK(ConsumeToText(u8"translate(10px,)", ConsumeTranslateFunction) == "<null>");
    CHECK(ConsumeToText(u8"translate(1px, 2px, 3px)", ConsumeTranslateFunction) == "<null>");

    CHECK(ConsumeToText(u8"translate3d(1px, 2%, 3px)", ConsumeTranslate3dFunction)
          == "translate3d(1px, 2%, 3px)");
    CHECK(ConsumeToText(u8"translate3d(1px, 2px, 3%)", ConsumeTranslate3dFunction) == "<null>");
    CHECK(ConsumeToText(u8"translate3d(1px, 2px)", ConsumeTranslate3dFunction) == "<null>");
  }
}
