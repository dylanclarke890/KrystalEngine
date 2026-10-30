#include "Krystal.Core/Types/Packed.hpp"
#include "catch2/catch_test_macros.hpp"

#include <array>
#include <memory>

namespace krys::tests
{
  TEST_CASE("Packed stores a value without alignment", "[Packed]")
  {
    STATIC_REQUIRE(sizeof(Packed<uint32>) == sizeof(uint32));
    STATIC_REQUIRE(alignof(Packed<uint32>) == 1);

    Packed<uint32> value {0x12'34'56'78u};
    REQUIRE(value.get() == 0x12'34'56'78u);

    value = 7u;
    REQUIRE(value.get() == 7u);
    REQUIRE(value.exchange(9u) == 7u);
    REQUIRE(value.get() == 9u);

    Packed<uint32> other {3u};
    value.swap(other);
    REQUIRE(value.get() == 3u);
    REQUIRE(other.get() == 9u);

    uint32 plain = 5u;
    value.swap(plain);
    REQUIRE(value.get() == 5u);
    REQUIRE(plain == 3u);

    REQUIRE(Packed<uint32> {}.get() == 0u);
  }

  TEST_CASE("PackedPtr stores a pointer in fewer bytes", "[Packed]")
  {
    STATIC_REQUIRE(sizeof(PackedPtr<int>) == KRYS_EFFECTIVE_ADDRESS_WIDTH / 8);
    STATIC_REQUIRE(alignof(PackedPtr<int>) == 1);

    std::array<int, 2> values {1, 2};

    PackedPtr<int> empty;
    REQUIRE(empty.get() == nullptr);
    REQUIRE(!empty);
    REQUIRE_FALSE(static_cast<bool>(empty));

    PackedPtr<int> pointer {&values[0]};
    REQUIRE(pointer.get() == &values[0]);
    REQUIRE(static_cast<bool>(pointer));
    REQUIRE(*pointer == 1);
    REQUIRE(pointer == &values[0]);

    pointer = &values[1];
    REQUIRE(pointer.get() == &values[1]);
    REQUIRE(pointer.exchange(&values[0]) == &values[1]);
    REQUIRE(pointer.get() == &values[0]);

    PackedPtr<int> other {&values[1]};
    pointer.swap(other);
    REQUIRE(pointer.get() == &values[1]);
    REQUIRE(other.get() == &values[0]);
    REQUIRE_FALSE(pointer == other);

    pointer.swap(nullptr);
    REQUIRE(pointer.get() == nullptr);

    other.clear();
    REQUIRE(other.get() == nullptr);
  }

  TEST_CASE("PackedPtr round-trips heap and stack addresses", "[Packed]")
  {
    auto heap = std::make_unique<std::array<double, 4>>();
    for (auto &element : *heap)
    {
      PackedPtr<double> pointer {&element};
      REQUIRE(pointer.get() == &element);
    }

    struct Object
    {
      int Value {42};
    } object;

    PackedPtr<const Object> pointer {&object};
    REQUIRE(pointer->Value == 42);
  }

  TEST_CASE("PackedPtr has a hash table deleted value", "[Packed]")
  {
    PackedPtr<int> deleted {HashTableDeletedValueTag {}};
    REQUIRE(deleted.IsHashTableDeletedValue());

    int value = 0;
    PackedPtr<int> pointer {&value};
    REQUIRE_FALSE(pointer.IsHashTableDeletedValue());
  }
}
