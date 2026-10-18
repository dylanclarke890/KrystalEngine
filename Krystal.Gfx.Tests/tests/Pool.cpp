#include "Krystal.Gfx/Pool.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Gfx/Handles.hpp"

namespace krys::gfx::tests
{
  TEST_CASE("Handle: packing", "[Gfx][Handle]")
  {
    STATIC_CHECK_FALSE(BufferHandle {}.IsValid());

    constexpr auto first = BufferHandle::Create(0u, 0u);
    STATIC_CHECK(first.IsValid());
    STATIC_CHECK(first.Index() == 0u);
    STATIC_CHECK(first.Generation() == 0u);

    constexpr auto last = BufferHandle::Create(BufferHandle::MaxIndex, 255u);
    STATIC_CHECK(last.Index() == BufferHandle::MaxIndex);
    STATIC_CHECK(last.Generation() == 255u);
    STATIC_CHECK(first != last);
  }

  TEST_CASE("Pool: add, get and remove", "[Gfx][Pool]")
  {
    Pool<string, BufferHandle> pool;
    CHECK(pool.TryGet(BufferHandle {}) == nullptr);

    auto a = pool.Add("a");
    auto b = pool.Add("b");
    CHECK(pool.Count() == 2u);
    CHECK(pool.Get(a) == "a");
    CHECK(*pool.TryGet(b) == "b");

    auto removed = pool.Remove(a);
    REQUIRE(removed.has_value());
    CHECK(*removed == "a");
    CHECK(pool.Count() == 1u);
    CHECK(pool.TryGet(a) == nullptr);
    CHECK_FALSE(pool.Contains(a));
    CHECK_FALSE(pool.Remove(a).has_value());

    // The slot is reused with a new generation, so the old handle stays stale.
    auto c = pool.Add("c");
    CHECK(c.Index() == a.Index());
    CHECK(c.Generation() == a.Generation() + 1u);
    CHECK(pool.TryGet(a) == nullptr);
    CHECK(pool.Get(c) == "c");
  }

  TEST_CASE("Pool: iteration and clearing", "[Gfx][Pool]")
  {
    Pool<int, TextureHandle> pool;
    auto a = pool.Add(1);
    auto b = pool.Add(2);
    auto c = pool.Add(3);
    static_cast<void>(pool.Remove(b));

    int sum = 0;
    uint32 visited = 0u;
    pool.ForEach(
      [&](TextureHandle handle, int value)
      {
        CHECK(pool.Contains(handle));
        sum += value;
        ++visited;
      });
    CHECK(visited == 2u);
    CHECK(sum == 4);

    pool.Clear();
    CHECK(pool.Count() == 0u);
    CHECK(pool.TryGet(a) == nullptr);
    CHECK(pool.TryGet(c) == nullptr);
    CHECK(pool.Add(5).IsValid());
  }
}
