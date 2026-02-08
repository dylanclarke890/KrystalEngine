#pragma once

#include "Krystal.Lib/Pointers/RefCounted/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CheckedRef.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefCounted.hpp"

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

  class TestCheckedRefCounted : public CanMakeCheckedPtr<TestCheckedRefCounted>
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TestCheckedRefCounted);

  public:
    TestCheckedRefCounted() = default;
    virtual ~TestCheckedRefCounted() = default;
  };
}