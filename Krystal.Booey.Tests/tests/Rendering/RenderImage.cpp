#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"

namespace krys::boo::tests
{
  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "RenderImage: alt text of both directions is drawn in visual order",
                   "[Rendering][RenderImage]")
  {
    // "ab", then Hebrew shin and lamed: the Hebrew is drawn right to left, lamed first.
    PaintFixture fixture(u8"<!DOCTYPE html><img alt='ab\u05E9\u05DC' style='width:150px;height:40px'>");
    CHECK(DrawnText(*fixture.PaintedDisplayList(), u8"\u05E9\u05DC") == u8"\u05DC\u05E9");
  }
}
