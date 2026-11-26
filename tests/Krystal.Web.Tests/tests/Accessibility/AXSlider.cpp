#include "Krystal.Web/Accessibility/AXSlider.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Accessibility/AXTestUtils.hpp"

namespace krys::web::ax::tests
{
  TEST_CASE("AXSlider: a range input", "[Accessibility][AXSlider]")
  {
    AXFixture page(u8"<input id=slider type=range min=0 max=10 value=4>"
                   u8"<input id=vertical type=range aria-orientation=vertical>");

    auto &slider = page.Object(u8"slider");
    CHECK(slider.Role() == AXRole::Slider);
    CHECK(slider.ValueForRange() == 4);
    CHECK(slider.MinValueForRange() == 0);
    CHECK(slider.MaxValueForRange() == 10);
    CHECK(slider.Orientation() == AXOrientation::Horizontal);
    CHECK(page.Object(u8"vertical").Orientation() == AXOrientation::Vertical);

    SECTION("its only child is its thumb")
    {
      REQUIRE(slider.Children().size() == 1);
      auto &thumb = *slider.Children()[0];
      CHECK(thumb.Role() == AXRole::SliderThumb);
      CHECK(thumb.IsMockObject());
      CHECK(thumb.ParentObject() == &slider);
      CHECK_FALSE(thumb.ElementRect().IsEmpty());
      CHECK(slider.ElementRect().Contains(thumb.ElementRect()));
    }
  }
}
