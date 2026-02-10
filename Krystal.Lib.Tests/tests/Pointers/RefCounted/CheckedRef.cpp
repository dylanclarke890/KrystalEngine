#include "Krystal.Lib/Pointers/RefCounted/CheckedPtr.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  class TestCheckedRefCounted : public CanMakeCheckedPtr<TestCheckedRefCounted>
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TestCheckedRefCounted);

  public:
    TestCheckedRefCounted() = default;
    virtual ~TestCheckedRefCounted() = default;
  };

  TEST_CASE("CheckedRef construction increments count", "[CheckedRef]")
  {
    auto *obj = new TestCheckedRefCounted();
    REQUIRE(obj->CheckedPtrCount() == 0);

    {
      CheckedRef<TestCheckedRefCounted> ref = ShareCheckedRef(*obj);
      REQUIRE(obj->CheckedPtrCount() == 1);
      REQUIRE(ref.get() == obj);
    }

    REQUIRE(obj->CheckedPtrCount() == 0);
    delete obj;
  }

  TEST_CASE("CheckedRef copy construction increments count", "[CheckedRef]")
  {
    auto *obj = new TestCheckedRefCounted();

    {
      CheckedRef<TestCheckedRefCounted> r1 = ShareCheckedRef(*obj);
      REQUIRE(obj->CheckedPtrCount() == 1);

      {
        CheckedRef<TestCheckedRefCounted> r2(r1);
        REQUIRE(obj->CheckedPtrCount() == 2);
        REQUIRE(r2.get() == obj);
      }

      REQUIRE(obj->CheckedPtrCount() == 1);
    }
    delete obj;
  }

  TEST_CASE("CheckedRef move construction transfers ownership", "[CheckedRef]")
  {
    auto *obj = new TestCheckedRefCounted();

    {
      CheckedRef<TestCheckedRefCounted> r1 = ShareCheckedRef(*obj);
      REQUIRE(obj->CheckedPtrCount() == 1);

      CheckedRef<TestCheckedRefCounted> r2(std::move(r1));
      REQUIRE(obj->CheckedPtrCount() == 1);
      REQUIRE(r2.get() == obj);
    }

    delete obj;
  }

  TEST_CASE("CheckedRef assignment updates counts correctly", "[CheckedRef]")
  {
    auto *a = new TestCheckedRefCounted();
    auto *b = new TestCheckedRefCounted();

    {
      CheckedRef<TestCheckedRefCounted> ref = ShareCheckedRef(*a);
      REQUIRE(a->CheckedPtrCount() == 1);
      REQUIRE(b->CheckedPtrCount() == 0);

      ref = ShareCheckedRef(*b);

      REQUIRE(a->CheckedPtrCount() == 0);
      REQUIRE(b->CheckedPtrCount() == 1);
      REQUIRE(ref.get() == b);
    }

    delete a;
    delete b;
  }

  TEST_CASE("CheckedRef equality comparisons work", "[CheckedRef]")
  {
    auto *a = new TestCheckedRefCounted();
    auto *b = new TestCheckedRefCounted();

    {
      CheckedRef<TestCheckedRefCounted> ra = ShareCheckedRef(*a);
      CheckedRef<TestCheckedRefCounted> ra2 = ShareCheckedRef(*a);
      CheckedRef<TestCheckedRefCounted> rb = ShareCheckedRef(*b);

      REQUIRE(ra.get() == a);
      REQUIRE(ra2.get() == a);
      REQUIRE(rb.get() == b);
      REQUIRE(ra.get() == ra2.get());
      REQUIRE(ra.get() != rb.get());
    }

    delete a;
    delete b;
  }
}
