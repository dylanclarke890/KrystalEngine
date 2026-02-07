#pragma once

#include "Krystal.Lib/Pointers/RefCounted/RawPtrTraits.hpp"
#include "Krystal.Lib/Pointers/RefCounted/Ref.hpp"
#include <algorithm>
#include <utility>

namespace Krys
{
  template <typename T, typename PtrTraits, typename RefDerefTraits>
  class RefPtr;
  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  RefPtr<T, PtrTraits, RefDerefTraits> adoptRef(T *);

  template <typename T, typename _PtrTraits = RawPtrTraits<T>,
            typename _RefDerefTraits = DefaultRefDerefTraits<T>>
  class RefPtr
  {
    KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW;

  public:
    using PtrTraits = _PtrTraits;
    using RefDerefTraits = _RefDerefTraits;
    typedef T ValueType;
    typedef ValueType *PtrType;

    static constexpr bool isRefPtr = true;

    KRYS_ALWAYS_INLINE constexpr RefPtr() : _ptr(nullptr)
    {
    }
    KRYS_ALWAYS_INLINE constexpr RefPtr(std::nullptr_t) : _ptr(nullptr)
    {
    }
    KRYS_ALWAYS_INLINE RefPtr(T *ptr) : _ptr(RefDerefTraits::refIfNotNull(ptr))
    {
    }
    KRYS_ALWAYS_INLINE RefPtr(T &ptr) : _ptr(&RefDerefTraits::ref(ptr))
    {
    }
    KRYS_ALWAYS_INLINE RefPtr(const RefPtr &o) : _ptr(RefDerefTraits::refIfNotNull(PtrTraits::unwrap(o._ptr)))
    {
    }
    template <typename X, typename Y, typename Z>
    RefPtr(const RefPtr<X, Y, Z> &o) : _ptr(RefDerefTraits::refIfNotNull(PtrTraits::unwrap(o.get())))
    {
    }

    KRYS_ALWAYS_INLINE RefPtr(RefPtr &&o) : _ptr(o.leakRef())
    {
    }
    template <typename X, typename Y, typename Z>
    RefPtr(RefPtr<X, Y, Z> &&o) : _ptr(o.leakRef())
    {
    }
    template <typename X, typename Y>
    RefPtr(Ref<X, Y> &&);

    // Hash table deleted values, which are only constructed and never copied or destroyed.
    RefPtr(HashTableDeletedValueType) : _ptr(PtrTraits::hashTableDeletedValue())
    {
    }
    bool isHashTableDeletedValue() const
    {
      return PtrTraits::isHashTableDeletedValue(_ptr);
    }

    RefPtr(HashTableEmptyValueType) : _ptr(hashTableEmptyValue())
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

    KRYS_ALWAYS_INLINE ~RefPtr()
    {
      RefDerefTraits::derefIfNotNull(PtrTraits::exchange(_ptr, nullptr));
    }

    T *get() const KRYS_LIFETIME_BOUND
    {
      return PtrTraits::unwrap(_ptr);
    }
    T *unsafeGet() const
    {
      return PtrTraits::unwrap(_ptr);
    } // FIXME: Replace with get() then remove.

    Ref<T> releaseNonNull()
    {
      assert(_ptr);
      Ref<T> tmp(adoptRef(*_ptr));
      _ptr = nullptr;
      return tmp;
    }

    KRYS_NODISCARD T *leakRef();

    KRYS_ALWAYS_INLINE T &operator*() const KRYS_LIFETIME_BOUND
    {
      assert(_ptr);
      return *PtrTraits::unwrap(_ptr);
    }
    KRYS_ALWAYS_INLINE T *operator->() const KRYS_LIFETIME_BOUND
    {
      return &**this;
    }

    bool operator!() const
    {
      return !_ptr;
    }

    // This conversion operator allows implicit conversion to bool but not to other integer types.
    using UnspecifiedBoolType = void (RefPtr::*)() const;
    operator UnspecifiedBoolType() const
    {
      return _ptr ? &RefPtr::unspecifiedBoolTypeInstance : nullptr;
    }

    explicit operator bool() const
    {
      return !!_ptr;
    }

    RefPtr &operator=(const RefPtr &);
    RefPtr &operator=(T *);
    RefPtr &operator=(std::nullptr_t);
    template <typename X, typename Y, typename Z>
    RefPtr &operator=(const RefPtr<X, Y, Z> &);
    RefPtr &operator=(RefPtr &&);
    template <typename X, typename Y, typename Z>
    RefPtr &operator=(RefPtr<X, Y, Z> &&);
    template <typename X>
    RefPtr &operator=(Ref<X> &&);

