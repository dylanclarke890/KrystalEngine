#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Detection/Sanitizers.hpp"
#include "Krystal.Lib/ForbidHeapAllocation.hpp"
#include "Krystal.Lib/Pointers/RefCounted/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RawPtrTraits.hpp"
#include <cassert>

#if KRYS_ASAN_ENABLED
extern "C" void __asan_poison_memory_region(void const volatile *addr, size_t size);
extern "C" void __asan_unpoison_memory_region(void const volatile *addr, size_t size);
extern "C" int __asan_address_is_poisoned(void const volatile *addr);
#endif

namespace Krys
{
  template <typename T, typename PtrTraits, typename RefDerefTraits>
  class RefPtr;

  inline void adopted(const void *)
  {
  }

  template <typename T>
  struct DefaultRefDerefTraits
  {
    static KRYS_ALWAYS_INLINE T *refIfNotNull(T *ptr)
    {
      if (ptr) [[likely]]
      {
        ptr->AddRef();
      }
      return ptr;
    }

    static KRYS_ALWAYS_INLINE T &ref(T &ref)
    {
      ref.AddRef();
      return ref;
    }

    static KRYS_ALWAYS_INLINE void derefIfNotNull(T *ptr)
    {
      if (ptr) [[likely]]
      {
        ptr->SubRef();
      }
    }
  };

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  class Ref;

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  Ref<T, PtrTraits, RefDerefTraits> adoptRef(T &);

  template <typename T, typename _PtrTraits, typename RefDerefTraits>
  class Ref
  {
    KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW;

    static_assert(!IsPointer<T>, "T must not be a pointer type.");

  public:
    using PtrTraits = _PtrTraits;
    static constexpr bool isRef = true;

    ~Ref()
    {
#if KRYS_ASAN_ENABLED
      if (__asan_address_is_poisoned(this))
        __asan_unpoison_memory_region(this, sizeof(*this));
#endif
      if (auto *ptr = PtrTraits::exchange(_ptr, nullptr))
        RefDerefTraits::derefIfNotNull(ptr);
    }

    Ref(T &object) : _ptr(&RefDerefTraits::ref(object))
    {
    }

    Ref(const Ref &other) : _ptr(&RefDerefTraits::ref(other.get()))
    {
    }

    template <typename X, typename Y>
    Ref(const Ref<X, Y> &other) : _ptr(&RefDerefTraits::ref(other.get()))
    {
    }

    Ref(Ref &&other) : _ptr(&other.leakRef())
    {
      assert(_ptr);
    }

    template <typename X, typename Y>
    Ref(Ref<X, Y> &&other) : _ptr(&other.leakRef())
    {
      assert(_ptr);
    }

    Ref &operator=(T &);
    Ref &operator=(Ref &&);
    template <typename X, typename Y, typename Z>
    Ref &operator=(Ref<X, Y, Z> &&);

    Ref &operator=(const Ref &);
    template <typename X, typename Y, typename Z>
    Ref &operator=(const Ref<X, Y, Z> &);

    template <typename X, typename Y, typename Z>
    void swap(Ref<X, Y, Z> &);

    // Hash table deleted values, which are only constructed and never copied or destroyed.
    Ref(HashTableDeletedValueType) : _ptr(PtrTraits::hashTableDeletedValue())
    {
    }
    bool isHashTableDeletedValue() const
    {
      return PtrTraits::isHashTableDeletedValue(_ptr);
    }

    Ref(HashTableEmptyValueType) : _ptr(hashTableEmptyValue())
    {
    }
    bool isHashTableEmptyValue() const
    {
      return _ptr == hashTableEmptyValue();
    }
    static T *hashTableEmptyValue()
    {
      return nullptr;
    }

    const T *ptrAllowingHashTableEmptyValue() const KRYS_LIFETIME_BOUND
    {
      assert(_ptr || isHashTableEmptyValue());
      return PtrTraits::unwrap(_ptr);
    }
    T *ptrAllowingHashTableEmptyValue() KRYS_LIFETIME_BOUND
    {
      assert(_ptr || isHashTableEmptyValue());
      return PtrTraits::unwrap(_ptr);
    }

