#include "Krystal.Lib/Pointers/WeakPtr.hpp"
#include "Krystal.Lib/Mixins/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  class TestWeakRefCounted : public RefCounted<TestWeakRefCounted>, public CanMakeWeakPtr<TestWeakRefCounted>
  {
  public:
    TestWeakRefCounted() = default;
    virtual ~TestWeakRefCounted() = default;
  };

  struct TestWeakRefObject : public TestWeakRefCounted
  {
    bool *Deleted;

    TestWeakRefObject(bool *ptr) : Deleted(ptr)
    {
    }

    ~TestWeakRefObject() override
    {
      *Deleted = true;
    }
  };

  TEST_CASE("WeakPtr does not affect refcount", "[WeakPtr]")
  {
    auto *obj = new TestWeakRefCounted();
    REQUIRE(obj->GetRefCount() == 1);

    WeakPtr<TestWeakRefCounted> weakPtr = CreateWeakPtr(obj);
    REQUIRE(obj->GetRefCount() == 1);

    obj->SubRef();
  }

  TEST_CASE("WeakPtr becomes null when object is destroyed", "[WeakPtr]")
  {
    WeakPtr<TestWeakRefCounted> weakPtr;

    {
      auto *obj = new TestWeakRefCounted();
      weakPtr = CreateWeakPtr(obj);

      REQUIRE(weakPtr);
      REQUIRE(weakPtr.get() == obj);

      obj->SubRef();
    }

    REQUIRE(!weakPtr);
    REQUIRE(weakPtr.lock() == nullptr);
  }

  TEST_CASE("WeakPtr can be created from impl", "[WeakPtr]")
  {
    auto *obj = new TestWeakRefCounted();

    RefPtr<WeakPtrImpl> impl = ShareRefPtr(&obj->WeakImpl());
    WeakPtr<TestWeakRefCounted> weakPtr(Krys::Move(impl));

    REQUIRE(impl.get() == nullptr);
    REQUIRE(weakPtr.get() == obj);

    obj->SubRef();
  }

  TEST_CASE("WeakPtr can be moved", "[WeakPtr]")
  {
    auto *obj = new TestWeakRefCounted();

    WeakPtr<TestWeakRefCounted> weakPtr1 = CreateWeakPtr(obj);
    WeakPtr<TestWeakRefCounted> weakPtr2(Krys::Move(weakPtr1));

    REQUIRE(!weakPtr1);
    REQUIRE(weakPtr2.lock());
    REQUIRE(weakPtr2.get() == obj);

    obj->SubRef();
  }

  TEST_CASE("WeakPtr copy shares invalidation state", "[WeakPtr]")
  {
    WeakPtr<TestWeakRefCounted> w1;
    WeakPtr<TestWeakRefCounted> w2;

    {
      auto *obj = new TestWeakRefCounted();
      w1 = CreateWeakPtr(obj);
      w2 = w1;

      REQUIRE(w1);
      REQUIRE(w2);

      obj->SubRef();
    }

    REQUIRE(!w1.lock());
    REQUIRE(!w2.lock());
  }

  TEST_CASE("WeakPtr remains valid while any strong ref exists", "[WeakPtr]")
  {
    WeakPtr<TestWeakRefCounted> weakPtr;

    {
      auto *obj = new TestWeakRefCounted();
      Ref<TestWeakRefCounted> ref = AdoptRef(*obj);
      weakPtr = CreateWeakPtr(obj);

      {
        Ref<TestWeakRefCounted> copy = ref;
        REQUIRE(weakPtr.lock());
      }

      REQUIRE(weakPtr.lock());
    }

    REQUIRE(!weakPtr.lock());
  }
}