    template <typename X, typename Y, typename Z>
    void swap(RefPtr<X, Y, Z> &);

    RefPtr copyRef() && = delete;
    KRYS_NODISCARD RefPtr copyRef() const &
    {
      return RefPtr(_ptr);
    }

  private:
    void unspecifiedBoolTypeInstance() const
    {
    }

    friend RefPtr adoptRef<T, PtrTraits, RefDerefTraits>(T *);
    template <typename X, typename Y, typename Z>
    friend class RefPtr;

    template <typename T1, typename U, typename V, typename X, typename Y, typename Z>
    friend bool operator==(const RefPtr<T1, U, V> &, const RefPtr<X, Y, Z> &);
    template <typename T1, typename U, typename V, typename X>
    friend bool operator==(const RefPtr<T1, U, V> &, X *);

    enum AdoptTag
    {
      Adopt
    };
    RefPtr(T *ptr, AdoptTag) : _ptr(ptr)
    {
    }

    typename PtrTraits::StorageType _ptr;
  };

  // Template deduction guide.
  template <typename X, typename Y>
  RefPtr(Ref<X, Y> &&) -> RefPtr<X, Y, DefaultRefDerefTraits<X>>;

  template <typename T, typename U, typename V>
  template <typename X, typename Y>
  inline RefPtr<T, U, V>::RefPtr(Ref<X, Y> &&reference) : _ptr(&reference.leakRef())
  {
  }

  template <typename T, typename U, typename V>
  inline T *RefPtr<T, U, V>::leakRef()
  {
    return U::exchange(_ptr, nullptr);
  }

  template <typename T, typename U, typename V>
  inline RefPtr<T, U, V> &RefPtr<T, U, V>::operator=(const RefPtr &o)
  {
    RefPtr ptr = o;
    swap(ptr);
    return *this;
  }

  template <typename T, typename U, typename V>
  template <typename X, typename Y, typename Z>
  inline RefPtr<T, U, V> &RefPtr<T, U, V>::operator=(const RefPtr<X, Y, Z> &o)
  {
    RefPtr ptr = o;
    swap(ptr);
    return *this;
  }

  template <typename T, typename U, typename V>
  inline RefPtr<T, U, V> &RefPtr<T, U, V>::operator=(T *optr)
  {
    RefPtr ptr = optr;
    swap(ptr);
    return *this;
  }

  template <typename T, typename U, typename V>
  inline RefPtr<T, U, V> &RefPtr<T, U, V>::operator=(std::nullptr_t)
  {
    V::derefIfNotNull(U::exchange(_ptr, nullptr));
    return *this;
  }

  template <typename T, typename U, typename V>
  inline RefPtr<T, U, V> &RefPtr<T, U, V>::operator=(RefPtr &&o)
  {
    RefPtr ptr = Krys::Move(o);
    swap(ptr);
    return *this;
  }

  template <typename T, typename U, typename V>
  template <typename X, typename Y, typename Z>
  inline RefPtr<T, U, V> &RefPtr<T, U, V>::operator=(RefPtr<X, Y, Z> &&o)
  {
    RefPtr ptr = Krys::Move(o);
    swap(ptr);
    return *this;
  }

  template <typename T, typename V, typename W>
  template <typename U>
  inline RefPtr<T, V, W> &RefPtr<T, V, W>::operator=(Ref<U> &&reference)
  {
    RefPtr ptr = Krys::Move(reference);
    swap(ptr);
    return *this;
  }

  template <class T, typename U, typename V>
  template <typename X, typename Y, typename Z>
  inline void RefPtr<T, U, V>::swap(RefPtr<X, Y, Z> &o)
  {
    U::swap(_ptr, o._ptr);
  }

  template <typename T, typename U, typename V>
  inline void swap(RefPtr<T, U, V> &a, RefPtr<T, U, V> &b)
  {
    a.swap(b);
  }

  template <typename T, typename U, typename V, typename X, typename Y, typename Z>
  inline bool operator==(const RefPtr<T, U, V> &a, const RefPtr<X, Y, Z> &b)
  {
    return a._ptr == b._ptr;
  }

  template <typename T, typename U, typename V, typename X>
  inline bool operator==(const RefPtr<T, U, V> &a, X *b)
  {
    return a._ptr == b;
  }

