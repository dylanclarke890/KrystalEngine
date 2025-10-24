#include "Krystal.UI.Styles/Values/UnitValue.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI::Styles;

  TEST_CASE("UnitValue::Equality", "[UnitValue]")
  {
    REQUIRE((UnitValue {12.5f, Unit::Percent}) == (UnitValue {12.5f, Unit::Percent}));
    REQUIRE((UnitValue {12.5f, Unit::Percent}) != (UnitValue {56.7f, Unit::Percent}));
    REQUIRE((UnitValue {12.5f, Unit::Percent}) != (UnitValue {12.5f, Unit::Point}));
    REQUIRE((UnitValue {12.5f, Unit::Percent}) != (UnitValue {12.5f, Unit::Auto}));
    REQUIRE((UnitValue {12.5f, Unit::Percent}) != (UnitValue {12.5f, Unit::Undefined}));

    REQUIRE((UnitValue {12.5f, Unit::Undefined})
            == (UnitValue {std::numeric_limits<float>::quiet_NaN(), Unit::Undefined}));
    REQUIRE((UnitValue {0, Unit::Auto}) == (UnitValue {-1, Unit::Auto}));
  }
}