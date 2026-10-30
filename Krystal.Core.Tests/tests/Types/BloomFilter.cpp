#include "Krystal.Core/Types/BloomFilter.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::tests
{
  TEST_CASE("BloomFilter never gives false negatives", "[BloomFilter]")
  {
    BloomFilter<12> filter;
    REQUIRE_FALSE(filter.MayContain(0x12'34'56'78u));

    filter.Add(0x12'34'56'78u);
    filter.Add(0x9a'bc'de'f0u);
    REQUIRE(filter.MayContain(0x12'34'56'78u));
    REQUIRE(filter.MayContain(0x9a'bc'de'f0u));

    // Both halves of the hash must be set.
    REQUIRE_FALSE(filter.MayContain(0x12'34'00'01u));
    REQUIRE_FALSE(filter.MayContain(0x00'01'56'78u));

    BloomFilter<12> other;
    other.Add(0x00'42'00'42u);
    filter.Add(other);
    REQUIRE(filter.MayContain(0x00'42'00'42u));
    REQUIRE(filter.MayContain(0x12'34'56'78u));

    filter.Clear();
    REQUIRE_FALSE(filter.MayContain(0x12'34'56'78u));
    REQUIRE_FALSE(filter.MayContain(0x00'42'00'42u));
  }

  TEST_CASE("CountingBloomFilter removes what it added", "[BloomFilter]")
  {
    CountingBloomFilter<12> filter;
    REQUIRE(filter.IsClear());

    filter.Add(0x12'34'56'78u);
    filter.Add(0x12'34'56'78u);
    filter.Add(0x9a'bc'de'f0u);
    REQUIRE(filter.MayContain(0x12'34'56'78u));
    REQUIRE(filter.MayContain(0x9a'bc'de'f0u));
    REQUIRE_FALSE(filter.LikelyEmpty());

    filter.Remove(0x12'34'56'78u);
    REQUIRE(filter.MayContain(0x12'34'56'78u));
    filter.Remove(0x12'34'56'78u);
    REQUIRE_FALSE(filter.MayContain(0x12'34'56'78u));
    REQUIRE(filter.MayContain(0x9a'bc'de'f0u));

    filter.Remove(0x9a'bc'de'f0u);
    REQUIRE(filter.IsClear());
    REQUIRE(filter.LikelyEmpty());
  }

  TEST_CASE("CountingBloomFilter keeps overflowed buckets until cleared", "[BloomFilter]")
  {
    CountingBloomFilter<12> filter;
    for (uint32 i = 0; i < CountingBloomFilter<12>::MaximumCount() + 10; ++i)
    {
      filter.Add(0x00'07'00'07u);
    }
    for (uint32 i = 0; i < CountingBloomFilter<12>::MaximumCount() + 10; ++i)
    {
      filter.Remove(0x00'07'00'07u);
    }

    REQUIRE(filter.MayContain(0x00'07'00'07u));
    REQUIRE(filter.LikelyEmpty());
    REQUIRE_FALSE(filter.IsClear());

    filter.Clear();
    REQUIRE(filter.IsClear());
  }
}