  template <typename T, typename U, typename V>
  inline RefPtr<T, U, V> adoptRef(T *p)
  {
    adopted(p);
    return RefPtr<T, U, V>(p, RefPtr<T, U, V>::Adopt);
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  inline RefPtr<T, U, V> upcast(const RefPtr<X, Y, Z> &p)
  {
    static_assert(!std::same_as<X, T>, "Unnecessary cast to same type");
    static_assert(std::derived_from<X, T>, "Should be an upcast");
    return RefPtr<T, U, V>(static_cast<T *>(p.get()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  inline RefPtr<T, U, V> upcast(RefPtr<X, Y, Z> &&p)
  {
    static_assert(!std::same_as<X, T>, "Unnecessary cast to same type");
    static_assert(std::derived_from<X, T>, "Should be an upcast");
    return adoptRef(static_cast<T *>(p.leakRef()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  inline RefPtr<T, U, V> unsafeRefPtrDowncast(const RefPtr<X, Y, Z> &p)
  {
    static_assert(!std::same_as<X, T>, "Unnecessary cast to same type");
    static_assert(std::derived_from<T, X>, "Use upcast instead");
    return RefPtr<T, U, V>(static_cast<T *>(p.get()));
  }

  template <typename T, typename U = RawPtrTraits<T>, typename V = DefaultRefDerefTraits<T>, typename X,
            typename Y, typename Z>
  inline RefPtr<T, U, V> unsafeRefPtrDowncast(RefPtr<X, Y, Z> &&p)
  {
    static_assert(!std::same_as<X, T>, "Unnecessary cast to same type");
    static_assert(std::derived_from<T, X>, "Use upcast instead");
    return adoptRef(static_cast<T *>(p.leakRef()));
  }

  template <typename T, typename U, typename V>
  struct IsSmartPtr<RefPtr<T, U, V>>
  {
    static constexpr bool value = true;
    static constexpr bool isNullable = true;
  };

  template <typename ExpectedType, typename ArgType, typename PtrTraits, typename RefDerefTraits>
  inline bool is(const RefPtr<ArgType, PtrTraits, RefDerefTraits> &source)
  {
    return is<ExpectedType>(source.get());
  }

  template <typename Target, typename Source, typename PtrTraits, typename RefDerefTraits>
  inline RefPtr<match_constness_t<Source, Target>, PtrTraits, RefDerefTraits>
    uncheckedDowncast(RefPtr<Source, PtrTraits, RefDerefTraits> source)
  {
    static_assert(!std::same_as<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::derived_from<Target, Source>, "Should be a downcast");
    assert(!source || is<Target>(*source));
    return unsafeRefPtrDowncast<match_constness_t<Source, Target>>(Krys::Move(source));
  }

  template <typename Target, typename Source, typename PtrTraits, typename RefDerefTraits>
  inline RefPtr<match_constness_t<Source, Target>, PtrTraits, RefDerefTraits>
    downcast(RefPtr<Source, PtrTraits, RefDerefTraits> source)
  {
    static_assert(!std::same_as<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::derived_from<Target, Source>, "Should be a downcast");
    assert(!source || is<Target>(*source));
    return unsafeRefPtrDowncast<match_constness_t<Source, Target>>(Krys::Move(source));
  }

  template <typename Target, typename Source, typename TargetPtrTraits = RawPtrTraits<Target>,
            typename TargetRefDerefTraits = DefaultRefDerefTraits<Target>, typename SourcePtrTraits,
            typename SourceRefDerefTraits>
  inline RefPtr<match_constness_t<Source, Target>, TargetPtrTraits, TargetRefDerefTraits>
    dynamicDowncast(RefPtr<Source, SourcePtrTraits, SourceRefDerefTraits> source)
  {
    static_assert(!std::same_as<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::derived_from<Target, Source>, "Should be a downcast");
    if (!is<Target>(source))
      return nullptr;
    return unsafeRefPtrDowncast<match_constness_t<Source, Target>, TargetPtrTraits, TargetRefDerefTraits>(
      Krys::Move(source));
  }

  template <typename T, typename U>
  KRYS_ALWAYS_INLINE void lazyInitialize(const RefPtr<T> &ptr, Ref<U> &&obj)
  {
    assert(!ptr);
    const_cast<RefPtr<T> &>(ptr) = Krys::Move(obj);
  }
}