    T *operator->() const KRYS_LIFETIME_BOUND
    {
      assert(_ptr);
      return PtrTraits::unwrap(_ptr);
    }
    T *ptr() const KRYS_LIFETIME_BOUND KRYS_RETURNS_NONNULL
    {
      assert(_ptr);
      return PtrTraits::unwrap(_ptr);
    }
    T &get() const KRYS_LIFETIME_BOUND
    {
      assert(_ptr);
      return *PtrTraits::unwrap(_ptr);
    }
    operator T &() const KRYS_LIFETIME_BOUND
    {
      assert(_ptr);
      return *PtrTraits::unwrap(_ptr);
    }
    bool operator!() const
    {
      assert(_ptr);
      return !*_ptr;
    }

    template <typename X, typename Y, typename Z>
    KRYS_NODISCARD Ref<T, PtrTraits, RefDerefTraits> replace(Ref<X, Y, Z> &&);

    // The following function is deprecated.
    Ref copyRef() && = delete;
    KRYS_NODISCARD Ref copyRef() const &
    {
      return Ref(*_ptr);
    }

    KRYS_NODISCARD T &leakRef()
    {
      assert(_ptr);

      T &result = *PtrTraits::exchange(_ptr, nullptr);
#if KRYS_ASAN_ENABLED
      __asan_poison_memory_region(this, sizeof(*this));
#endif
      return result;
    }

  private:
    friend Ref adoptRef<T>(T &);
    template <typename X, typename Y, typename Z>
    friend class Ref;

    template <typename X, typename Y, typename Z, typename U, typename V, typename W>
    friend bool operator==(const Ref<X, Y, Z> &, const Ref<U, V, W> &);

    enum AdoptTag
    {
      Adopt
    };
    Ref(T &object, AdoptTag) : _ptr(&object)
    {
    }

    typename PtrTraits::StorageType _ptr;
  };

  template <typename T, typename _PtrTraits, typename RefDerefTraits>
  Ref<T, _PtrTraits, RefDerefTraits> adoptRef(T &);

  template <typename T, typename _PtrTraits, typename RefDerefTraits>
  inline Ref<T, _PtrTraits, RefDerefTraits> &Ref<T, _PtrTraits, RefDerefTraits>::operator=(T &reference)
  {
    Ref copiedReference = reference;
    swap(copiedReference);
    return *this;
  }

  template <typename T, typename _PtrTraits, typename RefDerefTraits>
  inline Ref<T, _PtrTraits, RefDerefTraits> &Ref<T, _PtrTraits, RefDerefTraits>::operator=(Ref &&reference)
  {
#if KRYS_ASAN_ENABLED
    if (__asan_address_is_poisoned(this))
      __asan_unpoison_memory_region(this, sizeof(*this));
#endif
    Ref movedReference = Krys::Move(reference);
    swap(movedReference);
    return *this;
  }

  template <typename T, typename _PtrTraits, typename RefDerefTraits>
  template <typename U, typename _OtherPtrTraits, typename OtherRefDerefTraits>
  inline Ref<T, _PtrTraits, RefDerefTraits> &
    Ref<T, _PtrTraits, RefDerefTraits>::operator=(Ref<U, _OtherPtrTraits, OtherRefDerefTraits> &&reference)
  {
#if KRYS_ASAN_ENABLED
    if (__asan_address_is_poisoned(this))
      __asan_unpoison_memory_region(this, sizeof(*this));
#endif
    Ref movedReference = Krys::Move(reference);
    swap(movedReference);
    return *this;
  }

  template <typename T, typename _PtrTraits, typename RefDerefTraits>
  inline Ref<T, _PtrTraits, RefDerefTraits> &
    Ref<T, _PtrTraits, RefDerefTraits>::operator=(const Ref &reference)
  {
#if KRYS_ASAN_ENABLED
    if (__asan_address_is_poisoned(this))
      __asan_unpoison_memory_region(this, sizeof(*this));
#endif
    Ref copiedReference = reference;
    swap(copiedReference);
    return *this;
  }

