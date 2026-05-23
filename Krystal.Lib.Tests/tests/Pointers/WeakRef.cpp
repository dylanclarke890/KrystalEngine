#include "Krystal.Lib/Mixins/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"
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

  TEST_CASE("WeakRef does not affect refcount", "[WeakRef]")
  {
    auto *obj = new TestWeakRefCounted();
    REQUIRE(obj->GetRefCount() == 1);

    WeakRef<TestWeakRefCounted> weakRef = CreateWeakRef(*obj);
    REQUIRE(obj->GetRefCount() == 1);

    obj->SubRef();
  }
}