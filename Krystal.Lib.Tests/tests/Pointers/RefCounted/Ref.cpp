#include "Krystal.Lib/Pointers/RefCounted/Ref.hpp"
#include "Krystal.Lib/Core/Move.hpp"
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

  TEST_CASE("Ref manages reference count through copy and move", "[Ref]")
  {
    auto *obj = new TestRefCounted();
    REQUIRE(obj->GetRefCount() == 1);

    {
      auto ref1 = AdoptRef(*obj);
      REQUIRE(obj->GetRefCount() == 1);

      {
        auto ref2 = ref1;
        REQUIRE(obj->GetRefCount() == 2);

        auto ref3 = Krys::Move(ref2);
        REQUIRE(obj->GetRefCount() == 2);
      }

      REQUIRE(obj->GetRefCount() == 1);
    }

    REQUIRE(obj->GetRefCountDebugger().DeletionHasBegun());
  }

  TEST_CASE("Ref copy increments and decrements refcount", "[Ref]")
  {
    auto *obj = new TestRefCounted();

    {
      Ref<TestRefCounted> ref1 = AdoptRef(*obj);
      REQUIRE(obj->GetRefCount() == 1);

      {
        Ref<TestRefCounted> ref2 = ref1;
        REQUIRE(obj->GetRefCount() == 2);
      }

      REQUIRE(obj->GetRefCount() == 1);
    }

    REQUIRE(obj->GetRefCountDebugger().DeletionHasBegun());
  }

  TEST_CASE("Ref move construction does not change refcount", "[Ref]")
  {
    auto *obj = new TestRefCounted();

    {
      Ref<TestRefCounted> ref1 = AdoptRef(*obj);
      REQUIRE(obj->GetRefCount() == 1);

      Ref<TestRefCounted> ref2 = Krys::Move(ref1);
      REQUIRE(obj->GetRefCount() == 1);
    }

    REQUIRE(obj->GetRefCountDebugger().DeletionHasBegun());
  }

  TEST_CASE("Ref copy assignment updates refcount correctly", "[Ref]")
  {
    auto *obj = new TestRefCounted();

    {
      Ref<TestRefCounted> ref1 = AdoptRef(*obj);
      REQUIRE(obj->GetRefCount() == 1);

      Ref<TestRefCounted> ref2 = ref1;
      REQUIRE(obj->GetRefCount() == 2);

      ref2 = ref1;
      REQUIRE(obj->GetRefCount() == 2);
    }

    REQUIRE(obj->GetRefCountDebugger().DeletionHasBegun());
  }

  TEST_CASE("Ref move assignment transfers ownership", "[Ref]")
  {
    auto *obj = new TestRefCounted();

    {
      Ref<TestRefCounted> ref1 = AdoptRef(*obj);
      REQUIRE(obj->GetRefCount() == 1);

      Ref<TestRefCounted> ref2 = Krys::Move(ref1);
      REQUIRE(obj->GetRefCount() == 1);

      Ref<TestRefCounted> ref3 = Krys::Move(ref2);
      REQUIRE(obj->GetRefCount() == 1);
    }

    REQUIRE(obj->GetRefCountDebugger().DeletionHasBegun());
  }

  TEST_CASE("Ref self-assignment is safe", "[Ref]")
  {
    auto *obj = new TestRefCounted();

    {
      Ref<TestRefCounted> ref = AdoptRef(*obj);
      REQUIRE(obj->GetRefCount() == 1);

      ref = ref;
      REQUIRE(obj->GetRefCount() == 1);

      ref = Krys::Move(ref);
      REQUIRE(obj->GetRefCount() == 1);
    }

    REQUIRE(obj->GetRefCountDebugger().DeletionHasBegun());
  }

  TEST_CASE("Ref release transfers ownership without refcount change", "[Ref]")
  {
    auto *obj = new TestRefCounted();

    TestRefCounted *raw = nullptr;

    {
      Ref<TestRefCounted> ref = AdoptRef(*obj);
      REQUIRE(obj->GetRefCount() == 1);

      raw = ref.release();
      REQUIRE(obj->GetRefCount() == 1);
    }

    // raw still owns the ref here
    raw->SubRef();
    REQUIRE(obj->GetRefCountDebugger().DeletionHasBegun());
  }

}