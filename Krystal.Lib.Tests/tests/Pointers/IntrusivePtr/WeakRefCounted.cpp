#include "Krystal.Lib/Pointers/IntrusivePtrV1/RefCounted.hpp"
#include "Krystal.Lib/Pointers/IntrusivePtrV1/RefPtr.hpp"
#include <catch_all.hpp>
#include <cstdint>
#include <type_traits>

namespace Krys::Tests
{
  namespace
  {
    int DerivedCount = 0;

    struct DerivedCounted : WeakRefCounted<DerivedCounted>
    {
      friend RefCounted;

    public:
      DerivedCounted() noexcept
      {
        ++DerivedCount;
      }

    private:
      ~DerivedCounted() noexcept
      {
        auto weak = GetWeakPtr();
        CHECK(weak);
        --DerivedCount;
      }
    };

    int WrappedCount = 0;
    struct wrapped
    {
      wrapped()
      {
        ++WrappedCount;
      }
      ~wrapped()
      {
        --WrappedCount;
      }

      int value = 5;
    };

    using WrappedCounted = WeakRefCountedAdapter<wrapped>;

    int WithCustomWeakReferenceCount = 0;

    struct CustomWeakReference;

    class WithCustomWeakReference
        : public RefCounted<WithCustomWeakReference, RefCountedFlag::ProvideWeakReferences>
    {
      friend RefCounted;

    public:
      WithCustomWeakReference()
      {
        ++WithCustomWeakReferenceCount;
      }

    private:
      ~WithCustomWeakReference() noexcept
      {
        auto weak = GetWeakPtr();
        CHECK(weak);
        --WithCustomWeakReferenceCount;
      }

      CustomWeakReference *MakeWeakReference(intptr_t count) const;
    };

    struct CustomWeakReference : WeakReference<WithCustomWeakReference>
    {
      CustomWeakReference(intptr_t count, WithCustomWeakReference *obj) : WeakReference(count, obj)
      {
      }

      ~CustomWeakReference()
      {
        CHECK(OnOwnerDestructionCalled);
        CHECK(WithCustomWeakReferenceCount == 0);
      }

      void OnOwnerDestruction() const
      {
        CHECK(WithCustomWeakReferenceCount
              == 0); // owner is still alive but its refcount is 0, you cannot resurrect!
        OnOwnerDestructionCalled = true;
      }

      mutable bool OnOwnerDestructionCalled = false;
    };

    inline CustomWeakReference *WithCustomWeakReference::MakeWeakReference(intptr_t count) const
    {
      return new CustomWeakReference(count, const_cast<WithCustomWeakReference *>(this));
    }
  }

  TEST_CASE("RefCounted - Weak Ref counted type traits are correct")
  {
    SECTION("Base")
    {
      CHECK(!DefaultConstructible<WeakRefCounted<DerivedCounted>>);
      CHECK(!CopyConstructible<WeakRefCounted<DerivedCounted>>);
      CHECK(!MoveConstructible<WeakRefCounted<DerivedCounted>>);
      CHECK(!CopyAssignable<WeakRefCounted<DerivedCounted>>);
      CHECK(!MoveAssignable<WeakRefCounted<DerivedCounted>>);
      CHECK(!Swappable<WeakRefCounted<DerivedCounted>>);
      CHECK(!Destructible<WeakRefCounted<DerivedCounted>>);
    }

    SECTION("Derived")
    {
      CHECK(!DefaultConstructible<DerivedCounted>);
      CHECK(!CopyConstructible<DerivedCounted>);
      CHECK(!MoveConstructible<DerivedCounted>);
      CHECK(!CopyAssignable<DerivedCounted>);
      CHECK(!MoveAssignable<DerivedCounted>);
      CHECK(!Swappable<DerivedCounted>);
      CHECK(!Destructible<DerivedCounted>);
    }

    SECTION("Derived WeakRef")
    {
      CHECK(!DefaultConstructible<DerivedCounted::weak_value_type>);
      CHECK(!CopyConstructible<DerivedCounted::weak_value_type>);
      CHECK(!MoveConstructible<DerivedCounted::weak_value_type>);
      CHECK(!CopyAssignable<DerivedCounted::weak_value_type>);
      CHECK(!MoveAssignable<DerivedCounted::weak_value_type>);
      CHECK(!Swappable<DerivedCounted::weak_value_type>);
      CHECK(!Destructible<DerivedCounted::weak_value_type>);
    }

    SECTION("Wrapped")
    {
      CHECK(!DefaultConstructible<WrappedCounted>);
      CHECK(!CopyConstructible<WrappedCounted>);
      CHECK(!MoveConstructible<WrappedCounted>);
      CHECK(!CopyAssignable<WrappedCounted>);
      CHECK(!MoveAssignable<WrappedCounted>);
      CHECK(!Destructible<WrappedCounted>);
    }

    SECTION("Wrapped WeakRef")
    {
      CHECK(!DefaultConstructible<WrappedCounted::weak_value_type>);
      CHECK(!CopyConstructible<WrappedCounted::weak_value_type>);
      CHECK(!MoveConstructible<WrappedCounted::weak_value_type>);
      CHECK(!CopyAssignable<WrappedCounted::weak_value_type>);
      CHECK(!MoveAssignable<WrappedCounted::weak_value_type>);
      CHECK(!Destructible<WrappedCounted::weak_value_type>);
    }
  }

  TEST_CASE("RefCounted - Weak Ref counted works")
  {
    SECTION("Derived")
    {
      auto original = IntrusiveRefPtrAttach(new DerivedCounted());
      auto weak1 = original->GetWeakPtr();
      CHECK(DerivedCount == 1);
      auto strong1 = weak1->Lock();
      CHECK(original == strong1);
      auto weak2 = strong1->GetWeakPtr();
      CHECK(weak1 == weak2);
      auto weak3 = weak_cast(strong1);
      CHECK(weak1 == weak3);
      original.reset();
      strong1.reset();
      CHECK(DerivedCount == 0);

      strong1 = weak1->Lock();
      CHECK(!strong1);
    }

    SECTION("Const Derived")
    {
      IntrusiveRefPtr<const DerivedCounted> original = IntrusiveRefPtrAttach(new DerivedCounted());
      auto weak1 = original->GetWeakPtr();
      CHECK(DerivedCount == 1);
      auto strong1 = weak1->Lock();
      CHECK(original == strong1);
      auto weak2 = strong1->GetWeakPtr();
      CHECK(weak1 == weak2);
      original.reset();
      strong1.reset();
      CHECK(DerivedCount == 0);

      strong1 = weak1->Lock();
      CHECK(!strong1);
      strong1 = strong_cast(weak2);
      CHECK(!strong1);
    }

    SECTION("Wrapped")
    {
      auto p = IntrusiveRefPtrAttach(new WrappedCounted());
      CHECK(WrappedCount == 1);
      CHECK(p->value == 5);
      p.reset();
      CHECK(WrappedCount == 0);
    }

    SECTION("Custom Weak Reference")
    {
      auto strong = IntrusiveRefPtrAttach(new WithCustomWeakReference);
      CHECK(WithCustomWeakReferenceCount == 1);
      auto weak = strong->GetWeakPtr();
      auto strong1 = weak->Lock();
      CHECK(strong1 == strong);
      CHECK(WithCustomWeakReferenceCount == 1);
      strong.reset();
      CHECK(WithCustomWeakReferenceCount == 1);
      strong1.reset();
      CHECK(WithCustomWeakReferenceCount == 0);
      strong1 = weak->Lock();
      CHECK(!strong1);
    }
  }
}