#include "Krystal.Core/Types/FixedVector.hpp"
#include "catch2/catch_test_macros.hpp"

#include <array>
#include <vector>

namespace krys::tests
{
  TEST_CASE("FixedVector stores a fixed-size sequence", "[FixedVector]")
  {
    FixedVector<int> values {1, 2, 3};

    REQUIRE(values.size() == 3);
    REQUIRE_FALSE(values.isEmpty());
    REQUIRE(values.first() == 1);
    REQUIRE(values.last() == 3);
    REQUIRE(values[1] == 2);
    REQUIRE(values.contains(2));
    REQUIRE(values.find(3) == 2);
    REQUIRE(values.reverseFind(2) == 1);
    REQUIRE(values.containsIf([](int value) { return value > 2; }));
  }

  TEST_CASE("FixedVector maps ranges and creates values from a generator", "[FixedVector]")
  {
    const std::array source {1, 2, 3};
    auto mapped = FixedVector<int>::map(source, [](int value) { return value * 2; });
    auto generated = FixedVector<int>::createWithSizeFromGenerator(3, [](size_t index) {
      return static_cast<int>(index + 1);
    });
    const FixedVector<int> expectedMapped {2, 4, 6};
    const FixedVector<int> expectedGenerated {1, 2, 3};
    const std::vector expected {3, 5, 7};

    REQUIRE(mapped == expectedMapped);
    REQUIRE(generated == expectedGenerated);
    REQUIRE(mapped.map([](int value) { return value + 1; }) == expected);
  }
}
