#include "Krystal.Lib/Pointers/WeakRef.hpp"
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

      REQUIRE(weakRef->Impl());
      REQUIRE(weakRef->Impl().template get<TestWeakRefObject>() == obj);

      obj->SubRef();
    }

    REQUIRE(!weakRef->Impl());
    delete weakRef;
  }

  TEST_CASE("WeakRef can be moved", "[WeakRef]")
  {
    auto *obj = new TestWeakRefCounted();

    SingleThreadWeakRef<TestWeakRefCounted> weakRef1(*obj);
    SingleThreadWeakRef<TestWeakRefCounted> weakRef2(Krys::Move(weakRef1));

    REQUIRE(weakRef1.get() == nullptr);
    REQUIRE(weakRef2.Impl());
    REQUIRE(weakRef2.Impl().template get<TestWeakRefObject>() == obj);

    obj->SubRef();
  }

  TEST_CASE("WeakRef can be created from impl", "[WeakRef]")
  {
    auto *obj = new TestWeakRefCounted();

    Ref<SingleThreadWeakPtrImpl> impl = ShareRef(obj->WeakImpl());
    SingleThreadWeakRef<TestWeakRefCounted> weakRef(Krys::Move(impl));

    REQUIRE(impl.get() == nullptr);
    REQUIRE(weakRef.Impl());
    REQUIRE(weakRef.Impl().template get<TestWeakRefObject>() == obj);

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

      REQUIRE(w1->Impl());
      REQUIRE(w2->Impl());

      obj->SubRef();
    }

    REQUIRE(!w1->Impl());
    REQUIRE(!w2->Impl());

    delete w1;
    delete w2;
  }

  TEST_CASE("WeakRef remains valid while any strong ref exists", "[WeakRef]")
  {
    SingleThreadWeakRef<TestWeakRefCounted> *weakRef = nullptr;

    {
      auto *obj = new TestWeakRefCounted();
      Ref<TestWeakRefCounted> ref = AdoptRef(*obj);
      weakRef = new SingleThreadWeakRef<TestWeakRefCounted>(*obj);

      {
        Ref<TestWeakRefCounted> copy = ref;
        REQUIRE(weakRef->Impl());
      }

      REQUIRE(weakRef->Impl());
    }

    REQUIRE(!weakRef->Impl());
    delete weakRef;
  }
}