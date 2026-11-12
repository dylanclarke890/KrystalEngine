#include "Krystal.Booey/Accessibility/AccessibilityProgressIndicator.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AccessibilityProgressIndicator: progress and meter elements",
            "[Accessibility][AccessibilityProgressIndicator]")
  {
    AccessibilityFixture page(u8"<progress id=progress value=30 max=60></progress>"
                              u8"<progress id=indeterminate></progress>"
                              u8"<meter id=meter min=10 max=20 value=15>Half full</meter>");

    auto &progress = page.Object(u8"progress");
    CHECK(progress.Role() == AccessibilityRole::ProgressIndicator);
    CHECK_FALSE(progress.IsIgnored());
    CHECK(progress.ValueForRange() == 30);
    CHECK(progress.MaxValueForRange() == 60);
    CHECK(progress.MinValueForRange() == 0);
    CHECK_FALSE(progress.IsIndeterminate());
    CHECK(page.Object(u8"indeterminate").IsIndeterminate());

    auto &meter = page.Object(u8"meter");
    CHECK(meter.Role() == AccessibilityRole::Meter);
    CHECK(meter.ValueForRange() == 15);
    CHECK(meter.MinValueForRange() == 10);
    CHECK(meter.MaxValueForRange() == 20);
    CHECK(meter.ValueDescription() == u8"Half full");
  }

  TEST_CASE("AccessibilityProgressIndicator: ARIA progress bars take their range from ARIA",
            "[Accessibility][AccessibilityProgressIndicator]")
  {
    AccessibilityFixture page(
      u8"<div id=bar role=progressbar aria-valuenow=3 aria-valuemax=4 aria-valuetext='3 of 4'></div>"
      u8"<div id=default role=slider></div>");
    auto &bar = page.Object(u8"bar");
    CHECK(bar.ValueForRange() == 3);
    CHECK(bar.MinValueForRange() == 0);
    CHECK(bar.MaxValueForRange() == 4);
    CHECK(bar.ValueDescription() == u8"3 of 4");
    // With no value, a slider is half way between its minimum and maximum (0 and 100 by default).
    CHECK(page.Object(u8"default").ValueForRange() == 50);
    CHECK(page.Object(u8"default").Orientation() == AccessibilityOrientation::Horizontal);
  }
}
