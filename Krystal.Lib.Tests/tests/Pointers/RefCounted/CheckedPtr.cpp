#include "Krystal.Lib/Pointers/RefCounted/CheckedPtr.hpp"
#include "Krystal.Lib.Tests/Pointers/RefCounted/TestRefCounted.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("CheckedPtr default construction is null", "[CheckedPtr]")
  {
    CheckedPtr<TestCheckedRefCounted> ptr;
    REQUIRE_FALSE(ptr);
    REQUIRE(ptr.get() == nullptr);
  }

  TEST_CASE("CheckedPtr increments and decrements count on scope exit", "[CheckedPtr]")
  {
    auto *obj = new TestCheckedRefCounted();
    REQUIRE(obj->checkedPtrCount() == 0);

    {
      CheckedPtr<TestCheckedRefCounted> ptr(obj);
      REQUIRE(ptr);
      REQUIRE(obj->checkedPtrCount() == 1);
    }

    REQUIRE(obj->checkedPtrCount() == 0);
    delete obj;
  }

  TEST_CASE("CheckedPtr copy construction increments count", "[CheckedPtr]")
  {
    auto *obj = new TestCheckedRefCounted();

    {
      CheckedPtr<TestCheckedRefCounted> p1(obj);
      REQUIRE(obj->checkedPtrCount() == 1);

      {
        CheckedPtr<TestCheckedRefCounted> p2(p1);
        REQUIRE(obj->checkedPtrCount() == 2);
        REQUIRE(p2.get() == obj);
      }

      REQUIRE(obj->checkedPtrCount() == 1);
    }

    delete obj;
  }

  TEST_CASE("CheckedPtr move construction does not increment count", "[CheckedPtr]")
  {
    auto *obj = new TestCheckedRefCounted();

    {
      CheckedPtr<TestCheckedRefCounted> p1(obj);
      REQUIRE(obj->checkedPtrCount() == 1);

      CheckedPtr<TestCheckedRefCounted> p2(std::move(p1));
      REQUIRE(obj->checkedPtrCount() == 1);
      REQUIRE(p2.get() == obj);
      REQUIRE(p1.get() == nullptr);
    }

    delete obj;
  }

  TEST_CASE("CheckedPtr assignment updates counts correctly", "[CheckedPtr]")
  {
    auto *a = new TestCheckedRefCounted();
    auto *b = new TestCheckedRefCounted();

    {
      CheckedPtr<TestCheckedRefCounted> ptr(a);
      REQUIRE(a->checkedPtrCount() == 1);
      REQUIRE(b->checkedPtrCount() == 0);

      ptr = b;

      REQUIRE(a->checkedPtrCount() == 0);
      REQUIRE(b->checkedPtrCount() == 1);
      REQUIRE(ptr.get() == b);
    }

    delete a;
    delete b;
  }

  TEST_CASE("CheckedPtr assignment to nullptr decrements count", "[CheckedPtr]")
  {
    auto *obj = new TestCheckedRefCounted();

    {
      CheckedPtr<TestCheckedRefCounted> ptr(obj);
      REQUIRE(obj->checkedPtrCount() == 1);

      ptr = nullptr;

      REQUIRE(obj->checkedPtrCount() == 0);
      REQUIRE_FALSE(ptr);
    }

    delete obj;
  }

  TEST_CASE("CheckedPtr comparisons work as expected", "[CheckedPtr]")
  {
    auto *a = new TestCheckedRefCounted();
    auto *b = new TestCheckedRefCounted();

    {
      CheckedPtr<TestCheckedRefCounted> pa(a);
      CheckedPtr<TestCheckedRefCounted> pa2(a);
      CheckedPtr<TestCheckedRefCounted> pb(b);

      REQUIRE(pa == a);
      REQUIRE(pa == pa2);
      REQUIRE(pa != pb);
      REQUIRE(pb == b);
    }

    delete a;
    delete b;
  }
}
