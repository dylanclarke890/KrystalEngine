#include "Krystal.Web/Graphics/Geometry/BoxExtents.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::web::tests
{
  TEST_CASE("BoxExtents: expanding and contracting a RectF", "[Graphics][BoxExtents]")
  {
    RectF rect(10, 10, 10, 10);
    FloatBoxExtent box(1.0f, 2.0f, 3.0f, 4.0f);
    Expand(rect, box);
    CHECK(rect == RectF(6, 9, 16, 14));
    CHECK(RectF(10, 10, 10, 10) + box == rect);
    Contract(rect, box);
    CHECK(rect == RectF(10, 10, 10, 10));
  }
}
