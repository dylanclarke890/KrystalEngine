#include "Krystal.UI.Layout/LayoutEngine.hpp"
#include "Krystal.UI.Styles/Enums/Edge.hpp"
#include <catch_all.hpp>
#include <deque>

namespace Krys::Tests
{
  using namespace Krys::UI::Layout;
  using namespace Krys::UI::Styles;

  TEST_CASE("Ordinals::iteration", "[Ordinals]")
  {
    std::deque expectedEdges {Edge::Left, Edge::Top,        Edge::Right,    Edge::Bottom, Edge::Start,
                              Edge::End,  Edge::Horizontal, Edge::Vertical, Edge::All};

    for (auto edge : Ordinals<Edge>())
    {
      REQUIRE(edge == expectedEdges.front());
      expectedEdges.pop_front();
    }

    REQUIRE(expectedEdges.empty());
  }
}