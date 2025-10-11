#include "Krystal.UI/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("LayoutEngine::ResolveSizeUnit", "[LayoutEngine]")
  {
    UI::LayoutEngine layoutEngine;
    SECTION("Points")
    {
      UI::SizeUnit sizeUnit(72.f, UI::SizeUnitType::Points);
      float resolved = layoutEngine.ResolveSizeUnit(sizeUnit, 200.f);
      REQUIRE(resolved == 96.f); // 72 points at 96 DPI should be 96 pixels
    }

    SECTION("Pixels")
    {
      UI::SizeUnit sizeUnit(150.f, UI::SizeUnitType::Pixels);
      float resolved = layoutEngine.ResolveSizeUnit(sizeUnit, 200.f);
      REQUIRE(resolved == 150.f); // Pixels should be direct
    }

    SECTION("Percentage")
    {
      UI::SizeUnit sizeUnit(50.f, UI::SizeUnitType::Percentage);
      float resolved = layoutEngine.ResolveSizeUnit(sizeUnit, 200.f);
      REQUIRE(resolved == 100.f); // 50% of 200 should be 100
    }
  }

  TEST_CASE("LayoutEngine::ComputeLayout", "[LayoutEngine]")
  {
    UI::LayoutEngine layoutEngine;
    auto viewport = UI::BoundingBox(0.f, 0.f, 1000.f, 1000.f);

    UI::Body body;
    layoutEngine.ComputeLayout(body, viewport);

    const auto &computedLayout = body.GetComputedLayout();
    REQUIRE(computedLayout.Bounds.GetWidth() == 1000.f);
  }
}