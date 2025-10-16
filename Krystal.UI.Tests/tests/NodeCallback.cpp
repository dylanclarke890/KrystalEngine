#include "Krystal.UI/LayoutEngine.hpp"
#include "Krystal.UI/Node/Node.hpp"
#include <catch_all.hpp>

#include <ostream>
namespace Krys::Tests
{
  using namespace Krys::UI;

  static bool Equal(const Size &lhs, const Size &rhs)
  {
    return lhs.Width == rhs.Width && lhs.Height == rhs.Height;
  }

  TEST_CASE("LayoutEngine::hasMeasureFunc_initial", "[LayoutEngine]")
  {
    auto n = Node {};
    REQUIRE(!n.HasMeasureFunc());
  }

  TEST_CASE("LayoutEngine::hasMeasureFunc_with_measure_fn", "[LayoutEngine]")
  {
    auto n = Node {};
    n.SetMeasureFunc([](NodeConstRef, float, MeasureMode, float, MeasureMode) { return Size {}; });
    REQUIRE(n.HasMeasureFunc());
  }

  TEST_CASE("LayoutEngine::measure_with_measure_fn", "[LayoutEngine]")
  {
    auto n = Node {};

    n.SetMeasureFunc([](NodeConstRef, float w, MeasureMode wm, float h, MeasureMode hm)
                     { return Size {w * static_cast<float>(wm), h / static_cast<float>(hm)}; });

    REQUIRE(Equal(n.Measure(23, MeasureMode::Exactly, 24, MeasureMode::AtMost), (Size {23, 12})));
  }

  TEST_CASE("LayoutEngine::hasMeasureFunc_after_unset", "[LayoutEngine]")
  {
    auto n = Node {};
    n.SetMeasureFunc([](NodeConstRef, float, MeasureMode, float, MeasureMode) { return Size {}; });

    n.SetMeasureFunc(nullptr);
    REQUIRE(!n.HasMeasureFunc());
  }

  TEST_CASE("LayoutEngine::hasBaselineFunc_initial", "[LayoutEngine]")
  {
    auto n = Node {};
    REQUIRE(!n.HasBaselineFunc());
  }

  TEST_CASE("LayoutEngine::hasBaselineFunc_with_baseline_fn", "[LayoutEngine]")
  {
    auto n = Node {};
    n.SetBaselineFunc([](NodeConstRef, float, float) { return 0.0f; });
    REQUIRE(n.HasBaselineFunc());
  }

  TEST_CASE("LayoutEngine::baseline_with_baseline_fn", "[LayoutEngine]")
  {
    auto n = Node {};
    n.SetBaselineFunc([](NodeConstRef, float w, float h) { return w + h; });

    REQUIRE(n.Baseline(1.25f, 2.5f) == 3.75f);
  }

  TEST_CASE("LayoutEngine::hasBaselineFunc_after_unset", "[LayoutEngine]")
  {
    auto n = Node {};
    n.SetBaselineFunc([](NodeConstRef, float, float) { return 0.0f; });

    n.SetBaselineFunc(nullptr);
    REQUIRE(!n.HasBaselineFunc());
  }
}