#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    KRYS_NODISCARD BoundaryPoint At(Node &node, uint64 offset) noexcept
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
      CHECK(std::is_lt(TreeOrder(At(d, 0), At(bText, 1))));
      CHECK(std::is_gt(TreeOrder(At(d, 1), At(bText, 1))));
      CHECK(std::is_lt(TreeOrder(At(bText, 2), At(iText, 0))));
      CHECK(std::is_eq(TreeOrder(At(d, 1), At(d, 1))));
      CHECK(MakeBoundaryPointBeforeNode(i)->Offset == 1);
      CHECK(MakeBoundaryPointAfterNode(i)->Offset == 2);
      CHECK(MakeBoundaryPointAfterNodeContents(bText).Offset == 2);
    }

    SECTION("Ranges selecting nodes")
    {
      auto selectingB = *MakeRangeSelectingNode(b);
      CHECK(selectingB.Start == At(d, 0));
      CHECK(selectingB.End == At(d, 1));
      CHECK(!selectingB.Collapsed());
      auto contents = MakeRangeSelectingNodeContents(d);
      CHECK(contents.EndOffset() == 2);
      CHECK(Contains(contents, selectingB));
      CHECK(!Contains(selectingB, contents));
      CHECK(Contains(contents, bText));
      CHECK(!Contains(contents, p));
      CHECK(Contains(contents, At(iText, 1)));
      CHECK(CommonInclusiveAncestor(SimpleRange {At(bText, 1), At(iText, 1)}) == &d);
    }

    SECTION("Intersection and union")
    {
      SimpleRange first {At(bText, 1), At(iText, 1)};
      SimpleRange second {At(iText, 0), At(d, 2)};
      CHECK(Intersects(first, second));
      CHECK(Intersects(first, i));
      CHECK(!Intersects(first, p));
      auto common = *Intersection(first, second);
      CHECK(common.Start == At(iText, 0));
      CHECK(common.End == At(iText, 1));
      auto both = UnionRange(first, second);
      CHECK(both.Start == At(bText, 1));
      CHECK(both.End == At(d, 2));
      CHECK(!Intersection(first, SimpleRange {At(p, 0), At(p, 0)}));
      CHECK(std::is_eq(TreeOrder(first, At(i, 0))));
      CHECK(std::is_lt(TreeOrder(At(d, 0), first)));
    }

    SECTION("Intersecting nodes")
    {
      List<Node *> nodes;
      for (auto &node : IntersectingNodes(SimpleRange {At(bText, 1), At(iText, 1)}))
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