  template <typename T, typename _PtrTraits, typename RefDerefTraits>
  template <typename U, typename _OtherPtrTraits, typename OtherRefDerefTraits>
  inline Ref<T, _PtrTraits, RefDerefTraits> &Ref<T, _PtrTraits, RefDerefTraits>::operator=(
    const Ref<U, _OtherPtrTraits, OtherRefDerefTraits> &reference)
  {
#if KRYS_ASAN_ENABLED
    if (__asan_address_is_poisoned(this))
      __asan_unpoison_memory_region(this, sizeof(*this));
#endif
    Ref copiedReference = reference;
    swap(copiedReference);
    return *this;
  }

  template <typename X, typename APtrTraits, typename ARefDerefTraits, typename Y, typename BPtrTraits,
            typename BRefDerefTraits>
  inline bool operator==(const Ref<X, APtrTraits, ARefDerefTraits> &a,
                         const Ref<Y, BPtrTraits, BRefDerefTraits> &b)
  {
    return a._ptr == b._ptr;
  }

  template <typename X, typename _PtrTraits, typename RefDerefTraits>
  template <typename Y, typename _OtherPtrTraits, typename OtherRefDerefTraits>
  inline void Ref<X, _PtrTraits, RefDerefTraits>::swap(Ref<Y, _OtherPtrTraits, OtherRefDerefTraits> &other)
  {
    _PtrTraits::swap(_ptr, other._ptr);
  }

  template <typename X, typename APtrTraits, typename ARefDerefTraits, typename Y, typename BPtrTraits,
            typename BRefDerefTraits>
  requires(!std::same_as<APtrTraits, RawPtrTraits<X>> || !std::same_as<BPtrTraits, RawPtrTraits<Y>>)
  inline void swap(Ref<X, APtrTraits, ARefDerefTraits> &a, Ref<Y, BPtrTraits, BRefDerefTraits> &b)
  {
    a.swap(b);
  }

  template <typename X, typename _PtrTraits, typename RefDerefTraits>
  template <typename Y, typename _OtherPtrTraits, typename OtherRefDerefTraits>
  inline Ref<X, _PtrTraits, RefDerefTraits>
    Ref<X, _PtrTraits, RefDerefTraits>::replace(Ref<Y, _OtherPtrTraits, OtherRefDerefTraits> &&reference)
  {
#if KRYS_ASAN_ENABLED
    if (__asan_address_is_poisoned(this))
      __asan_unpoison_memory_region(this, sizeof(*this));
#endif
    auto oldReference = adoptRef(*_ptr);
    _ptr = &reference.leakRef();
    return oldReference;
  }

  template <typename X, typename _PtrTraits = RawPtrTraits<X>,
            typename RefDerefTraits = DefaultRefDerefTraits<X>, typename Y, typename _OtherPtrTraits,
            typename OtherRefDerefTraits>
  inline Ref<X, _PtrTraits, RefDerefTraits> upcast(Ref<Y, _OtherPtrTraits, OtherRefDerefTraits> &&reference)
  {
    static_assert(!std::same_as<Y, X>, "Unnecessary cast to same type");
    static_assert(std::derived_from<Y, X>, "Should be an upcast");
    return adoptRef(static_cast<X &>(reference.leakRef()));
  }

  template <typename X, typename _PtrTraits = RawPtrTraits<X>,
            typename RefDerefTraits = DefaultRefDerefTraits<X>, typename Y, typename _OtherPtrTraits,
            typename OtherRefDerefTraits>
  KRYS_ALWAYS_INLINE Ref<X, _PtrTraits, RefDerefTraits>
    upcast(const Ref<Y, _OtherPtrTraits, OtherRefDerefTraits> &reference)
  {
    static_assert(!std::same_as<Y, X>, "Unnecessary cast to same type");
    static_assert(std::derived_from<Y, X>, "Should be an upcast");
    return upcast<X, _PtrTraits, RefDerefTraits>(reference.copyRef());
  }

  template <typename X, typename _PtrTraits = RawPtrTraits<X>,
            typename RefDerefTraits = DefaultRefDerefTraits<X>, typename Y, typename _OtherPtrTraits,
            typename OtherRefDerefTraits>
  inline Ref<X, _PtrTraits, RefDerefTraits>
    unsafeRefDowncast(Ref<Y, _OtherPtrTraits, OtherRefDerefTraits> &&reference)
  {
    static_assert(!std::same_as<Y, X>, "Unnecessary cast to same type");
    static_assert(std::derived_from<X, Y>, "Use upcast instead");
    return adoptRef(static_cast<X &>(reference.leakRef()));
  }

