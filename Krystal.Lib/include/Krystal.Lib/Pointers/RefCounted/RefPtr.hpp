#pragma once

#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtrTraits.hpp"
#include "Krystal.Lib/Pointers/RefCounted/Ref.hpp"
#include <algorithm>
#include <utility>

namespace Krys
{
  template <typename T, typename PtrTraits, typename RefDerefTraits>
  class RefPtr;

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  constexpr RefPtr<T, PtrTraits, RefDerefTraits> AdoptRef(RawPtr<T>) noexcept;

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  class RefPtr
  {
    KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW;

    friend constexpr RefPtr AdoptRef<T, PtrTraits, RefDerefTraits>(RawPtr<T>) noexcept;

    template <typename X, typename Y, typename Z>
    friend class RefPtr;

  private:
    typename PtrTraits::storage_type _ptr;

  public:
    KRYS_ALWAYS_INLINE constexpr RefPtr() noexcept : _ptr(nullptr)
    {
    }

    KRYS_ALWAYS_INLINE constexpr RefPtr(std::nullptr_t) noexcept : _ptr(nullptr)
    {
    }

    KRYS_ALWAYS_INLINE constexpr RefPtr(RawPtr<T> ptr) noexcept : _ptr(RefDerefTraits::AddRef(ptr))
    {
    }

    KRYS_ALWAYS_INLINE constexpr RefPtr(T &ptr) noexcept : _ptr(&RefDerefTraits::AddRef(ptr))
    {
    }

    KRYS_ALWAYS_INLINE constexpr ~RefPtr() noexcept
    {
      RefDerefTraits::SubRef(PtrTraits::exchange(_ptr, nullptr));
    }

    KRYS_ALWAYS_INLINE constexpr RefPtr(const RefPtr &o) noexcept
        : _ptr(RefDerefTraits::AddRef(PtrTraits::unwrap(o._ptr)))
    {
    }

    template <typename X, typename Y, typename Z>
    constexpr RefPtr(const RefPtr<X, Y, Z> &o) noexcept
        : _ptr(RefDerefTraits::AddRef(PtrTraits::unwrap(o.get())))
    {
    }

    KRYS_ALWAYS_INLINE constexpr RefPtr(RefPtr &&o) noexcept : _ptr(o.LeakRef())
    {
    }

    template <typename X, typename Y, typename Z>
    constexpr RefPtr(RefPtr<X, Y, Z> &&o) noexcept : _ptr(o.LeakRef())
    {
    }

    template <typename X, typename Y>
    constexpr RefPtr(Ref<X, Y> &&reference) noexcept : _ptr(&reference.LeakRef())
    {
    }

    // Hash table deleted values, which are only constructed and never copied or destroyed.
    constexpr RefPtr(HashTableDeletedValueType) noexcept : _ptr(PtrTraits::GetHashTableDeletedValue())
    {
    }

    constexpr RefPtr(HashTableEmptyValueType) noexcept : _ptr(GetHashTableEmptyValue())
    {
    }

    constexpr RefPtr &operator=(const RefPtr &o) noexcept
    {
      RefPtr ptr = o;
      swap(ptr);
      return *this;
    }

    constexpr RefPtr &operator=(RawPtr<T> optr) noexcept
    {
      RefPtr ptr = optr;
      swap(ptr);
      return *this;
    }

    constexpr RefPtr &operator=(std::nullptr_t) noexcept
    {
      RefDerefTraits::SubRef(PtrTraits::exchange(_ptr, nullptr));
      return *this;
    }

    template <typename X, typename Y, typename Z>
    constexpr RefPtr &operator=(const RefPtr<X, Y, Z> &o) noexcept
    {
      RefPtr ptr = o;
      swap(ptr);
      return *this;
    }

    constexpr RefPtr &operator=(RefPtr &&o) noexcept
    {
      RefPtr ptr = Krys::Move(o);
      swap(ptr);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    constexpr RefPtr &operator=(RefPtr<X, Y, Z> &&o) noexcept
    {
      RefPtr ptr = Krys::Move(o);
      swap(ptr);
      return *this;
    }

    template <typename X>
    constexpr RefPtr &operator=(Ref<X> &&reference) noexcept
    {
      RefPtr ptr = Krys::Move(reference);
      swap(ptr);
      return *this;
    }

    KRYS_NODISCARD constexpr bool IsHashTableDeletedValue() const noexcept
    {
      return PtrTraits::IsHashTableDeletedValue(_ptr);
    }

    KRYS_NODISCARD constexpr bool IsHashTableEmptyValue() const noexcept
    {
      return _ptr == GetHashTableEmptyValue();
    }

    KRYS_NODISCARD constexpr static RawPtr<T> GetHashTableEmptyValue() noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD constexpr RawPtr<T> get() const noexcept KRYS_LIFETIME_BOUND
    {
      return PtrTraits::unwrap(_ptr);
    }

    KRYS_NODISCARD constexpr Ref<T> ReleaseNonNull() noexcept
    {
      assert(_ptr);
      Ref<T> tmp(AdoptRef(*_ptr));
      _ptr = nullptr;
      return tmp;
    }

    KRYS_NODISCARD constexpr RawPtr<T> LeakRef() noexcept
    {
      return PtrTraits::exchange(_ptr, nullptr);
    }

    KRYS_ALWAYS_INLINE constexpr T &operator*() const noexcept KRYS_LIFETIME_BOUND
    {
      assert(_ptr);
      return *PtrTraits::unwrap(_ptr);
    }

    KRYS_ALWAYS_INLINE constexpr RawPtr<T> operator->() const noexcept KRYS_LIFETIME_BOUND
    {
      return &**this;
    }

    constexpr bool operator!() const noexcept
    {
      return !_ptr;
    }

    explicit constexpr operator bool() const noexcept
    {
      return !!_ptr;
    }

    template <typename X, typename Y, typename Z>
    constexpr void swap(RefPtr<X, Y, Z> &o) noexcept
    {
      PtrTraits::swap(_ptr, o._ptr);
    }

    RefPtr CopyRef() && = delete;
    KRYS_NODISCARD constexpr RefPtr CopyRef() const & noexcept
    {
      return RefPtr(_ptr);
    }

    constexpr bool operator==(std::nullptr_t) noexcept
    {
      return !_ptr;
    }

    enum AdoptTag
    {
      Adopt
    };

    RefPtr(RawPtr<T> ptr, AdoptTag) : _ptr(ptr)
    {
    }

  private:
    template <typename T1, typename U, typename V, typename X, typename Y, typename Z>
    friend constexpr bool operator==(const RefPtr<T1, U, V> &, const RefPtr<X, Y, Z> &) noexcept;

    template <typename T1, typename U, typename V, typename X>
    friend constexpr bool operator==(const RefPtr<T1, U, V> &, RawPtr<X>) noexcept;
  };

  // Template deduction guide.
  template <typename X, typename Y>
  RefPtr(Ref<X, Y> &&) -> RefPtr<X, Y, DefaultRefDerefTraits<X>>;

  template <typename T, typename U, typename V>
  constexpr inline void swap(RefPtr<T, U, V> &a, RefPtr<T, U, V> &b) noexcept
  {
    a.swap(b);
  }

  template <typename T, typename U, typename V, typename X, typename Y, typename Z>
  constexpr inline bool operator==(const RefPtr<T, U, V> &a, const RefPtr<X, Y, Z> &b) noexcept
  {
    return a._ptr == b._ptr;
  }

  template <typename T, typename U, typename V, typename X>
  constexpr inline bool operator==(const RefPtr<T, U, V> &a, RawPtr<X> b) noexcept
  {
    return a._ptr == b;
  }

  template <typename T, typename U, typename V>
  constexpr inline RefPtr<T, U, V> AdoptRef(RawPtr<T> p) noexcept
  {
    return RefPtr<T, U, V>(p, RefPtr<T, U, V>::Adopt);
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  inline RefPtr<T, U, V> Upcast(const RefPtr<X, Y, Z> &p) noexcept
  {
    static_assert(!SameType<X, T>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<X, T>, "Should be an upcast");
    return RefPtr<T, U, V>(static_cast<RawPtr<T>>(p.get()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  inline RefPtr<T, U, V> Upcast(RefPtr<X, Y, Z> &&p) noexcept
  {
    static_assert(!SameType<X, T>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<X, T>, "Should be an upcast");
    return AdoptRef(static_cast<RawPtr<T>>(p.leakRef()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  inline RefPtr<T, U, V> UnsafeRefPtrDowncast(const RefPtr<X, Y, Z> &p) noexcept
  {
    static_assert(!SameType<X, T>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<T, X>, "Use upcast instead");
    return RefPtr<T, U, V>(static_cast<RawPtr<T>>(p.get()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  inline RefPtr<T, U, V> UnsafeRefPtrDowncast(RefPtr<X, Y, Z> &&p) noexcept
  {
    static_assert(!SameType<X, T>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<T, X>, "Use upcast instead");
    return AdoptRef(static_cast<RawPtr<T>>(p.LeakRef()));
  }

  template <typename T, typename U, typename V>
  struct IsSmartPtr<RefPtr<T, U, V>>
  {
    static constexpr bool value = true;
    static constexpr bool isNullable = true;
  };

  template <typename TExpected, typename TArg, typename PtrTraits, typename RefDerefTraits>
  inline bool Is(const RefPtr<TArg, PtrTraits, RefDerefTraits> &source) noexcept
  {
    return Is<TExpected>(source.get());
  }

  template <typename Target, typename Source, typename PtrTraits, typename RefDerefTraits>
  inline RefPtr<match_constness_t<Source, Target>, PtrTraits, RefDerefTraits>
    UncheckedDowncast(RefPtr<Source, PtrTraits, RefDerefTraits> source) noexcept
  {
    static_assert(!SameType<Source, Target>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<Target, Source>, "Should be a downcast");
    assert(!source || Is<Target>(*source));
    return UnsafeRefPtrDowncast<match_constness_t<Source, Target>>(Krys::Move(source));
  }

  template <typename Target, typename Source, typename PtrTraits, typename RefDerefTraits>
  inline RefPtr<match_constness_t<Source, Target>, PtrTraits, RefDerefTraits>
    Downcast(RefPtr<Source, PtrTraits, RefDerefTraits> source) noexcept
  {
    static_assert(!SameType<Source, Target>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<Target, Source>, "Should be a downcast");
    assert(!source || Is<Target>(*source));
    return UnsafeRefPtrDowncast<match_constness_t<Source, Target>>(Krys::Move(source));
  }

  template <typename Target, typename Source, typename TargetPtrTraits = RawPtrTraits<Target>,
            typename TargetRefDerefTraits = DefaultRefDerefTraits<Target>, typename SourcePtrTraits,
            typename SourceRefDerefTraits>
  inline RefPtr<match_constness_t<Source, Target>, TargetPtrTraits, TargetRefDerefTraits>
    DynamicDowncast(RefPtr<Source, SourcePtrTraits, SourceRefDerefTraits> source) noexcept
  {
    static_assert(!SameType<Source, Target>, "Unnecessary cast to same type");
    static_assert(DerivedFrom<Target, Source>, "Should be a downcast");
    if (!Is<Target>(source))
    {
      return nullptr;
    }

    return UnsafeRefPtrDowncast<match_constness_t<Source, Target>, TargetPtrTraits, TargetRefDerefTraits>(
      Krys::Move(source));
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>, typename... Args>
  requires(Constructible<T, Args...>)
  KRYS_NODISCARD constexpr inline RefPtr<T, PtrTraits, RefDerefTraits> CreateRefPtr(Args &&...args)
  {
    RawPtr<T> ptr = new T(std::forward<Args>(args)...);
    return RefPtr<T, PtrTraits, RefDerefTraits>(ptr, RefPtr<T, PtrTraits, RefDerefTraits>::Adopt);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  KRYS_NODISCARD RefPtr<T, PtrTraits, RefDerefTraits> AttachRefPtr(RawPtr<T> ptr) noexcept
  {
    return RefPtr<T, PtrTraits, RefDerefTraits>(ptr, RefPtr<T, PtrTraits, RefDerefTraits>::Adopt);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  KRYS_NODISCARD RefPtr<T, PtrTraits, RefDerefTraits> AttachRefPtr(T &ptr) noexcept
  {
    return RefPtr<T, PtrTraits, RefDerefTraits>(&ptr, RefPtr<T, PtrTraits, RefDerefTraits>::Adopt);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  KRYS_NODISCARD constexpr RefPtr<T, PtrTraits, RefDerefTraits> RetainRefPtr(T &ptr) noexcept
  {
    return RefPtr<T, PtrTraits, RefDerefTraits>(&ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  KRYS_NODISCARD constexpr RefPtr<T, PtrTraits, RefDerefTraits> RetainRefPtr(RawPtr<T> ptr) noexcept
  {
    return RefPtr<T, PtrTraits, RefDerefTraits>(ptr);
  }
}