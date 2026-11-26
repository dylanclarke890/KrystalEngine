#include "Krystal.Core/Geometry/Segment.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::tests
{
  TEST_CASE("SegmentF: length and dilation", "[Geometry][SegmentF]")
  {
    STATIC_CHECK(SegmentF {2.f, 5.f}.Length() == 3.f);
    STATIC_CHECK(SegmentF {2.f, 5.f}.Dilate(1.f) == SegmentF {1.f, 6.f});
  }

  TEST_CASE("SegmentF: the difference with dilated segments", "[Geometry][SegmentF]")
  {
    // Gaps of 1 or less are dropped, as are segments past the end.
    auto gaps = DifferenceWithDilation({0.f, 20.f}, {{12.f, 13.f}, {4.f, 5.f}, {25.f, 30.f}}, 1.f);
    CHECK(gaps == List<SegmentF> {{0.f, 3.f}, {6.f, 11.f}, {14.f, 20.f}});
    CHECK(DifferenceWithDilation({0.f, 2.f}, {{1.f, 1.f}}, 1.f).empty());
  }
}
