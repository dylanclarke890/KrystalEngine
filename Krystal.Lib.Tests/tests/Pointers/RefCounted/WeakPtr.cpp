#include "Krystal.Lib/Pointers/RefCounted/WeakPtr.hpp"
#include "Krystal.Lib/Mixins/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  class TestWeakRefCounted : public RefCounted<TestWeakRefCounted>,
                             public CanMakeSingleThreadWeakPtr<TestWeakRefCounted>
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

    SingleThreadWeakPtr<TestWeakRefCounted> weakPtr(*obj);
    REQUIRE(obj->GetRefCount() == 1);

    obj->SubRef();
  }

  TEST_CASE("WeakPtr becomes null when object is destroyed", "[WeakPtr]")
  {
    SingleThreadWeakPtr<TestWeakRefCounted> weakPtr;

    {
      auto *obj = new TestWeakRefCounted();
      weakPtr = SingleThreadWeakPtr<TestWeakRefCounted>(*obj);

      REQUIRE(weakPtr);
      REQUIRE(weakPtr.get() == obj);

      obj->SubRef();
    }

    REQUIRE(!weakPtr);
    REQUIRE(weakPtr.get() == nullptr);
  }

  TEST_CASE("WeakPtr can be created from impl", "[WeakPtr]")
  {
    auto *obj = new TestWeakRefCounted();

    RefPtr<SingleThreadWeakPtrImpl> impl = ShareRefPtr(&obj->WeakImpl());
    SingleThreadWeakPtr<TestWeakRefCounted> weakPtr(Krys::Move(impl));

    REQUIRE(impl.get() == nullptr);
    REQUIRE(weakPtr.get() == obj);

    obj->SubRef();
  }

  TEST_CASE("WeakPtr can be moved", "[WeakPtr]")
  {
    auto *obj = new TestWeakRefCounted();

    SingleThreadWeakPtr<TestWeakRefCounted> weakPtr1(*obj);
    SingleThreadWeakPtr<TestWeakRefCounted> weakPtr2(Krys::Move(weakPtr1));

    REQUIRE(!weakPtr1);
    REQUIRE(weakPtr2);
    REQUIRE(weakPtr2.get() == obj);

    obj->SubRef();
  }

  TEST_CASE("WeakPtr copy shares invalidation state", "[WeakPtr]")
  {
    SingleThreadWeakPtr<TestWeakRefCounted> w1;
    SingleThreadWeakPtr<TestWeakRefCounted> w2;

    {
      auto *obj = new TestWeakRefCounted();
      w1 = SingleThreadWeakPtr<TestWeakRefCounted>(*obj);
      w2 = w1;

      REQUIRE(w1);
      REQUIRE(w2);

      obj->SubRef();
    }

    REQUIRE(!w1);
    REQUIRE(!w2);
  }

  TEST_CASE("WeakPtr remains valid while any strong ref exists", "[WeakPtr]")
  {
    SingleThreadWeakPtr<TestWeakRefCounted> weakPtr;

    {
      auto *obj = new TestWeakRefCounted();
      Ref<TestWeakRefCounted> ref = AdoptRef(*obj);
      weakPtr = SingleThreadWeakPtr<TestWeakRefCounted>(*obj);

      {
        Ref<TestWeakRefCounted> copy = ref;
        REQUIRE(weakPtr);
      }

      REQUIRE(weakPtr);
    }

    REQUIRE(!weakPtr);
  }
}
