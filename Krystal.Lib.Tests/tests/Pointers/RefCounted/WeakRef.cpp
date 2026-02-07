#include "Krystal.Lib/Pointers/RefCounted/WeakRef.hpp"
#include "Krystal.Lib.Tests/Pointers/RefCounted/TestRefCounted.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("WeakRef does not affect refcount", "[WeakRef]")
  {
    auto *obj = new TestWeakRefCounted();
    REQUIRE(obj->GetRefCount() == 1);

    SingleThreadWeakRef<TestWeakRefCounted> weakRef(*obj);
    REQUIRE(obj->GetRefCount() == 1);

    obj->SubRef();
  }

  TEST_CASE("WeakRef invalidates when object is destroyed", "[WeakRef]")
  {
    SingleThreadWeakRef<TestWeakRefCounted> *weakRef = nullptr;

    {
      auto *obj = new TestWeakRefCounted();
      weakRef = new SingleThreadWeakRef<TestWeakRefCounted>(*obj);

      REQUIRE(weakRef->impl());
      REQUIRE(weakRef->impl().template get<TestWeakRefObject>() == obj);

      obj->SubRef();
    }

    REQUIRE(!weakRef->impl());
    delete weakRef;
  }

  TEST_CASE("WeakRef can be moved", "[WeakRef]")
  {
    auto *obj = new TestWeakRefCounted();

    SingleThreadWeakRef<TestWeakRefCounted> weakRef1(*obj);
    SingleThreadWeakRef<TestWeakRefCounted> weakRef2(Krys::Move(weakRef1));

    REQUIRE(weakRef1.isHashTableEmptyValue());
    REQUIRE(weakRef2.impl());
    REQUIRE(weakRef2.impl().template get<TestWeakRefObject>() == obj);

    obj->SubRef();
  }

  TEST_CASE("WeakRef can be created from impl", "[WeakRef]")
  {
    auto *obj = new TestWeakRefCounted();

    Ref<SingleThreadWeakPtrImpl> impl = obj->weakImpl();
    SingleThreadWeakRef<TestWeakRefCounted> weakRef(Krys::Move(impl));

    REQUIRE(impl.isHashTableEmptyValue());
    REQUIRE(weakRef.impl());
    REQUIRE(weakRef.impl().template get<TestWeakRefObject>() == obj);

    obj->SubRef();
  }

  TEST_CASE("WeakRef copy shares invalidation state", "[WeakRef]")
  {
    SingleThreadWeakRef<TestWeakRefCounted> *w1 = nullptr;
    SingleThreadWeakRef<TestWeakRefCounted> *w2 = nullptr;

    {
      auto *obj = new TestWeakRefCounted();
      w1 = new SingleThreadWeakRef<TestWeakRefCounted>(*obj);
      w2 = new SingleThreadWeakRef<TestWeakRefCounted>(*w1);

      REQUIRE(w1->impl());
      REQUIRE(w2->impl());

      obj->SubRef();
    }

    REQUIRE(!w1->impl());
    REQUIRE(!w2->impl());

    delete w1;
    delete w2;
  }

  TEST_CASE("WeakRef remains valid while any strong ref exists", "[WeakRef]")
  {
    SingleThreadWeakRef<TestWeakRefCounted> *weakRef = nullptr;

    {
      auto *obj = new TestWeakRefCounted();
      Ref<TestWeakRefCounted> ref = adoptRef(*obj);
      weakRef = new SingleThreadWeakRef<TestWeakRefCounted>(*obj);

      {
        Ref<TestWeakRefCounted> copy = ref;
        REQUIRE(weakRef->impl());
      }

      REQUIRE(weakRef->impl());
    }

    REQUIRE(!weakRef->impl());
    delete weakRef;
  }
}