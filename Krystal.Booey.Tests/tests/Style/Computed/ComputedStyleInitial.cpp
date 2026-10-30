#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle.hpp"

namespace krys::boo::style::tests
{
  using namespace css::Literals;

  TEST_CASE("ComputedStyle: generated initial values", "[Style][ComputedStyle]")
  {
    SECTION("Enumerations")
    {
      STATIC_CHECK(ComputedStyle::InitialDisplay() == DisplayType::Inline);
      // Keywords whose enumerator is spelled differently from the keyword.
      STATIC_CHECK(ComputedStyle::InitialDirection() == TextDirection::LTR);
      STATIC_CHECK(ComputedStyle::InitialFillRule() == WindRule::NonZero);
      STATIC_CHECK(ComputedStyle::InitialFlexWrap() == FlexWrap::NoWrap);
    }

    SECTION("Values")
    {
      CHECK(ComputedStyle::InitialColor() == boo::Color {boo::Color::black});
      CHECK(ComputedStyle::InitialOpacity() == Opacity {1});
      CHECK(ComputedStyle::InitialWidth() == PreferredSize {css::keywords::Auto {}});
      CHECK(ComputedStyle::InitialBorderTopWidth() == LineWidth {LineWidth::Length {3.0f}});
      CHECK(ComputedStyle::InitialBorderHorizontalSpacing() == WebkitBorderSpacing {0_css_px});
      CHECK(ComputedStyle::InitialBorderImageSource().IsNone());
    }

    SECTION("Name scopes")
    {
      CHECK(ComputedStyle::InitialAnchorScope() == NameScope {css::keywords::None {}});
      CHECK(ComputedStyle::InitialTimelineScope().Type == NameScope::Type::None);
    }
  }
}
