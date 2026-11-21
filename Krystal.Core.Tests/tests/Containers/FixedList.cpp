#include "Krystal.Core/Containers/FixedList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Containers/RefCountedFixedList.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <algorithm>
#include <array>
#include <ranges>
#include <string>

namespace krys::tests
{
  namespace
  {
    struct NotDefaultConstructible
    {
      int Value;

      explicit NotDefaultConstructible(int value) noexcept : Value(value)
      {
      }

      KRYS_NODISCARD bool operator==(const NotDefaultConstructible &) const noexcept = default;
    };
  }

  TEST_CASE("FixedList: is one pointer, and null when empty", "[FixedList]")
  {
    STATIC_CHECK(sizeof(FixedList<std::string>) == sizeof(void *));

    const FixedList<int> empty;
    CHECK(empty.empty());
    CHECK(empty.size() == 0);
    CHECK(empty.data() == nullptr);
    CHECK(empty.begin() == empty.end());
    CHECK(FixedList<int>(0).empty());
  }

  TEST_CASE("FixedList: holds the elements it's made with", "[FixedList]")
  {
    FixedList<int> values {1, 2, 3};
    CHECK(values.size() == 3);
    CHECK_FALSE(values.empty());
    CHECK(values.front() == 1);
    CHECK(values.back() == 3);
    CHECK(values[1] == 2);

    values[1] = 5;
    CHECK(std::ranges::equal(values, std::array {1, 5, 3}));
    CHECK(std::ranges::equal(std::ranges::reverse_view(values), std::array {3, 5, 1}));

    CHECK(FixedList<int>(3) == FixedList<int> {0, 0, 0});
    CHECK(FixedList<int>(2, 7) == FixedList<int> {7, 7});
    CHECK(FixedList<int>(std::from_range, std::array {4, 5}) == FixedList<int> {4, 5});
  }

  TEST_CASE("FixedList: copies, moves and clears", "[FixedList]")
  {
    FixedList<std::string> original {"a", "b"};
    FixedList<std::string> copy = original;
    CHECK(copy == original);
    CHECK(copy.data() != original.data());

    FixedList<std::string> moved = krys::move(copy);
    CHECK(moved == original);
    CHECK(copy.empty());

    copy = moved;
    CHECK(copy == original);

    moved.clear();
    CHECK(moved.empty());
  }

  TEST_CASE("FixedList: is a contiguous range", "[FixedList]")
  {
    FixedList<int> values {1, 2, 3};
    Span<const int> view = values;
    CHECK(view.size() == 3);
    CHECK(view.data() == values.data());
  }

  TEST_CASE("FixedList: is built from ranges", "[FixedList]")
  {
    const std::array source {1, 2, 3};
    CHECK(FixedList<int>(std::from_range, source | std::views::transform([](int value) { return value * 2; }))
          == FixedList<int> {2, 4, 6});
    CHECK(FixedList<int>(std::from_range, std::views::iota(1, 4)) == FixedList<int> {1, 2, 3});

    // Elements are built in place, so they needn't be default-constructible.
    FixedList<NotDefaultConstructible> built(
      std::from_range,
      source | std::views::transform([](int value) { return NotDefaultConstructible(value); }));
    CHECK(built.back() == NotDefaultConstructible(3));
  }

  TEST_CASE("RefCountedFixedList: shares its elements", "[FixedList]")
  {
    auto list = RefCountedFixedList<int>::Create({1, 2, 3});
    RefPtr<RefCountedFixedList<int>> other = list;
    CHECK(other->data() == list->data());
    CHECK(list->front() == 1);
    CHECK(list->back() == 3);
    CHECK_FALSE(list->empty());
    CHECK(*RefCountedFixedList<int>::Create(std::from_range, std::array {1, 2})
          == *RefCountedFixedList<int>::Create({1, 2}));
  }
}
