#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Detection/Sanitizers.hpp"
#include "Krystal.Lib/ForbidHeapAllocation.hpp"
#include "Krystal.Lib/Pointers/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtrTraits.hpp"
#include <cassert>

#if KRYS_ASAN_ENABLED
extern "C" void __asan_poison_memory_region(void const volatile *addr, size_t size);
extern "C" void __asan_unpoison_memory_region(void const volatile *addr, size_t size);
extern "C" int __asan_address_is_poisoned(void const volatile *addr);
#endif

namespace Krys
{
  template <typename T>
  struct DefaultRefDerefTraits
  {
    constexpr static KRYS_ALWAYS_INLINE RawPtr<T> AddRef(RawPtr<T> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->AddRef();
      }
      return ptr;
    }

    constexpr static KRYS_ALWAYS_INLINE T &AddRef(T &ref) noexcept
    {
      ref.AddRef();
      return ref;
    }

    constexpr static KRYS_ALWAYS_INLINE void SubRef(RawPtr<T> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->SubRef();
      }
    }
  };

  template <typename T, typename PtrTraits, typename RefDerefTraits>
  class RefPtr;

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  class Ref;

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  constexpr Ref<T, PtrTraits, RefDerefTraits> AdoptRef(T &) noexcept;

  template <typename T, typename PtrTraits, typename RefDerefTraits>
  class Ref
  {
    static_assert(!IsPointer<T>, "T must not be a pointer type.");

    KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW;

    friend constexpr Ref AdoptRef<T>(T &) noexcept;

    template <typename X, typename Y, typename Z>
    friend class Ref;

  private:
    typename PtrTraits::storage_type _ptr;

  public:
    constexpr ~Ref() noexcept
    {
#if KRYS_ASAN_ENABLED
      if (__asan_address_is_poisoned(this))
      {
        __asan_unpoison_memory_region(this, sizeof(*this));
      }
#endif
      if (auto *ptr = PtrTraits::exchange(_ptr, nullptr))
      {
        RefDerefTraits::SubRef(ptr);
      }
    }

    constexpr Ref(T &object) noexcept : _ptr(&RefDerefTraits::AddRef(object))
    {
    }

    constexpr Ref(const Ref &other) noexcept : _ptr(&RefDerefTraits::AddRef(other.get()))
    {
    }

    template <typename X, typename Y>
    constexpr Ref(const Ref<X, Y> &other) noexcept : _ptr(&RefDerefTraits::AddRef(other.get()))
    {
    }

    constexpr Ref(Ref &&other) noexcept : _ptr(&other.LeakRef())
    {
      assert(_ptr);
    }

    template <typename X, typename Y>
    constexpr Ref(Ref<X, Y> &&other) noexcept : _ptr(&other.LeakRef())
    {
      assert(_ptr);
    }

    constexpr Ref &operator=(T &reference) noexcept
    {
      Ref copiedReference = reference;
      swap(copiedReference);
      return *this;
    }

    constexpr Ref &operator=(Ref &&reference) noexcept
    {
#if KRYS_ASAN_ENABLED
      if (__asan_address_is_poisoned(this))
        __asan_unpoison_memory_region(this, sizeof(*this));
#endif
      Ref movedReference = Krys::Move(reference);
      swap(movedReference);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    constexpr Ref &operator=(Ref<X, Y, Z> &&reference) noexcept
    {
#if KRYS_ASAN_ENABLED
      if (__asan_address_is_poisoned(this))
        __asan_unpoison_memory_region(this, sizeof(*this));
#endif
      Ref movedReference = Krys::Move(reference);
      swap(movedReference);
      return *this;
    }

    constexpr Ref &operator=(const Ref &reference) noexcept
    {
#if KRYS_ASAN_ENABLED
      if (__asan_address_is_poisoned(this))
        __asan_unpoison_memory_region(this, sizeof(*this));
#endif
      Ref copy = reference;
      swap(copy);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    constexpr Ref &operator=(const Ref<X, Y, Z> &reference) noexcept
    {
#if KRYS_ASAN_ENABLED
      if (__asan_address_is_poisoned(this))
        __asan_unpoison_memory_region(this, sizeof(*this));
#endif
      Ref copy = reference;
      swap(copy);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    constexpr void swap(Ref<X, Y, Z> &other) noexcept
    {
      PtrTraits::swap(_ptr, other._ptr);
    }

    // Hash table deleted values, which are only constructed and never copied or destroyed.
    constexpr Ref(HashTableDeletedValueType) noexcept : _ptr(PtrTraits::GetHashTableDeletedValue())
    {
    }

    constexpr bool IsHashTableDeletedValue() const noexcept
    {
      return PtrTraits::IsHashTableDeletedValue(_ptr);
    }

    constexpr Ref(HashTableEmptyValueType) noexcept : _ptr(GetHashTableEmptyValue())
    {
    }

    constexpr bool IsHashTableEmptyValue() const noexcept
    {
      return _ptr == GetHashTableEmptyValue();
    }

    constexpr static RawPtr<T> GetHashTableEmptyValue() noexcept
    {
      return nullptr;
    }

    constexpr const RawPtr<T> PtrAllowingHashTableEmptyValue() const noexcept KRYS_LIFETIME_BOUND
    {
      assert(_ptr || IsHashTableEmptyValue());
      return PtrTraits::unwrap(_ptr);
    }

    constexpr RawPtr<T> PtrAllowingHashTableEmptyValue() noexcept KRYS_LIFETIME_BOUND
    {
      assert(_ptr || IsHashTableEmptyValue());
      return PtrTraits::unwrap(_ptr);
    }

    constexpr RawPtr<T> operator->() const noexcept KRYS_LIFETIME_BOUND
    {
      assert(_ptr);
      return PtrTraits::unwrap(_ptr);
    }

    constexpr RawPtr<T> ptr() const noexcept KRYS_LIFETIME_BOUND KRYS_RETURNS_NONNULL
    {
      assert(_ptr);
      return PtrTraits::unwrap(_ptr);
    }

    constexpr T &get() const noexcept KRYS_LIFETIME_BOUND
    {
      assert(_ptr);
      return *PtrTraits::unwrap(_ptr);
    }

    constexpr operator T &() const noexcept KRYS_LIFETIME_BOUND
    {
      assert(_ptr);
      return *PtrTraits::unwrap(_ptr);
    }

    constexpr bool operator!() const noexcept
    {
      assert(_ptr);
      return !*_ptr;
    }

    template <typename X, typename Y, typename Z>
    KRYS_NODISCARD constexpr Ref<T, PtrTraits, RefDerefTraits> replace(Ref<X, Y, Z> &&reference) noexcept
    {
#if KRYS_ASAN_ENABLED
      if (__asan_address_is_poisoned(this))
        __asan_unpoison_memory_region(this, sizeof(*this));
#endif

      auto oldReference = AdoptRef(*_ptr);
      _ptr = &reference.leakRef();
      return oldReference;
    }

    Ref CopyRef() && noexcept = delete;
    KRYS_NODISCARD constexpr Ref CopyRef() const & noexcept
    {
      return Ref(*_ptr);
    }

    KRYS_NODISCARD constexpr T &LeakRef() noexcept
    {
      assert(_ptr);

      T &result = *PtrTraits::exchange(_ptr, nullptr);
#if KRYS_ASAN_ENABLED
      __asan_poison_memory_region(this, sizeof(*this));
#endif
      return result;
    }

  private:
    enum AdoptTag
    {
      Adopt
    };

    constexpr Ref(T &object, AdoptTag) noexcept : _ptr(&object)
    {
    }
  };

  template <typename T, typename PtrTraits, typename RefDerefTraits>
  constexpr inline Ref<T, PtrTraits, RefDerefTraits> AdoptRef(T &reference) noexcept
  {
    return Ref<T, PtrTraits, RefDerefTraits>(reference, Ref<T, PtrTraits, RefDerefTraits>::Adopt);
  }

  template <typename X, typename APtrTraits, typename ARefDerefTraits, typename Y, typename BPtrTraits,
            typename BRefDerefTraits>
  requires(!SameType<APtrTraits, RawPtrTraits<X>> || !SameType<BPtrTraits, RawPtrTraits<Y>>)
  inline void swap(Ref<X, APtrTraits, ARefDerefTraits> &a, Ref<Y, BPtrTraits, BRefDerefTraits> &b) noexcept
  {
    a.swap(b);
  }

  template <typename X, typename PtrTraits = RawPtrTraits<X>,
            typename RefDerefTraits = DefaultRefDerefTraits<X>, typename Y, typename OtherPtrTraits,
            typename OtherRefDerefTraits>
  inline Ref<X, PtrTraits, RefDerefTraits>
    Upcast(Ref<Y, OtherPtrTraits, OtherRefDerefTraits> &&reference) noexcept
  {
    static_assert(!SameType<Y, X>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<Y, X>, "Should be an upcast");
    return AdoptRef(static_cast<X &>(reference.LeakRef()));
  }

  template <typename X, typename PtrTraits = RawPtrTraits<X>,
            typename RefDerefTraits = DefaultRefDerefTraits<X>, typename Y, typename OtherPtrTraits,
            typename OtherRefDerefTraits>
  KRYS_ALWAYS_INLINE Ref<X, PtrTraits, RefDerefTraits>
    Upcast(const Ref<Y, OtherPtrTraits, OtherRefDerefTraits> &reference) noexcept
  {
    static_assert(!SameType<Y, X>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<Y, X>, "Should be an upcast");
    return Upcast<X, PtrTraits, RefDerefTraits>(reference.CopyRef());
  }

  template <typename X, typename PtrTraits = RawPtrTraits<X>,
            typename RefDerefTraits = DefaultRefDerefTraits<X>, typename Y, typename OtherPtrTraits,
            typename OtherRefDerefTraits>
  inline Ref<X, PtrTraits, RefDerefTraits>
    UnsafeRefDowncast(Ref<Y, OtherPtrTraits, OtherRefDerefTraits> &&reference) noexcept
  {
    static_assert(!SameType<Y, X>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<X, Y>, "Use upcast instead");
    return AdoptRef(static_cast<X &>(reference.LeakRef()));
  }

  template <typename X, typename PtrTraits = RawPtrTraits<X>,
            typename RefDerefTraits = DefaultRefDerefTraits<X>, typename Y, typename OtherPtrTraits,
            typename OtherRefDerefTraits>
  KRYS_ALWAYS_INLINE Ref<X, PtrTraits, RefDerefTraits>
    UnsafeRefDowncast(const Ref<Y, OtherPtrTraits, OtherRefDerefTraits> &reference) noexcept
  {
    static_assert(!SameType<Y, X>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<X, Y>, "Use upcast instead");
    return UnsafeRefDowncast<X, PtrTraits, RefDerefTraits>(reference.CopyRef());
  }

  template <typename T, typename PtrTraits, typename RefDerefTraits>
  struct GetPtrHelper<Ref<T, PtrTraits, RefDerefTraits>>
  {
    using pointer_type = RawPtr<T>;
    using underlying_type = T;

    static pointer_type GetPtr(const Ref<T, PtrTraits, RefDerefTraits> &p) noexcept
    {
      return const_cast<pointer_type>(p.ptr());
    }
  };

  template <typename T, typename PtrTraits, typename RefDerefTraits>
  struct IsSmartPtr<Ref<T, PtrTraits, RefDerefTraits>>
  {
    static constexpr bool value = true;
    static constexpr bool nullable = false;
  };

  template <typename TExpected, typename TArg, typename PtrTraits, typename RefDerefTraits>
  inline bool Is(const Ref<TArg, PtrTraits, RefDerefTraits> &source) noexcept
  {
    return Is<TExpected>(source.get());
  }

  template <typename Target, typename Source, typename PtrTraits, typename RefDerefTraits>
  inline Ref<match_constness_t<Source, Target>, PtrTraits, RefDerefTraits>
    UncheckedDowncast(Ref<Source, PtrTraits, RefDerefTraits> source) noexcept
  {
    static_assert(!SameType<Source, Target>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<Target, Source>, "Should be a downcast");
    assert(Is<Target>(source));
    return UnsafeRefDowncast<match_constness_t<Source, Target>>(Krys::Move(source));
  }

  template <typename Target, typename Source, typename PtrTraits, typename RefDerefTraits>
  inline Ref<match_constness_t<Source, Target>, PtrTraits, RefDerefTraits>
    Downcast(Ref<Source, PtrTraits, RefDerefTraits> source) noexcept
  {
    static_assert(!SameType<Source, Target>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<Target, Source>, "Should be a downcast");
    assert(Is<Target>(source));
    return UnsafeRefDowncast<match_constness_t<Source, Target>>(Krys::Move(source));
  }

  template <typename Target, typename Source, typename PtrTraits, typename RefDerefTraits>
  inline RefPtr<match_constness_t<Source, Target>, PtrTraits, RefDerefTraits>
    DynamicDowncast(Ref<Source, PtrTraits, RefDerefTraits> source) noexcept
  {
    static_assert(!SameType<Source, Target>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<Target, Source>, "Should be a downcast");
    if (!Is<Target>(source))
    {
      return nullptr;
    }

    return UnsafeRefDowncast<match_constness_t<Source, Target>>(Krys::Move(source));
  }

  template <typename T, typename PtrTraits, typename RefDerefTraits>
  KRYS_NODISCARD constexpr inline bool
    ArePointingToEqualData(const Ref<T, PtrTraits, RefDerefTraits> &a,
                           const Ref<T, PtrTraits, RefDerefTraits> &b) noexcept
  {
    return a.ptr() == b.ptr() || a.get() == b.get();
  }
}
