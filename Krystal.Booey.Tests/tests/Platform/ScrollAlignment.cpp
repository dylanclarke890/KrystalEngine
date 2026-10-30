#include "Krystal.Booey/Platform/ScrollAlignment.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
{
  namespace
  {
    const LayoutRect VisibleBounds {0_lu, 0_lu, 100_lu, 50_lu};

    KRYS_NODISCARD LayoutPoint Reveal(const LayoutRect &exposeRect, const ScrollAlignment &alignX,
                                      const ScrollAlignment &alignY)
    {
      auto rect = GetRectToExposeForScrollIntoView(VisibleBounds, exposeRect, alignX, alignY);
      CHECK(rect.Size() == VisibleBounds.Size());
      return rect.Location();
    }
  }

  TEST_CASE("ScrollAlignment: the rect to expose", "[Platform][ScrollAlignment]")
  {
    const auto &ifNeeded = ScrollAlignment::AlignToEdgeIfNeeded;
    const auto &centerIfNeeded = ScrollAlignment::AlignCenterIfNeeded;

    // A visible rect doesn't scroll.
    CHECK(Reveal(LayoutRect(10_lu, 10_lu, 20_lu, 10_lu), ifNeeded, ifNeeded) == LayoutPoint());
    CHECK(Reveal(LayoutRect(10_lu, 10_lu, 20_lu, 10_lu), centerIfNeeded, centerIfNeeded) == LayoutPoint());

    // A hidden rect after the visible bounds is aligned to the far edge; one before, to the near edge.
    CHECK(Reveal(LayoutRect(200_lu, 100_lu, 20_lu, 10_lu), ifNeeded, ifNeeded) == LayoutPoint(120_lu, 60_lu));
    CHECK(Reveal(LayoutRect(-50_lu, -30_lu, 20_lu, 10_lu), ifNeeded, ifNeeded)
          == LayoutPoint(-50_lu, -30_lu));

    // Centred, rounding the vertical half-difference up.
    CHECK(Reveal(LayoutRect(200_lu, 100_lu, 20_lu, 11_lu), centerIfNeeded, centerIfNeeded)
          == LayoutPoint(160_lu, 81_lu));

    // A partly visible rect is aligned to the nearest edge even when centring if needed.
    CHECK(Reveal(LayoutRect(10_lu, 45_lu, 20_lu, 10_lu), centerIfNeeded, centerIfNeeded)
          == LayoutPoint(0_lu, 5_lu));

    // Always aligned, even when visible.
    CHECK(Reveal(LayoutRect(10_lu, 10_lu, 20_lu, 10_lu), ScrollAlignment::AlignLeftAlways,
                 ScrollAlignment::AlignTopAlways)
          == LayoutPoint(10_lu, 10_lu));
    CHECK(Reveal(LayoutRect(10_lu, 10_lu, 20_lu, 10_lu), ScrollAlignment::AlignRightAlways,
                 ScrollAlignment::AlignBottomAlways)
          == LayoutPoint(-70_lu, -30_lu));
    CHECK(Reveal(LayoutRect(10_lu, 10_lu, 20_lu, 10_lu), ScrollAlignment::NoScroll, ScrollAlignment::NoScroll)
          == LayoutPoint());

    SECTION("Horizontal visibility threshold")
    {
      // At least 32px of a horizontally clipped rect showing counts as visible...
      auto wide = LayoutRect(60_lu, 0_lu, 60_lu, 10_lu);
      CHECK(Reveal(wide, ifNeeded, ifNeeded) == LayoutPoint());

      // ...unless the threshold is turned off.
      auto alignX = ifNeeded;
      alignX.DisableLegacyHorizontalVisibilityThreshold();
      CHECK(Reveal(wide, alignX, ifNeeded) == LayoutPoint(20_lu, 0_lu));
    }

    SECTION("Visibility check rect")
    {
      // A partly visible rect doesn't scroll when only centring rects that aren't visible...
      auto exposeRect = LayoutRect(10_lu, 45_lu, 20_lu, 10_lu);
      const auto &align = ScrollAlignment::AlignCenterIfNotVisible;
      CHECK(Reveal(exposeRect, align, align) == LayoutPoint());

      // ...but it's centred if the rect its visibility is checked against is hidden.
      auto rect = GetRectToExposeForScrollIntoView(VisibleBounds, exposeRect, align, align,
                                                   LayoutRect(10_lu, 60_lu, 20_lu, 10_lu));
      CHECK(rect.Location() == LayoutPoint(0_lu, 25_lu));
    }
  }
}
