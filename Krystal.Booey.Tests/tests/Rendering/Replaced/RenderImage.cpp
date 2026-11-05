#include "Krystal.Booey/Rendering/Replaced/RenderImage.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include <string>

namespace krys::boo::tests
{
  namespace
  {
    /// @brief @p text as a `std::string`, which Catch can print.
    KRYS_NODISCARD std::string Narrow(utf8_stringview text) noexcept
    {
      return std::string(text.begin(), text.end());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "RenderImage: alt text of both directions is drawn in visual order",
                   "[Rendering][RenderImage]")
  {
    // "ab", then Hebrew shin and lamed: the Hebrew is drawn right to left, lamed first.
    PaintFixture fixture(u8"<!DOCTYPE html><img alt='ab\u05E9\u05DC' style='width:150px;height:40px'>");
    CHECK(DrawnText(*fixture.PaintedDisplayList(), u8"\u05E9\u05DC") == u8"\u05DC\u05E9");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "RenderImage: with dir=auto, alt text takes its first strong character's direction",
                   "[Rendering][RenderImage][html:directionality-of-the-attribute]")
  {
    // Hebrew shin and lamed, then "ab" (drawn as two runs): right to left, "ab" is on the left.
    auto drawn = [](const char8_t *attributes)
    {
      PaintFixture fixture(utf8_string(u8"<!DOCTYPE html><img ") + attributes
                           + u8" style='width:150px;height:40px'>");
      return Narrow(DrawnText(*fixture.PaintedDisplayList(), u8"ab\u05E9\u05DC"));
    };
    CHECK(drawn(u8"dir=auto alt='\u05E9\u05DCab'") == Narrow(u8"ab|\u05DC\u05E9"));
    CHECK(drawn(u8"dir=auto title='\u05E9\u05DCab'") == Narrow(u8"ab|\u05DC\u05E9"));
    // Otherwise the image's direction.
    CHECK(drawn(u8"alt='\u05E9\u05DCab'") == Narrow(u8"\u05DC\u05E9|ab"));
  }
}
