#include "Krystal.Lib/Pointers/RefCounted/CheckedRef.hpp"
#include "Krystal.Lib.Tests/Pointers/RefCounted/TestRefCounted.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("CheckedRef construction increments count", "[CheckedRef]")
  {
    auto *obj = new TestCheckedRefCounted();
    REQUIRE(obj->checkedPtrCount() == 0);

    {
      CheckedRef<TestCheckedRefCounted> ref(*obj);
      REQUIRE(obj->checkedPtrCount() == 1);
      REQUIRE(&ref.get() == obj);
    }

    REQUIRE(obj->checkedPtrCount() == 0);
    delete obj;
  }

  TEST_CASE("CheckedRef copy construction increments count", "[CheckedRef]")
  {
    auto *obj = new TestCheckedRefCounted();

    {
      CheckedRef<TestCheckedRefCounted> r1(*obj);
      REQUIRE(obj->checkedPtrCount() == 1);

      {
        CheckedRef<TestCheckedRefCounted> r2(r1);
        REQUIRE(obj->checkedPtrCount() == 2);
        REQUIRE(&r2.get() == obj);
      }

      REQUIRE(obj->checkedPtrCount() == 1);
    }
    delete obj;
  }

  TEST_CASE("CheckedRef move construction transfers ownership", "[CheckedRef]")
  {
    auto *obj = new TestCheckedRefCounted();

    {
      CheckedRef<TestCheckedRefCounted> r1(*obj);
      REQUIRE(obj->checkedPtrCount() == 1);

      CheckedRef<TestCheckedRefCounted> r2(std::move(r1));
      REQUIRE(obj->checkedPtrCount() == 1);
      REQUIRE(&r2.get() == obj);
    }

    delete obj;
  }

  TEST_CASE("CheckedRef assignment updates counts correctly", "[CheckedRef]")
  {
    auto *a = new TestCheckedRefCounted();
    auto *b = new TestCheckedRefCounted();

    {
      CheckedRef<TestCheckedRefCounted> ref(*a);
      REQUIRE(a->checkedPtrCount() == 1);
      REQUIRE(b->checkedPtrCount() == 0);

      ref = *b;

      REQUIRE(a->checkedPtrCount() == 0);
      REQUIRE(b->checkedPtrCount() == 1);
      REQUIRE(&ref.get() == b);
    }

    delete a;
    delete b;
  }

  TEST_CASE("CheckedRef equality comparisons work", "[CheckedRef]")
  {
    auto *a = new TestCheckedRefCounted();
    auto *b = new TestCheckedRefCounted();

    {
      CheckedRef<TestCheckedRefCounted> ra(*a);
      CheckedRef<TestCheckedRefCounted> ra2(*a);
      CheckedRef<TestCheckedRefCounted> rb(*b);

      REQUIRE(&ra.get() == a);
      REQUIRE(&ra2.get() == a);
      REQUIRE(&rb.get() == b);
      REQUIRE(&ra.get() == &ra2.get());
      REQUIRE(&ra.get() != &rb.get());
    }

    delete a;
    delete b;
  }
}
