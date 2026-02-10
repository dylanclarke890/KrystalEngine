#include "Krystal.Lib/Pointers/RefCounted/RefPtr.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Pointers/RefCounted/Ref.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefCounted.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  class TestRefCounted : public RefCounted<TestRefCounted>
  {
  public:
    TestRefCounted() = default;
    virtual ~TestRefCounted() = default;
  };

  struct TestObject : TestRefCounted
  {
    bool *Deleted;

    TestObject(bool *ptr) : Deleted(ptr)
    {
    }

    ~TestObject() override
    {
      *Deleted = true;
    }
  };

  TEST_CASE("RefPtr constructed from Ref increments refcount", "[RefPtr]")
  {
    auto *obj = new TestRefCounted();
    REQUIRE(obj->GetRefCount() == 1);

    {
      auto ref = AdoptRef(*obj);
      REQUIRE(obj->GetRefCount() == 1);

      {
        RefPtr<TestRefCounted> p1(ref);
        REQUIRE(obj->GetRefCount() == 2);
      }

      REQUIRE(obj->GetRefCount() == 1);
    }
  }

  TEST_CASE("Ref can be moved into RefPtr without refcount change", "[RefPtr]")
  {
    auto *obj = new TestRefCounted();

    {
      Ref<TestRefCounted> ref = AdoptRef(*obj);
      REQUIRE(obj->GetRefCount() == 1);

      RefPtr<TestRefCounted> ptr = std::move(ref);
      REQUIRE(obj->GetRefCount() == 1);
    }
  }

  TEST_CASE("RefPtr copy increments and decrements refcount", "[RefPtr]")
  {
    auto *obj = new TestRefCounted();

    {
      Ref<TestRefCounted> ref = AdoptRef(*obj);
      REQUIRE(obj->GetRefCount() == 1);

      RefPtr<TestRefCounted> p1(ref);
      REQUIRE(obj->GetRefCount() == 2);

      {
        RefPtr<TestRefCounted> p2 = p1;
        REQUIRE(obj->GetRefCount() == 3);
      }

      REQUIRE(obj->GetRefCount() == 2);
    }
  }

  TEST_CASE("RefPtr move does not change refcount", "[RefPtr]")
  {
    auto *obj = new TestRefCounted();

    {
      Ref<TestRefCounted> ref = AdoptRef(*obj);
      RefPtr<TestRefCounted> p1(ref);
      REQUIRE(obj->GetRefCount() == 2);

      RefPtr<TestRefCounted> p2 = std::move(p1);
      REQUIRE(obj->GetRefCount() == 2);
    }
  }

  TEST_CASE("RefPtr final release deletes object", "[RefPtr]")
  {
    bool deleted = false;
    auto *obj = new TestObject {&deleted};

    {
      Ref<TestObject> ref = AdoptRef(*obj);
      REQUIRE(obj->GetRefCount() == 1);

      RefPtr<TestObject> ptr(ref);
      REQUIRE(obj->GetRefCount() == 2);
    }

    REQUIRE(deleted);
  }

  TEST_CASE("RefPtr self-assignment is safe", "[RefPtr]")
  {
    auto *obj = new TestRefCounted();

    {
      Ref<TestRefCounted> ref = AdoptRef(*obj);
      RefPtr<TestRefCounted> p(ref);
      REQUIRE(obj->GetRefCount() == 2);

      p = p;
      REQUIRE(obj->GetRefCount() == 2);
    }
  }

  TEST_CASE("RefPtr move self-assignment is safe", "[RefPtr]")
  {
    auto *obj = new TestRefCounted();

    {
      Ref<TestRefCounted> ref = AdoptRef(*obj);
      RefPtr<TestRefCounted> p(ref);

      p = std::move(p);
      REQUIRE(obj->GetRefCount() == 2);
    }
  }

  TEST_CASE("Null RefPtr behaves correctly", "[RefPtr]")
  {
    RefPtr<TestRefCounted> p;
    REQUIRE(!p);

    RefPtr<TestRefCounted> p2 = p;
    REQUIRE(!p2);

    RefPtr<TestRefCounted> p3 = std::move(p2);
    REQUIRE(!p3);
  }
}