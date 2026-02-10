#include "Krystal.Lib/Pointers/RefCounted/CheckedPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CheckedRef.hpp"
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

  TEST_CASE("CheckedPtr default construction is null", "[CheckedPtr]")
  {
    CheckedPtr<TestCheckedRefCounted> ptr;
    REQUIRE_FALSE(ptr);
    REQUIRE(ptr.get() == nullptr);
  }

  TEST_CASE("CheckedPtr increments and decrements count on scope exit", "[CheckedPtr]")
  {
    auto *obj = new TestCheckedRefCounted();
    REQUIRE(obj->CheckedPtrCount() == 0);

    {
      CheckedPtr<TestCheckedRefCounted> ptr = ShareCheckedPtr(obj);
      REQUIRE(ptr);
      REQUIRE(obj->CheckedPtrCount() == 1);
    }

    REQUIRE(obj->CheckedPtrCount() == 0);
    delete obj;
  }

  TEST_CASE("CheckedPtr copy construction increments count", "[CheckedPtr]")
  {
    auto *obj = new TestCheckedRefCounted();

    {
      CheckedPtr<TestCheckedRefCounted> p1 = ShareCheckedPtr(obj);
      REQUIRE(obj->CheckedPtrCount() == 1);

      {
        CheckedPtr<TestCheckedRefCounted> p2(p1);
        REQUIRE(obj->CheckedPtrCount() == 2);
        REQUIRE(p2.get() == obj);
      }

      REQUIRE(obj->CheckedPtrCount() == 1);
    }

    delete obj;
  }

  TEST_CASE("CheckedPtr move construction does not increment count", "[CheckedPtr]")
  {
    auto *obj = new TestCheckedRefCounted();

    {
      CheckedPtr<TestCheckedRefCounted> p1 = ShareCheckedPtr(obj);
      REQUIRE(obj->CheckedPtrCount() == 1);

      CheckedPtr<TestCheckedRefCounted> p2(std::move(p1));
      REQUIRE(obj->CheckedPtrCount() == 1);
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
      CheckedPtr<TestCheckedRefCounted> ptr = ShareCheckedPtr(a);
      REQUIRE(a->CheckedPtrCount() == 1);
      REQUIRE(b->CheckedPtrCount() == 0);

      ptr = ShareCheckedPtr(b);

      REQUIRE(a->CheckedPtrCount() == 0);
      REQUIRE(b->CheckedPtrCount() == 1);
      REQUIRE(ptr.get() == b);
    }

    delete a;
    delete b;
  }

  TEST_CASE("CheckedPtr assignment to nullptr decrements count", "[CheckedPtr]")
  {
    auto *obj = new TestCheckedRefCounted();

    {
      CheckedPtr<TestCheckedRefCounted> ptr = ShareCheckedPtr(obj);
      REQUIRE(obj->CheckedPtrCount() == 1);

      ptr = nullptr;

      REQUIRE(obj->CheckedPtrCount() == 0);
      REQUIRE_FALSE(ptr);
    }

    delete obj;
  }

  TEST_CASE("CheckedPtr comparisons work as expected", "[CheckedPtr]")
  {
    auto *a = new TestCheckedRefCounted();
    auto *b = new TestCheckedRefCounted();

    {
      CheckedPtr<TestCheckedRefCounted> pa = ShareCheckedPtr(a);
      CheckedPtr<TestCheckedRefCounted> pa2 = ShareCheckedPtr(a);
      CheckedPtr<TestCheckedRefCounted> pb = ShareCheckedPtr(b);

      REQUIRE(pa == a);
      REQUIRE(pa == pa2);
      REQUIRE(pa != pb);
      REQUIRE(pb == b);
    }

    delete a;
    delete b;
  }
}
