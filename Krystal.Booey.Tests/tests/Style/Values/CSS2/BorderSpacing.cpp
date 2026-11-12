#include "Krystal.Booey/Style/Values/CSS2/BorderSpacing.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;
  using namespace css::Literals;

  TEST_CASE_METHOD(BuilderFixture, "Style border-spacing", "[Style][Values][CSS2]")
  {
    auto single = Convert<BorderSpacing>(PropertyId::BorderSpacing, u8"4px");
    CHECK(single == BorderSpacing {4_css_px, 4_css_px});
    CHECK(Serialised(single) == "4px");
    CHECK(Created(single) == "4px");

    auto pair = Convert<BorderSpacing>(PropertyId::BorderSpacing, u8"4px 2px");
    CHECK(pair.Width() == 4_css_px);
    CHECK(pair.Height() == 2_css_px);
    CHECK(Serialised(pair) == "4px 2px");
    CHECK(Created(pair) == "4px 2px");

    CHECK(ComputedStyle::InitialBorderSpacing() == BorderSpacing {0_css_px});
  }
}
