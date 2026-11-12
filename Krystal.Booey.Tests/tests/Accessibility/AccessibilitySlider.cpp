#include "Krystal.Booey/Accessibility/AccessibilitySlider.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AccessibilitySlider: a range input", "[Accessibility][AccessibilitySlider]")
  {
    AccessibilityFixture page(u8"<input id=slider type=range min=0 max=10 value=4>"
                              u8"<input id=vertical type=range aria-orientation=vertical>");

    auto &slider = page.Object(u8"slider");
    CHECK(slider.Role() == AccessibilityRole::Slider);
    CHECK(slider.ValueForRange() == 4);
    CHECK(slider.MinValueForRange() == 0);
    CHECK(slider.MaxValueForRange() == 10);
    CHECK(slider.Orientation() == AccessibilityOrientation::Horizontal);
    CHECK(page.Object(u8"vertical").Orientation() == AccessibilityOrientation::Vertical);

    SECTION("its only child is its thumb")
    {
      REQUIRE(slider.Children().size() == 1);
      auto &thumb = *slider.Children()[0];
      CHECK(thumb.Role() == AccessibilityRole::SliderThumb);
      CHECK(thumb.IsMockObject());
      CHECK(thumb.ParentObject() == &slider);
      CHECK_FALSE(thumb.ElementRect().IsEmpty());
      CHECK(slider.ElementRect().Contains(thumb.ElementRect()));
    }
  }
}
