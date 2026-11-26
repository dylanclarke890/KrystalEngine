#include "Krystal.Booey/Dom/Ranges/BoundaryPoint.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Dom/TestElement.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Dom/Nodes/Text.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    /// @brief parent > (childA > (grandchild > text), childB).
    struct PositionTestData
    {
      Ref<Document> Document;
      Ref<TestElement> Parent;
      Ref<TestElement> ChildA;
      Ref<TestElement> Grandchild;
      Ref<Text> Text;
      Ref<TestElement> ChildB;

      PositionTestData()
          : Document(CreateRef<boo::Document>()), Parent(CreateRef<TestElement>(*Document)),
            ChildA(CreateRef<TestElement>(*Document)), Grandchild(CreateRef<TestElement>(*Document)),
            Text(CreateRef<boo::Text>(*Document, u8"text")), ChildB(CreateRef<TestElement>(*Document))
      {
        REQUIRE_FALSE(Parent->AppendChild(*ChildA).HasException());
        REQUIRE_FALSE(ChildA->AppendChild(*Grandchild).HasException());
        REQUIRE_FALSE(Grandchild->AppendChild(*Text).HasException());
        REQUIRE_FALSE(Parent->AppendChild(*ChildB).HasException());
      }
    };

    /// @brief The position of (a, offsetA) relative to (b, offsetB), checked to be the reverse the other way.
    std::strong_ordering CompareBoundaryPoints(Node &a, uint64 offsetA, Node &b, uint64 offsetB)
    {
      auto position =
        BoundaryPoint {ShareRef(a), offsetA}.ComparePositionTo(BoundaryPoint {ShareRef(b), offsetB});
      auto reverse =
        BoundaryPoint {ShareRef(b), offsetB}.ComparePositionTo(BoundaryPoint {ShareRef(a), offsetA});
      CHECK((0 <=> position) == reverse);
      return position;
    }
  }

  TEST_CASE("Boundary point: a node and an offset, equal when both are",
            "[DOM][BoundaryPoint][dom:boundary-points][dom:concept-range-bp][dom:boundary-point-node]"
            "[dom:concept-range-bp-offset]")
  {
    PositionTestData data;
    BoundaryPoint point {data.Text, 2};
    CHECK(point.Container.get() == data.Text.get());
    CHECK(point.Offset == 2uz);
    CHECK(point == BoundaryPoint {data.Text, 2});
    CHECK_FALSE(point == BoundaryPoint {data.Text, 3});
    CHECK_FALSE(point == BoundaryPoint {data.Grandchild, 2});
  }

  TEST_CASE("Position of a boundary point: in the same node, by offset",
            "[DOM][BoundaryPoint][dom:concept-range-bp-position][dom:concept-range-bp-before]"
            "[dom:concept-range-bp-equal][dom:concept-range-bp-after]")
  {
    PositionTestData data;
    CHECK(CompareBoundaryPoints(*data.Text, 1, *data.Text, 1) == std::strong_ordering::equal);
    CHECK(CompareBoundaryPoints(*data.Text, 1, *data.Text, 3) == std::strong_ordering::less);
    CHECK(CompareBoundaryPoints(*data.Text, 3, *data.Text, 1) == std::strong_ordering::greater);
  }

  TEST_CASE("Position of a boundary point: in different nodes, by tree order, whatever the offsets",
            "[DOM][BoundaryPoint][dom:concept-range-bp-position]")
  {
    PositionTestData data;
    CHECK(CompareBoundaryPoints(*data.ChildA, 1, *data.ChildB, 0) == std::strong_ordering::less);
    CHECK(CompareBoundaryPoints(*data.Text, 4, *data.ChildB, 0) == std::strong_ordering::less);
    // nodeB is following nodeA, so the position is the reverse of the other way round.
    CHECK(CompareBoundaryPoints(*data.ChildB, 0, *data.Text, 4) == std::strong_ordering::greater);
  }

  TEST_CASE("Position of a boundary point: in an ancestor, after a descendant only if past its child",
            "[DOM][BoundaryPoint][dom:concept-range-bp-position]")
  {
    PositionTestData data;
    // The descendant is in the parent's child at index 0 (childA).
    CHECK(CompareBoundaryPoints(*data.Parent, 0, *data.Text, 2) == std::strong_ordering::less);
    CHECK(CompareBoundaryPoints(*data.Parent, 1, *data.Text, 2) == std::strong_ordering::greater);
    CHECK(CompareBoundaryPoints(*data.Parent, 2, *data.Text, 0) == std::strong_ordering::greater);
    // In childB, at index 1.
    CHECK(CompareBoundaryPoints(*data.Parent, 1, *data.ChildB, 0) == std::strong_ordering::less);
    CHECK(CompareBoundaryPoints(*data.Parent, 2, *data.ChildB, 0) == std::strong_ordering::greater);
    // A direct child of the ancestor.
    CHECK(CompareBoundaryPoints(*data.ChildA, 0, *data.Grandchild, 0) == std::strong_ordering::less);
    CHECK(CompareBoundaryPoints(*data.ChildA, 1, *data.Grandchild, 0) == std::strong_ordering::greater);
  }
}
