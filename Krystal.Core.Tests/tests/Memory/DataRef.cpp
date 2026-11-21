#include "Krystal.Core/Memory/DataRef.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Containers/TinyLRUCache.hpp"
#include "Krystal.Core/Memory/PointerComparison.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"

namespace krys::tests
{
  namespace
  {
    struct Counter : RefCounted<Counter>
    {
      int Value {0};

      explicit Counter(int value) noexcept : Value {value}
      {
      }

      KRYS_NODISCARD static Ref<Counter> Create(int value) noexcept
      {
        return AdoptRef(*new Counter(value));
      }

      KRYS_NODISCARD Ref<Counter> Copy() const noexcept
      {
        return Create(Value);
      }

      KRYS_NODISCARD bool operator==(const Counter &other) const noexcept
      {
        return Value == other.Value;
      }
    };

    /// @brief Counts the values it creates, so tests can see when the cache misses.
    struct CountingPolicy : TinyLRUCachePolicy<int, int>
    {
      static inline int Created = 0;

      KRYS_NODISCARD static bool IsKeyNull(const int &key) noexcept
      {
        return key < 0;
      }

      KRYS_NODISCARD static int CreateValueForKey(const int &key) noexcept
      {
        ++Created;
        return key * 10;
      }
    };
  }

  TEST_CASE("DataRef copies on write", "[DataRef]")
  {
    DataRef<Counter> a {Counter::Create(1)};
    auto b = a;
    // Copies share the data until one of them writes.
    REQUIRE(a.get() == b.get());
    REQUIRE((a == b));

    b.Access().Value = 2;
    REQUIRE(a.get() != b.get());
    REQUIRE(a->Value == 1);
    REQUIRE(b->Value == 2);
    REQUIRE_FALSE((a == b));

    // Data that isn't shared is written in place.
    const auto *before = b.get();
    b.Access().Value = 3;
    REQUIRE(b.get() == before);

    // Separate data with equal values compares equal.
    REQUIRE((DataRef<Counter> {Counter::Create(4)} == DataRef<Counter> {Counter::Create(4)}));
  }

  TEST_CASE("TinyLRUCache keeps the most recently used values", "[TinyLRUCache]")
  {
    TinyLRUCache<int, int, 2, CountingPolicy> cache;
    CountingPolicy::Created = 0;

    REQUIRE(cache.Get(1) == 10);
    REQUIRE(cache.Get(2) == 20);
    REQUIRE(cache.Get(1) == 10);
    REQUIRE(CountingPolicy::Created == 2);

    // 2 is now the least recently used, so 3 evicts it.
    REQUIRE(cache.Get(3) == 30);
    REQUIRE(cache.Get(1) == 10);
    REQUIRE(CountingPolicy::Created == 3);
    REQUIRE(cache.Get(2) == 20);
    REQUIRE(CountingPolicy::Created == 4);

    // Null keys share one default value and aren't cached.
    REQUIRE(cache.Get(-1) == 0);
    REQUIRE(CountingPolicy::Created == 4);
  }

  TEST_CASE("ArePointingToEqualData", "[PointerComparison]")
  {
    RefPtr<Counter> null;
    auto one = RefPtr<Counter> {Counter::Create(1)};
    auto otherOne = RefPtr<Counter> {Counter::Create(1)};
    auto two = RefPtr<Counter> {Counter::Create(2)};

    REQUIRE(ArePointingToEqualData(null, null));
    REQUIRE(ArePointingToEqualData(one, one));
    REQUIRE(ArePointingToEqualData(one, otherOne));
    REQUIRE_FALSE(ArePointingToEqualData(one, two));
    REQUIRE_FALSE(ArePointingToEqualData(one, null));
  }
}
