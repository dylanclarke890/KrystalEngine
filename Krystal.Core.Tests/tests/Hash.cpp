#include "Krystal.Core/Hash.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::tests
{
  TEST_CASE("Hash::Combine: combines into an existing seed", "[Hash]")
  {
    size_t seed = Hash::Combine(1);
    const size_t original = seed;

    seed = Hash::Combine(seed, 2);
    REQUIRE(seed == Hash::Combine(original, 2));
    REQUIRE(seed != original);
  }

  TEST_CASE("Hash::Combine: every argument contributes", "[Hash]")
  {
    REQUIRE(Hash::Combine(1, 2) != Hash::Combine(1, 3));
    REQUIRE(Hash::Combine(1, 2, 3) != Hash::Combine(1, 2, 4));
    REQUIRE(Hash::Combine(1, 2, 3) != Hash::Combine(1, 2));
    REQUIRE(Hash::Combine(1, 2) != Hash::Combine(2, 1));
  }
}
