#include "Krystal.Core/Types/ListHashSet.hpp"
#include "catch2/catch_test_macros.hpp"

#include <array>
#include <vector>

namespace krys::tests
{
  TEST_CASE("ListHashSet preserves insertion order and uniqueness", "[ListHashSet]")
  {
    ListHashSet<int> values {3, 1, 3, 2};
    std::vector<int> ordered(values.begin(), values.end());

    REQUIRE(values.size() == 3);
    REQUIRE_FALSE(values.isEmpty());
    REQUIRE(values.contains(1));
    REQUIRE_FALSE(values.contains(4));
    REQUIRE(ordered == std::vector {3, 1, 2});
  }

  TEST_CASE("ListHashSet removes values and copies independently", "[ListHashSet]")
  {
    ListHashSet<int> values {1, 2, 3};
    auto copy = values;
    const ListHashSet<int> expectedValues {1, 3};
    const ListHashSet<int> expectedCopy {1, 2, 3};

    REQUIRE(values.remove(2));
    REQUIRE_FALSE(values.remove(4));
    REQUIRE(values == expectedValues);
    REQUIRE(copy == expectedCopy);

    copy.clear();
    REQUIRE(copy.empty());
  }

  TEST_CASE("ListHashSet accepts mapped values without duplicates", "[ListHashSet]")
  {
    const std::array source {1, 2, 3};
    ListHashSet<int> values;
    for (int value : source)
    {
      values.add(value % 2);
    }
    const ListHashSet<int> expected {1, 0};

    REQUIRE(values == expected);
  }

  TEST_CASE("ListHashSet appends or moves values and inserts before others", "[ListHashSet]")
  {
    ListHashSet<int> values {1, 2, 3};

    REQUIRE_FALSE(values.appendOrMoveToLast(1).second);
    REQUIRE(values == ListHashSet<int> {2, 3, 1});
    REQUIRE(values.appendOrMoveToLast(4).second);
    REQUIRE(values == ListHashSet<int> {2, 3, 1, 4});

    REQUIRE(values.insertBefore(values.find(1), 5).second);
    REQUIRE(values == ListHashSet<int> {2, 3, 5, 1, 4});
    REQUIRE_FALSE(values.insertBefore(values.begin(), 4).second);
    REQUIRE(values == ListHashSet<int> {2, 3, 5, 1, 4});
    REQUIRE(values.insertBefore(values.end(), 6).second);
    REQUIRE(values.last() == 6);
    REQUIRE(values.contains(5));
    REQUIRE(values.remove(5));
    REQUIRE(values == ListHashSet<int> {2, 3, 1, 4, 6});
  }
}
