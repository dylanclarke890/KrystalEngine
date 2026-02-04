#include "Krystal.Lib/Pointers/IntrusivePtrV1/RefCounted.hpp"
#include <catch_all.hpp>
#include <cstdint>
#include <type_traits>

namespace Krys::Tests
{
  namespace
  {
    bool CountedAddRefCalled = false;
    bool CountedSubRefCalled = false;
    bool CountedDestroyCalled = false;
    bool MakeWeakReferenceCalled = false;
    bool GetWeakValueCalled = false;

    class AbstractWeakReference;

    class AbstractRefCounted : public RefCounted<AbstractRefCounted, RefCountedFlag::ProvideWeakReferences>
    {
      friend RefCounted;

    public:
      virtual void AddRef() const noexcept final
      {
        CountedAddRefCalled = true;
        RefCounted::AddRef();
      }

      virtual void SubRef() const noexcept final
      {
        CountedSubRefCalled = true;
        RefCounted::SubRef();
      }

    protected:
      virtual ~AbstractRefCounted() noexcept
      {
      }

      virtual void Destroy() const noexcept
      {
        CountedDestroyCalled = true;
        RefCounted::Destroy();
      }

      virtual AbstractWeakReference *MakeWeakReference(intptr_t count) const;

      virtual const WeakReference<AbstractRefCounted> *GetWeakValue() const
      {
        GetWeakValueCalled = true;
        return RefCounted::GetWeakValue();
      }
    };

    bool WeakAddRefCalled = false;
    bool WeakSubRefCalled = false;
    bool WeakDestroyCalled = false;
    bool AddOwnerRefCalled = false;
    bool SubOwnerRefCalled = false;
    bool LockOwnerCalled = false;
    bool OnOwnerDestructionCalled = false;

    class AbstractWeakReference : public WeakReference<AbstractRefCounted>
    {
      friend WeakReference;
      friend AbstractRefCounted;

    public:
      virtual void AddRef() const noexcept final
      {
        WeakAddRefCalled = true;
        WeakReference::AddRef();
      }

      virtual void SubRef() const noexcept final
      {
        WeakSubRefCalled = true;
        WeakReference::SubRef();
      }

    protected:
      virtual ~AbstractWeakReference() noexcept
      {
      }

      virtual void Destroy() const noexcept
      {
        WeakDestroyCalled = true;
        WeakReference::Destroy();
      }

    private:
      AbstractWeakReference(intptr_t count, AbstractRefCounted *owner) : WeakReference(count, owner)
      {
      }

      virtual void AddOwnerRef() noexcept
      {
        AddOwnerRefCalled = true;
        WeakReference::AddOwnerRef();
      }

      virtual void SubOwnerRef() noexcept
      {
        SubOwnerRefCalled = true;
        WeakReference::SubOwnerRef();
      }

      virtual AbstractRefCounted *LockOwner() const noexcept
      {
        LockOwnerCalled = true;
        return static_cast<AbstractRefCounted *>(WeakReference::LockOwner());
      }

      virtual void OnOwnerDestruction() const noexcept
      {
        OnOwnerDestructionCalled = true;
      }
    };

    inline auto AbstractRefCounted::MakeWeakReference(intptr_t count) const -> AbstractWeakReference *
    {
      MakeWeakReferenceCalled = true;
      return new AbstractWeakReference(count, const_cast<AbstractRefCounted *>(this));
    }

    class simple : public AbstractRefCounted
    {
    };
  }

  TEST_CASE("RefCounted - Abstract")
  {
    SECTION("Simple")
    {
      auto p = IntrusiveRefPtrAttach(new simple());
      decltype(p) p1 = p;
      p1.reset();
      auto w = weak_cast(p);
      static_assert(std::is_same_v<decltype(w)::element_type, WeakReference<AbstractRefCounted>>,
                    "invalid weak reference type");
      auto w1 = p->GetWeakPtr();
      static_assert(std::is_same_v<decltype(w1)::element_type, WeakReference<AbstractRefCounted>>,
                    "invalid weak reference type");
      auto p2 = strong_cast(w);
      auto p3 = p2;
      static_assert(std::is_same_v<decltype(p2)::element_type, AbstractRefCounted>,
                    "invalid weak reference type");

      p.reset();
      p2.reset();
      p3.reset();
      w.reset();
      w1.reset();

      CHECK(CountedAddRefCalled);
      CHECK(CountedSubRefCalled);
      CHECK(CountedDestroyCalled);
      CHECK(MakeWeakReferenceCalled);
      CHECK(GetWeakValueCalled);
      CHECK(WeakAddRefCalled);
      CHECK(WeakSubRefCalled);
      CHECK(WeakDestroyCalled);
      CHECK(AddOwnerRefCalled);
      CHECK(SubOwnerRefCalled);
      CHECK(LockOwnerCalled);
      CHECK(OnOwnerDestructionCalled);
    }
  }
}