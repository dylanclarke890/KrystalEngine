#include "Krystal.Core/Containers/IntervalTree.hpp"
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
    REQUIRE(tree.empty());
    tree.insert({0, 10, 1});
    tree.insert({5, 15, 2});
    tree.insert({20, 30, 3});
    tree.insert({12, 12, 4});
    tree.insert({-5, -1, 5});
    REQUIRE(tree.CheckInvariants());

    CHECK(DataOf(tree.AllOverlaps({11, 19, 0})) == List<int32> {2, 4});
    CHECK(DataOf(tree.AllOverlaps({10, 20, 0})) == List<int32> {1, 2, 4, 3});
    CHECK(DataOf(tree.AllOverlaps({31, 40, 0})).empty());
    CHECK(DataOf(tree.AllOverlaps({-100, 100, 0})) == List<int32> {5, 1, 2, 4, 3});

    CHECK(tree.contains({12, 12, 4}));
    CHECK_FALSE(tree.contains({12, 12, 5}));
  }

  TEST_CASE("IntervalTree removes intervals and keeps its invariants", "[IntervalTree]")
  {
    Tree tree;
    for (int32 i = 0; i < 200; ++i)
    {
      tree.insert({(i * 37) % 101, (i * 37) % 101 + i % 7, i});
    }
    REQUIRE(tree.CheckInvariants());

    for (int32 i = 0; i < 200; i += 2)
    {
      REQUIRE(tree.erase({(i * 37) % 101, (i * 37) % 101 + i % 7, i}));
    }
    REQUIRE(tree.CheckInvariants());
    REQUIRE_FALSE(tree.erase({0, 0, 0}));

    for (const auto &interval : tree.AllOverlaps({-1'000, 1'000, 0}))
    {
      CHECK(interval.Data() % 2 == 1);
    }
    CHECK(tree.AllOverlaps({-1'000, 1'000, 0}).size() == 100);

    tree.clear();
    CHECK(tree.empty());
  }

  TEST_CASE("IntervalTree finds the next interval after a point", "[IntervalTree]")
  {
    Tree tree;
    for (int32 low : {50, 20, 80, 10, 30, 70, 90, 25})
    {
      tree.insert({low, low + 1, low});
    }

    CHECK(tree.NextIntervalAfter(0).value().Data() == 10);
    CHECK(tree.NextIntervalAfter(10).value().Data() == 20);
    CHECK(tree.NextIntervalAfter(21).value().Data() == 25);
    CHECK(tree.NextIntervalAfter(49).value().Data() == 50);
    CHECK(tree.NextIntervalAfter(75).value().Data() == 80);
    CHECK_FALSE(tree.NextIntervalAfter(90));
  }
}
