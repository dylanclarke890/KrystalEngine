#include "Krystal.Core/Hashing/FastHash.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Hashing/StableHash.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <optional>
#include <string>
#include <variant>

namespace krys::tests
{
  namespace
  {
    struct HashedByMember
    {
      size_t Identifier;

      KRYS_NODISCARD size_t Hash() const noexcept
      {
        return Identifier;
      }
    };
  }

  TEST_CASE("CombineFastHashes: combines into an existing seed", "[Hashing]")
  {
    size_t seed = CombineFastHashes(1);
    const size_t original = seed;

    seed = CombineFastHashes(seed, 2);
    REQUIRE(seed == CombineFastHashes(original, 2));
    REQUIRE(seed != original);
  }

  TEST_CASE("CombineFastHashes: every argument contributes", "[Hashing]")
  {
    REQUIRE(CombineFastHashes(1, 2) != CombineFastHashes(1, 3));
    REQUIRE(CombineFastHashes(1, 2, 3) != CombineFastHashes(1, 2, 4));
    REQUIRE(CombineFastHashes(1, 2, 3) != CombineFastHashes(1, 2));
    REQUIRE(CombineFastHashes(1, 2) != CombineFastHashes(2, 1));
  }

  TEST_CASE("FastHash: integers and enumerations", "[Hashing]")
  {
    enum class Fruit : uint8
    {
      Apple,
      Pear,
    };

    CHECK(FastHash<int> {}(7) == FastHash<int> {}(7));
    CHECK(FastHash<int> {}(7) != FastHash<int> {}(8));
    CHECK(FastHash<Fruit> {}(Fruit::Apple) != FastHash<Fruit> {}(Fruit::Pear));
    // Small keys still spread over the whole hash, so tables can use any of its bits.
    CHECK((FastHash<uint32> {}(1) >> 32) != 0);
  }

  TEST_CASE("FastHash: floating-point numbers", "[Hashing]")
  {
    CHECK(FastHash<float> {}(0.0f) == FastHash<float> {}(-0.0f));
    CHECK(FastHash<double> {}(0.0) == FastHash<double> {}(-0.0));
    CHECK(FastHash<float> {}(1.5f) != FastHash<float> {}(2.5f));
  }

  TEST_CASE("FastHash: strings hash by content", "[Hashing]")
  {
    std::u8string owned = u8"krystal";
    CHECK(FastHash<std::u8string> {}(owned) == FastHash<std::u8string_view> {}(u8"krystal"));
    CHECK(FastHash<std::string> {}("a") != FastHash<std::string> {}("b"));
  }

  TEST_CASE("FastHash: pointers and smart pointers hash by address", "[Hashing]")
  {
    auto owner = CreateUnique<int>(1);
    CHECK(FastHash<UniquePtr<int>> {}(owner) == FastHash<int *> {}(owner.get()));
    CHECK(FastHash<int *> {}(nullptr) == FastHash<std::nullptr_t> {}(nullptr));
  }

  TEST_CASE("FastHash: pairs, tuples, optionals and variants", "[Hashing]")
  {
    CHECK(FastHash<std::pair<int, int>> {}({1, 2}) == CombineFastHashes(1, 2));
    CHECK(FastHash<std::tuple<int, int, int>> {}({1, 2, 3}) == CombineFastHashes(1, 2, 3));
    CHECK(FastHash<std::optional<int>> {}(std::nullopt) != FastHash<std::optional<int>> {}(0));
    CHECK(FastHash<std::variant<int, uint32>> {}(1) != FastHash<std::variant<int, uint32>> {}(1u));
  }

  TEST_CASE("FastHash: types with a Hash member", "[Hashing]")
  {
    CHECK(FastHash<HashedByMember> {}({1}) == FastHash<HashedByMember> {}({1}));
    CHECK(FastHash<HashedByMember> {}({1}) != FastHash<HashedByMember> {}({2}));
    // The member's result is mixed, since it may be as simple as an identifier.
    CHECK(FastHash<HashedByMember> {}({1}) != 1);
  }

  TEST_CASE("Fnv1a32: matches the reference values", "[Hashing]")
  {
    STATIC_CHECK(Fnv1a32(stringview {}) == 0x81'1C'9D'C5u);
    STATIC_CHECK(Fnv1a32(stringview {"a"}) == 0xE4'0C'29'2Cu);
    STATIC_CHECK(Fnv1a32(utf8_stringview {u8"foobar"}) == 0xBF'9C'F9'68u);
  }
}
