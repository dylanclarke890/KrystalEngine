#include "Krystal.Core/Types/IntervalTree.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::tests
{
  namespace
  {
    using Tree = IntervalTree<int32, int32>;

    KRYS_NODISCARD List<int32> DataOf(const List<Tree::IntervalType> &intervals)
    {
      List<int32> data;
      for (const auto &interval : intervals)
      {
        data.push_back(interval.Data());
      }
      return data;
    }
  }

  TEST_CASE("IntervalTree finds overlapping intervals in order", "[IntervalTree]")
  {
    Tree tree;
    REQUIRE(tree.isEmpty());
    tree.add({0, 10, 1});
    tree.add({5, 15, 2});
    tree.add({20, 30, 3});
    tree.add({12, 12, 4});
    tree.add({-5, -1, 5});
    REQUIRE(tree.checkInvariants());

    CHECK(DataOf(tree.allOverlaps({11, 19, 0})) == List<int32> {2, 4});
    CHECK(DataOf(tree.allOverlaps({10, 20, 0})) == List<int32> {1, 2, 4, 3});
    CHECK(DataOf(tree.allOverlaps({31, 40, 0})).empty());
    CHECK(DataOf(tree.allOverlaps({-100, 100, 0})) == List<int32> {5, 1, 2, 4, 3});

    CHECK(tree.contains({12, 12, 4}));
    CHECK_FALSE(tree.contains({12, 12, 5}));
  }

  TEST_CASE("IntervalTree removes intervals and keeps its invariants", "[IntervalTree]")
  {
    Tree tree;
    for (int32 i = 0; i < 200; ++i)
    {
      tree.add({(i * 37) % 101, (i * 37) % 101 + i % 7, i});
    }
    REQUIRE(tree.checkInvariants());

    for (int32 i = 0; i < 200; i += 2)
    {
      REQUIRE(tree.remove({(i * 37) % 101, (i * 37) % 101 + i % 7, i}));
    }
    REQUIRE(tree.checkInvariants());
    REQUIRE_FALSE(tree.remove({0, 0, 0}));

    for (const auto &interval : tree.allOverlaps({-1'000, 1'000, 0}))
    {
      CHECK(interval.Data() % 2 == 1);
    }
    CHECK(tree.allOverlaps({-1'000, 1'000, 0}).size() == 100);

    tree.clear();
    CHECK(tree.isEmpty());
  }

  TEST_CASE("IntervalTree finds the next interval after a point", "[IntervalTree]")
  {
    Tree tree;
    for (int32 low : {50, 20, 80, 10, 30, 70, 90, 25})
    {
      tree.add({low, low + 1, low});
    }

    CHECK(tree.nextIntervalAfter(0)->Data() == 10);
    CHECK(tree.nextIntervalAfter(10)->Data() == 20);
    CHECK(tree.nextIntervalAfter(21)->Data() == 25);
    CHECK(tree.nextIntervalAfter(49)->Data() == 50);
    CHECK(tree.nextIntervalAfter(75)->Data() == 80);
    CHECK_FALSE(tree.nextIntervalAfter(90));
  }
}
