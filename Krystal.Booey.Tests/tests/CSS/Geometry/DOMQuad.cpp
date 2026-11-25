#include "Krystal.Booey/CSS/Geometry/DOMQuad.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Geometry/DOMRect.hpp"
#include <cmath>

namespace krys::boo::tests
{
  TEST_CASE("DOMQuad: made from points, rectangles and dictionaries",
            "[CSS][Geometry][DOMQuad][geometry-1:dom-domquad-domquad][geometry-1:dom-domquad-fromrect]"
            "[geometry-1:create-a-domquad-from-the-domrectinit-dictionary][geometry-1:dom-domquad-fromquad]"
            "[geometry-1:create-a-domquad-from-the-domquadinit-dictionary][geometry-1:dom-domquad-p1]"
            "[geometry-1:dom-domquad-p2][geometry-1:dom-domquad-p3][geometry-1:dom-domquad-p4]")
  {
    // Points left out are the default point.
    auto quad = DOMQuad::Create({.X = 1, .Y = 2}, {.X = 3});
    CHECK(quad->P1().X() == 1);
    CHECK(quad->P1().Y() == 2);
    CHECK(quad->P2().X() == 3);
    CHECK(quad->P3().X() == 0);
    CHECK(quad->P4().W() == 1);

    // A rectangle's corners, clockwise from its origin.
    auto fromRect = DOMQuad::FromRect({.X = 2, .Y = 0, .Width = 10, .Height = 10});
    CHECK(fromRect->P1().X() == 2);
    CHECK(fromRect->P1().Y() == 0);
    CHECK(fromRect->P2().X() == 12);
    CHECK(fromRect->P2().Y() == 0);
    CHECK(fromRect->P3().X() == 12);
    CHECK(fromRect->P3().Y() == 10);
    CHECK(fromRect->P4().X() == 2);
    CHECK(fromRect->P4().Y() == 10);
    CHECK(fromRect->P3().Z() == 0);
    CHECK(fromRect->P3().W() == 1);

    auto fromQuad = DOMQuad::FromQuad({.P2 = {.X = 4, .W = 2}});
    CHECK(fromQuad->P1().X() == 0);
    CHECK(fromQuad->P2().X() == 4);
    CHECK(fromQuad->P2().W() == 2);

    // The points are the quad's own: changing one changes the quad.
    quad->P1().SetX(7);
    CHECK(quad->P1().X() == 7);
    CHECK(&quad->P1() == &quad->P1());
  }

  TEST_CASE("DOMQuad: bounds",
            "[CSS][Geometry][DOMQuad][geometry-1:dom-domquad-getbounds][geometry-1:nan-safe-minimum]"
            "[geometry-1:nan-safe-maximum]")
  {
    auto quad =
      DOMQuad::Create({.X = 40, .Y = 25}, {.X = 180, .Y = 8}, {.X = 210, .Y = 150}, {.X = 10, .Y = 180});
    auto bounds = quad->GetBounds();
    CHECK(bounds->X() == 10);
    CHECK(bounds->Y() == 8);
    CHECK(bounds->Width() == 200);
    CHECK(bounds->Height() == 172);

    // Each call makes a new rectangle.
    CHECK(quad->GetBounds().get() != bounds.get());

    // A NaN coordinate makes that axis's bounds NaN.
    quad->P3().SetX(std::nan(""));
    auto withNaN = quad->GetBounds();
    CHECK(std::isnan(withNaN->X()));
    CHECK(std::isnan(withNaN->Width()));
    CHECK(withNaN->Y() == 8);
    CHECK(withNaN->Height() == 172);
  }
}
