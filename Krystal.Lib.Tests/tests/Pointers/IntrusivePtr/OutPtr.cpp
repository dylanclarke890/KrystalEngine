#include "Krystal.Lib.Tests/Pointers/IntrusivePtr/Mocks.hpp"
#include "Krystal.Lib/Pointers/IntrusivePtr.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("IntrusivePtr - out_ptr basics")
  {
    instrumented_counted<> items[2] = {};
    auto c_func = [&](size_t idx, instrumented_counted<> **res)
    {
      CHECK(*res == nullptr);
      *res = &items[idx];
    };

    mock_ptr<instrumented_counted<>> p;

    c_func(0, std::out_ptr(p));
    CHECK(p.get() == &items[0]);
    CHECK(items[0].count == 1);

    c_func(1, std::out_ptr(p));
    CHECK(p.get() == &items[1]);
    CHECK(items[0].count == -1);
    CHECK(items[1].count == 1);
  }

  TEST_CASE("IntrusivePtr - out_ptr void_pp")
  {
    instrumented_counted<> items[2] = {};
    auto c_func = [&](size_t idx, void **res)
    {
      CHECK(*res == nullptr);
      *res = &items[idx];
    };

    mock_ptr<instrumented_counted<>> p;

    c_func(0, std::out_ptr(p));
    CHECK(p.get() == &items[0]);
    CHECK(items[0].count == 1);

    c_func(1, std::out_ptr(p));
    CHECK(p.get() == &items[1]);
    CHECK(items[0].count == -1);
    CHECK(items[1].count == 1);
  }

  TEST_CASE("IntrusivePtr - inout_ptr basics")
  {
    instrumented_counted<> items[2] = {};
    auto c_func = [&](size_t old, size_t idx, instrumented_counted<> **res)
    {
      if (old == size_t(-1))
        CHECK(*res == nullptr);
      else
        CHECK(*res == &items[old]);
      if (*res)
        mock_traits<>::SubRef(*res);
      *res = &items[idx];
    };

    mock_ptr<instrumented_counted<>> p;

    c_func(size_t(-1), 0, std::inout_ptr(p));
    CHECK(p.get() == &items[0]);
    CHECK(items[0].count == 1);

    c_func(0, 1, std::inout_ptr(p));
    CHECK(p.get() == &items[1]);
    CHECK(items[0].count == -1);
    CHECK(items[1].count == 1);
  }

  TEST_CASE("IntrusivePtr - inout_ptr void_pp")
  {
    instrumented_counted<> items[2] = {};
    auto c_func = [&](size_t old, size_t idx, void **res)
    {
      auto ptr = (instrumented_counted<> **)(res);
      if (old == size_t(-1))
        CHECK(*ptr == nullptr);
      else
        CHECK(*ptr == &items[old]);
      if (*ptr)
        mock_traits<>::SubRef(*ptr);
      *ptr = &items[idx];
    };

    mock_ptr<instrumented_counted<>> p;

    c_func(size_t(-1), 0, std::inout_ptr(p));
    CHECK(p.get() == &items[0]);
    CHECK(items[0].count == 1);

    c_func(0, 1, std::inout_ptr(p));
    CHECK(p.get() == &items[1]);
    CHECK(items[0].count == -1);
    CHECK(items[1].count == 1);
  }
}
