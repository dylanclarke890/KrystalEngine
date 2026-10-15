#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/LayoutRoundedRect.hpp"

namespace krys::boo::tests
{
  using Catch::Approx;

  TEST_CASE("FloatRoundedRect", "[Graphics][RoundedRect]")
  {
    FloatRoundedRect::Radii radii {FloatSize {10, 10}, FloatSize {20, 20}, FloatSize {0, 0},
                                   FloatSize {5, 5}};
    CHECK_FALSE(radii.IsZero());
    CHECK_FALSE(radii.HasEvenCorners());
    CHECK(FloatRoundedRect::Radii {4}.IsUniformCornerRadius());
    CHECK(FloatRoundedRect::Radii {}.IsZero());

    SECTION("radii")
    {
      auto scaled = radii;
      scaled.Scale(2);
      CHECK(scaled.TopLeft() == FloatSize {20, 20});
      CHECK(scaled.TopRight() == FloatSize {40, 40});

      // Expanding leaves zero corners alone.
      auto expanded = radii;
      expanded.Expand(1, 2, 3, 4);
      CHECK(expanded.TopLeft() == FloatSize {13, 11});
      CHECK(expanded.TopRight() == FloatSize {24, 21});
      CHECK(expanded.BottomLeft() == FloatSize {0, 0});
      CHECK(expanded.BottomRight() == FloatSize {9, 7});

      auto shrunk = radii;
      shrunk.Shrink(15);
      CHECK(shrunk.TopLeft() == FloatSize {0, 0});
      CHECK(shrunk.TopRight() == FloatSize {5, 5});

      auto expandedEvenIfZero = radii;
      expandedEvenIfZero.ExpandEvenIfZero(2);
      CHECK(expandedEvenIfZero.BottomLeft() == FloatSize {2, 2});
    }

    SECTION("constraints")
    {
      // The top radii sum to 30 in a rect 15 wide, so they're scaled by a half.
      FloatRect rect {0, 0, 15, 100};
      CHECK(CalcBorderRadiiConstraintScaleFor(rect, radii) == Approx(0.5));

      FloatRoundedRect roundedRect {rect, radii};
      CHECK(roundedRect.IsRounded());
      CHECK_FALSE(roundedRect.IsRenderable());
      roundedRect.AdjustRadii();
      CHECK(roundedRect.IsRenderable());
      CHECK(roundedRect.GetRadii().TopLeft() == FloatSize {5, 5});
      CHECK(roundedRect.GetRadii().TopRight() == FloatSize {10, 10});
    }

    SECTION("geometry")
    {
      FloatRoundedRect roundedRect {FloatRect {0, 0, 100, 100}, FloatRoundedRect::Radii {10}};
      CHECK(roundedRect.TopLeftCorner() == FloatRect {0, 0, 10, 10});
      CHECK(roundedRect.BottomRightCorner() == FloatRect {90, 90, 10, 10});

      // Away from the corners, the rect spans its full width; at the top, the corners pull it in.
      float minX = 0;
      float maxX = 0;
      CHECK(roundedRect.XInterceptsAtY(50, minX, maxX));
      CHECK(minX == 0);
      CHECK(maxX == 100);
      CHECK(roundedRect.XInterceptsAtY(0, minX, maxX));
      CHECK(minX == Approx(10));
      CHECK(maxX == Approx(90));
      CHECK_FALSE(roundedRect.XInterceptsAtY(150, minX, maxX));

      CHECK(roundedRect.IntersectionIsRectangular(FloatRect {20, 20, 10, 10}));
      CHECK_FALSE(roundedRect.IntersectionIsRectangular(FloatRect {0, 0, 5, 5}));

      roundedRect.InflateWithRadii(10);
      CHECK(roundedRect.Rect() == FloatRect {-10, -10, 120, 120});
      // The radii scale with the rect (here by 120 / 100).
      CHECK(roundedRect.GetRadii().TopLeft() == FloatSize {12, 12});
    }

    SECTION("regions")
    {
      // A rounded rect's region drops some of its corner pixels.
      auto region =
        ApproximateAsRegion(FloatRoundedRect {FloatRect {0, 0, 100, 100}, FloatRoundedRect::Radii {40}});
      CHECK(region.Contains(IntPoint {50, 50}));
      CHECK_FALSE(region.Contains(IntPoint {1, 1}));
      CHECK(region.Contains(IntPoint {99, 50}));
    }
  }

  TEST_CASE("LayoutRoundedRect", "[Graphics][RoundedRect]")
  {
    LayoutRoundedRect roundedRect {LayoutRect {0, 0, 100, 50},
                                   LayoutRoundedRect::Radii {{10, 10}, {10, 10}, {10, 10}, {10, 10}}};
    CHECK(roundedRect.IsRounded());
    CHECK(roundedRect.IsRenderable());
    CHECK(roundedRect.GetRadii().MinimumRadius() == 10);

    CHECK(roundedRect.Contains(LayoutRect {20, 20, 10, 10}));
    CHECK_FALSE(roundedRect.Contains(LayoutRect {0, 0, 2, 2}));

    // A quad over the clipped top-left corner misses; one through the middle hits.
    CHECK_FALSE(roundedRect.IntersectsQuad(FloatQuad {FloatRect {0, 0, 1, 1}}));
    CHECK(roundedRect.IntersectsQuad(FloatQuad {FloatRect {40, 20, 10, 10}}));

    SECTION("radii")
    {
      auto oversized = LayoutRoundedRect {LayoutRect {0, 0, 20, 20},
                                          LayoutRoundedRect::Radii {{20, 20}, {20, 20}, {0, 0}, {0, 0}}};
      CHECK_FALSE(oversized.IsRenderable());
      oversized.AdjustRadii();
      CHECK(oversized.IsRenderable());
      CHECK(oversized.GetRadii().TopLeft() == LayoutSize {10, 10});

      auto transposed = LayoutRoundedRect {LayoutRect {0, 0, 100, 50},
                                           LayoutRoundedRect::Radii {{1, 2}, {3, 4}, {5, 6}, {7, 8}}}
                          .TransposedRect();
      CHECK(transposed.Rect() == LayoutRect {0, 0, 50, 100});
      CHECK(transposed.GetRadii().TopLeft() == LayoutSize {2, 1});

      auto edges = roundedRect;
      edges.SetRadiiForEdges(LayoutRoundedRect::Radii {}, RectEdges<bool> {true, false, false, true});
      CHECK(edges.GetRadii().TopLeft().IsZero());
      CHECK_FALSE(edges.GetRadii().BottomLeft().IsZero());
    }

    SECTION("pixel snapping")
    {
      auto snapped = roundedRect.PixelSnappedRoundedRectForPainting(2);
      CHECK(snapped.Rect() == FloatRect {0, 0, 100, 50});
      CHECK(snapped.GetRadii().TopLeft() == FloatSize {10, 10});

      FloatRoundedRect fromLayout {roundedRect};
      CHECK(fromLayout.Rect() == FloatRect {0, 0, 100, 50});
    }
  }
}
