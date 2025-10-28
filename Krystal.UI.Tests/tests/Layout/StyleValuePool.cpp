#include "Krystal.UI/Styles/Values/StyleValuePool.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("StyleValuePool::undefined_at_init", "[StyleValuePool]")
  {
    StyleValuePool pool;
    StyleValueHandle handle;

    REQUIRE(handle.IsUndefined());
    REQUIRE(!handle.IsDefined());
    REQUIRE(pool.GetLength(handle) == StyleLength::Undefined());
    REQUIRE(pool.GetNumber(handle) == NullableFloat {});
  }

  TEST_CASE("StyleValuePool::auto_at_init", "[StyleValuePool]")
  {
    StyleValuePool pool;
    auto handle = StyleValueHandle::Auto();

    REQUIRE(handle.IsAuto());
    REQUIRE(pool.GetLength(handle) == StyleLength::Auto());
  }

  TEST_CASE("StyleValuePool::store_small_int_points", "[StyleValuePool]")
  {
    StyleValuePool pool;
    StyleValueHandle handle;

    pool.Store(handle, StyleLength::Points(10));

    REQUIRE(pool.GetLength(handle) == StyleLength::Points(10));
  }

  TEST_CASE("StyleValuePool::store_small_negative_int_points", "[StyleValuePool]")
  {
    StyleValuePool pool;
    StyleValueHandle handle;

    pool.Store(handle, StyleLength::Points(-10));

    REQUIRE(pool.GetLength(handle) == StyleLength::Points(-10));
  }

  TEST_CASE("StyleValuePool::store_small_int_percent", "[StyleValuePool]")
  {
    StyleValuePool pool;
    StyleValueHandle handle;

    pool.Store(handle, StyleLength::Percent(10));

    REQUIRE(pool.GetLength(handle) == StyleLength::Percent(10));
  }

  TEST_CASE("StyleValuePool::store_large_int_percent", "[StyleValuePool]")
  {
    StyleValuePool pool;
    StyleValueHandle handle;

    pool.Store(handle, StyleLength::Percent(262'144));

    REQUIRE(pool.GetLength(handle) == StyleLength::Percent(262'144));
  }

  TEST_CASE("StyleValuePool::store_large_int_after_small_int", "[StyleValuePool]")
  {
    StyleValuePool pool;
    StyleValueHandle handle;

    pool.Store(handle, StyleLength::Percent(10));
    pool.Store(handle, StyleLength::Percent(262'144));

    REQUIRE(pool.GetLength(handle) == StyleLength::Percent(262'144));
  }

  TEST_CASE("StyleValuePool::store_small_int_after_large_int", "[StyleValuePool]")
  {
    StyleValuePool pool;
    StyleValueHandle handle;

    pool.Store(handle, StyleLength::Percent(262'144));
    pool.Store(handle, StyleLength::Percent(10));

    REQUIRE(pool.GetLength(handle) == StyleLength::Percent(10));
  }

  TEST_CASE("StyleValuePool::store_small_int_number", "[StyleValuePool]")
  {
    StyleValuePool pool;
    StyleValueHandle handle;

    pool.Store(handle, NullableFloat {10.0f});

    REQUIRE(pool.GetNumber(handle) == NullableFloat {10.0f});
  }

  TEST_CASE("StyleValuePool::store_undefined", "[StyleValuePool]")
  {
    StyleValuePool pool;
    StyleValueHandle handle;

    pool.Store(handle, StyleLength::Undefined());

    REQUIRE(handle.IsUndefined());
    REQUIRE(!handle.IsDefined());
    REQUIRE(pool.GetLength(handle) == StyleLength::Undefined());
  }

  TEST_CASE("StyleValuePool::store_undefined_after_small_int", "[StyleValuePool]")
  {
    StyleValuePool pool;
    StyleValueHandle handle;

    pool.Store(handle, StyleLength::Points(10));
    pool.Store(handle, StyleLength::Undefined());

    REQUIRE(handle.IsUndefined());
    REQUIRE(!handle.IsDefined());
    REQUIRE(pool.GetLength(handle) == StyleLength::Undefined());
  }

  TEST_CASE("StyleValuePool::store_undefined_after_large_int", "[StyleValuePool]")
  {
    StyleValuePool pool;
    StyleValueHandle handle;

    pool.Store(handle, StyleLength::Points(262'144));
    pool.Store(handle, StyleLength::Undefined());

    REQUIRE(handle.IsUndefined());
    REQUIRE(!handle.IsDefined());
    REQUIRE(pool.GetLength(handle) == StyleLength::Undefined());
  }

  TEST_CASE("StyleValuePool::store_keywords", "[StyleValuePool]")
  {
    StyleValuePool pool;
    StyleValueHandle handleMaxContent;
    StyleValueHandle handleFitContent;
    StyleValueHandle handleStretch;

    pool.Store(handleMaxContent, StyleSizeLength::MaxContent());
    pool.Store(handleFitContent, StyleSizeLength::FitContent());
    pool.Store(handleStretch, StyleSizeLength::Stretch());

    REQUIRE(pool.GetSize(handleMaxContent) == StyleSizeLength::MaxContent());
    REQUIRE(pool.GetSize(handleFitContent) == StyleSizeLength::FitContent());
    REQUIRE(pool.GetSize(handleStretch) == StyleSizeLength::Stretch());
  }

}