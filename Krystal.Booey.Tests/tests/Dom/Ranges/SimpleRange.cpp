#include "Krystal.Booey/Dom/Ranges/SimpleRange.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    KRYS_NODISCARD BoundaryPoint BoundaryAt(Node &node, uint64 offset) noexcept
    {
      return {ShareRef(node), offset};
    }
  }

  TEST_CASE("SimpleRange", "[DOM][SimpleRange]")
  {
    style::tests::ResolverFixture fixture(
      u8"<!DOCTYPE html><div id=d><b id=b>ab</b><i id=i>cd</i></div><p id=p></p>",
      style::tests::WithView::No);
    auto &d = fixture.ById(u8"d");
    auto &b = fixture.ById(u8"b");
    auto &i = fixture.ById(u8"i");
    auto &p = fixture.ById(u8"p");
    auto &bText = *b.FirstChild();
    auto &iText = *i.FirstChild();

    SECTION("Tree order of boundary points")
    {
      CHECK(std::is_lt(TreeOrder(BoundaryAt(d, 0), BoundaryAt(bText, 1))));
      CHECK(std::is_gt(TreeOrder(BoundaryAt(d, 1), BoundaryAt(bText, 1))));
      CHECK(std::is_lt(TreeOrder(BoundaryAt(bText, 2), BoundaryAt(iText, 0))));
      CHECK(std::is_eq(TreeOrder(BoundaryAt(d, 1), BoundaryAt(d, 1))));
      CHECK(CreateBoundaryPointBeforeNode(i).value().Offset == 1);
      CHECK(CreateBoundaryPointAfterNode(i).value().Offset == 2);
      CHECK(CreateBoundaryPointAfterNodeContents(bText).Offset == 2);
    }

    SECTION("Ranges selecting nodes")
    {
      auto selectingB = *CreateRangeSelectingNode(b);
      CHECK(selectingB.Start == BoundaryAt(d, 0));
      CHECK(selectingB.End == BoundaryAt(d, 1));
      CHECK(!selectingB.Collapsed());
      auto contents = CreateRangeSelectingNodeContents(d);
      CHECK(contents.EndOffset() == 2);
      CHECK(Contains(contents, selectingB));
      CHECK(!Contains(selectingB, contents));
      CHECK(Contains(contents, bText));
      CHECK(!Contains(contents, p));
      CHECK(Contains(contents, BoundaryAt(iText, 1)));
      CHECK(CommonInclusiveAncestor(SimpleRange {BoundaryAt(bText, 1), BoundaryAt(iText, 1)}) == &d);
    }

    SECTION("Intersection and union")
    {
      SimpleRange first {BoundaryAt(bText, 1), BoundaryAt(iText, 1)};
      SimpleRange second {BoundaryAt(iText, 0), BoundaryAt(d, 2)};
      CHECK(Intersects(first, second));
      CHECK(Intersects(first, i));
      CHECK(!Intersects(first, p));
      auto common = *Intersection(first, second);
      CHECK(common.Start == BoundaryAt(iText, 0));
      CHECK(common.End == BoundaryAt(iText, 1));
      auto both = UnionRange(first, second);
      CHECK(both.Start == BoundaryAt(bText, 1));
      CHECK(both.End == BoundaryAt(d, 2));
      CHECK(!Intersection(first, SimpleRange {BoundaryAt(p, 0), BoundaryAt(p, 0)}));
      CHECK(std::is_eq(TreeOrder(first, BoundaryAt(i, 0))));
      CHECK(std::is_lt(TreeOrder(BoundaryAt(d, 0), first)));
    }

    SECTION("Intersecting nodes")
    {
      List<Node *> nodes;
      for (auto &node : IntersectingNodes(SimpleRange {BoundaryAt(bText, 1), BoundaryAt(iText, 1)}))
      {
        nodes.push_back(&node);
      }
      REQUIRE(nodes.size() == 3);
      CHECK(nodes[0] == &bText);
      CHECK(nodes[1] == &i);
      CHECK(nodes[2] == &iText);
    }
  }
}