  template <typename X, typename _PtrTraits = RawPtrTraits<X>,
            typename RefDerefTraits = DefaultRefDerefTraits<X>, typename Y, typename _OtherPtrTraits,
            typename OtherRefDerefTraits>
  KRYS_ALWAYS_INLINE Ref<X, _PtrTraits, RefDerefTraits>
    unsafeRefDowncast(const Ref<Y, _OtherPtrTraits, OtherRefDerefTraits> &reference)
  {
    static_assert(!std::same_as<Y, X>, "Unnecessary cast to same type");
    static_assert(std::derived_from<X, Y>, "Use upcast instead");
    return unsafeRefDowncast<X, _PtrTraits, RefDerefTraits>(reference.copyRef());
  }

  template <typename T, typename _PtrTraits, typename RefDerefTraits>
  struct GetPtrHelper<Ref<T, _PtrTraits, RefDerefTraits>>
  {
    using PtrType = T *;
    using UnderlyingType = T;
    static T *getPtr(const Ref<T, _PtrTraits, RefDerefTraits> &p)
    {
      return const_cast<T *>(p.ptr());
    }
  };

  template <typename T, typename _PtrTraits, typename RefDerefTraits>
  struct IsSmartPtr<Ref<T, _PtrTraits, RefDerefTraits>>
  {
    static constexpr bool value = true;
    static constexpr bool isNullable = false;
  };

  template <typename T, typename _PtrTraits, typename RefDerefTraits>
  inline Ref<T, _PtrTraits, RefDerefTraits> adoptRef(T &reference)
  {
    adopted(&reference);
    return Ref<T, _PtrTraits, RefDerefTraits>(reference, Ref<T, _PtrTraits, RefDerefTraits>::Adopt);
  }

  template <typename ExpectedType, typename ArgType, typename PtrTraits, typename RefDerefTraits>
  inline bool is(const Ref<ArgType, PtrTraits, RefDerefTraits> &source)
  {
    return is<ExpectedType>(source.get());
  }

  template <typename Target, typename Source, typename PtrTraits, typename RefDerefTraits>
  inline Ref<match_constness_t<Source, Target>, PtrTraits, RefDerefTraits>
    uncheckedDowncast(Ref<Source, PtrTraits, RefDerefTraits> source)
  {
    static_assert(!std::same_as<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::derived_from<Target, Source>, "Should be a downcast");
    assert(is<Target>(source));
    return unsafeRefDowncast<match_constness_t<Source, Target>>(Krys::Move(source));
  }

  template <typename Target, typename Source, typename PtrTraits, typename RefDerefTraits>
  inline Ref<match_constness_t<Source, Target>, PtrTraits, RefDerefTraits>
    downcast(Ref<Source, PtrTraits, RefDerefTraits> source)
  {
    static_assert(!std::same_as<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::derived_from<Target, Source>, "Should be a downcast");
    assert(is<Target>(source));
    return unsafeRefDowncast<match_constness_t<Source, Target>>(Krys::Move(source));
  }

  template <typename Target, typename Source, typename PtrTraits, typename RefDerefTraits>
  inline RefPtr<match_constness_t<Source, Target>, PtrTraits, RefDerefTraits>
    dynamicDowncast(Ref<Source, PtrTraits, RefDerefTraits> source)
  {
    static_assert(!std::same_as<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::derived_from<Target, Source>, "Should be a downcast");
    if (!is<Target>(source))
      return nullptr;
    return unsafeRefDowncast<match_constness_t<Source, Target>>(Krys::Move(source));
  }

  template <typename T, typename PtrTraits, typename RefDerefTraits>
  inline bool arePointingToEqualData(const Ref<T, PtrTraits, RefDerefTraits> &a,
                                     const Ref<T, PtrTraits, RefDerefTraits> &b)
  {
    return a.ptr() == b.ptr() || a.get() == b.get();
  }
}
