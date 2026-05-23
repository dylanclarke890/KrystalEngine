#include "Krystal.Lib/Ranges/Reconstruct.hpp"
#include "Krystal.Lib/Ranges/TypeTraits.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include <catch_all.hpp>

#include <cstddef>
#include <string_view>
#include <type_traits>

namespace
{
  template <typename T>
  struct PtrRange
  {
    T *b {};
    T *e {};

    T *begin() noexcept
    {
      return b;
    }
    T *end() noexcept
    {
      return e;
    }

    const T *begin() const noexcept
    {
      return b;
    }
    const T *end() const noexcept
    {
      return e;
    }
  };
}

TEST_CASE("Ranges::reconstruct - Span from pointer iterator + pointer sentinel (sized sentinel path)")
{
  int a[] = {1, 2, 3, 4, 5};

  auto s = Krys::Ranges::reconstruct(std::in_place_type<Krys::Span<int>>, a + 1, a + 4);

  REQUIRE(s.data() == a + 1);
  REQUIRE(s.size() == 3);
  REQUIRE(s[0] == 2);
  REQUIRE(s[1] == 3);
  REQUIRE(s[2] == 4);

  STATIC_REQUIRE(noexcept(Krys::Ranges::reconstruct(std::in_place_type<Krys::Span<int>>, a + 1, a + 4)));
}

TEST_CASE("Ranges::reconstruct - Span from pointer iterator + size_t sentinel (count sentinel path)")
{
  int a[] = {1, 2, 3, 4, 5};

  auto s = Krys::Ranges::reconstruct(std::in_place_type<Krys::Span<int>>, a + 2, std::size_t {2});

  REQUIRE(s.data() == a + 2);
  REQUIRE(s.size() == 2);
  REQUIRE(s[0] == 3);
  REQUIRE(s[1] == 4);

  STATIC_REQUIRE(
    noexcept(Krys::Ranges::reconstruct(std::in_place_type<Krys::Span<int>>, a + 2, std::size_t {2})));
}

TEST_CASE("Ranges::reconstruct - Span from a range (range -> begin/end -> iterator/sentinel)")
{
  int a[] = {10, 20, 30, 40, 50};

  PtrRange<int> r {a + 1, a + 4}; // {20,30,40}

  auto s = Krys::Ranges::reconstruct(std::in_place_type<Krys::Span<int>>, r);

  REQUIRE(s.data() == a + 1);
  REQUIRE(s.size() == 3);
  REQUIRE(s[0] == 20);
  REQUIRE(s[2] == 40);
}

TEST_CASE("Ranges::const_reconstruct - Span<const T> from a const range")
{
  int a[] = {10, 20, 30, 40, 50};
  const PtrRange<int> r {a + 1, a + 4};

  auto cs = Krys::Ranges::const_reconstruct(std::in_place_type<Krys::Span<const int>>, r);

  // Exact type check (helps catch accidental fallback-to-subrange):
  STATIC_REQUIRE(std::is_same_v<decltype(cs), Krys::Span<const int>>);

  REQUIRE(cs.data() == a + 1);
  REQUIRE(cs.size() == 3);
  REQUIRE(cs[0] == 20);
  REQUIRE(cs[2] == 40);
}

TEST_CASE("Ranges::const_reconstruct - requesting Span<T> from const range falls back to subrange")
{
  int a[] = {10, 20, 30, 40, 50};
  const PtrRange<int> r {a + 1, a + 4};

  // Asking for Span<int> with const iterators should NOT be span-reconstructible.
  // Your CPO will still compile, but should fall back to subrange.
  auto res = Krys::Ranges::const_reconstruct(std::in_place_type<Krys::Span<int>>, r);

  STATIC_REQUIRE(std::is_same_v<decltype(res), std::ranges::subrange<const int *, const int *>>);

  REQUIRE(*res.begin() == 20);
  REQUIRE(*(res.end() - 1) == 40);
}

TEST_CASE("Ranges::reconstruct - std::string_view from pointer iterator + pointer sentinel")
{
  constexpr char text[] = "abcdef";

  auto sv = Krys::Ranges::reconstruct(std::in_place_type<std::string_view>, text + 1, text + 4);

  REQUIRE(sv == "bcd");
  REQUIRE(sv.data() == text + 1);
  REQUIRE(sv.size() == 3);
}

TEST_CASE("Ranges::reconstruct - std::string_view from pointer iterator + size_t sentinel")
{
  constexpr char text[] = "abcdef";

  auto sv = Krys::Ranges::reconstruct(std::in_place_type<std::string_view>, text + 2, std::size_t {3});

  REQUIRE(sv == "cde");
  REQUIRE(sv.data() == text + 2);
  REQUIRE(sv.size() == 3);
}

TEST_CASE("Ranges::const_reconstruct - std::string_view from a string_view range")
{
  std::string_view in = "hello";

  auto out = Krys::Ranges::const_reconstruct(std::in_place_type<std::string_view>, in);

  STATIC_REQUIRE(std::is_same_v<decltype(out), std::string_view>);
  REQUIRE(out == in);
}

TEST_CASE("Ranges::reconstruct - tagless iterator+sentinel (no in_place) returns subrange fallback")
{
  int a[] = {1, 2, 3, 4, 5};

  auto sr = Krys::Ranges::reconstruct(a + 1, a + 4);

  STATIC_REQUIRE(std::is_same_v<decltype(sr), std::ranges::subrange<int *, int *>>);

  REQUIRE(*sr.begin() == 2);
  REQUIRE(*(sr.end() - 1) == 4);
  REQUIRE((sr.end() - sr.begin()) == 3);
}
