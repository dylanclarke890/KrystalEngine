#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::tests
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