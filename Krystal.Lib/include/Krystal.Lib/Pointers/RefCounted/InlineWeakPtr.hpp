#pragma once

#include "Krystal.Lib/Pointers/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/InlineWeakRef.hpp"
#include <bit>

namespace Krys
{
  template <typename T>
  class InlineWeakPtr
  {
    KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW;

  public:
    KRYS_ALWAYS_INLINE constexpr InlineWeakPtr() : _ptr(nullptr)
    {
    }
    KRYS_ALWAYS_INLINE constexpr InlineWeakPtr(std::nullptr_t) : _ptr(nullptr)
    {
    }
    KRYS_ALWAYS_INLINE InlineWeakPtr(T *ptr) : _ptr(weakRefIfNotNull(ptr))
    {
    }
    KRYS_ALWAYS_INLINE InlineWeakPtr(T &ptr) : _ptr(&weakRef(ptr))
    {
    }
    KRYS_ALWAYS_INLINE InlineWeakPtr(const InlineWeakPtr &o) : _ptr(weakRefIfNotNull(o._ptr))
    {
    }
    template <typename X>
    InlineWeakPtr(const InlineWeakPtr<X> &o) : _ptr(weakRefIfNotNull(o._ptr))
    {
    }

    KRYS_ALWAYS_INLINE InlineWeakPtr(InlineWeakPtr &&o) : _ptr(o.leakWeak())
    {
    }
    template <typename X>
    InlineWeakPtr(InlineWeakPtr<X> &&o) : _ptr(o.leakWeak())
    {
    }

    static T *hashTableDeletedValue()
    {
      return std::bit_cast<T *>(static_cast<uintptr_t>(-1));
    }

    InlineWeakPtr(HashTableDeletedValueType) : _ptr(hashTableDeletedValue())
    {
    }

    InlineWeakPtr(HashTableEmptyValueType) : _ptr(nullptr)
    {
    }

    KRYS_ALWAYS_INLINE ~InlineWeakPtr()
    {
      weakDerefIfNotNull(_ptr);
    }

    bool isHashTableDeletedValue() const
    {
      return _ptr == hashTableDeletedValue();
    }
    bool isHashTableEmptyValue() const
    {
      return !_ptr;
    }
    bool isWeakNullValue() const
    {
      return !_ptr->refCount();
    }

    T *get() const KRYS_LIFETIME_BOUND;

    KRYS_NODISCARD T *leakWeak();

    T &operator*() const KRYS_LIFETIME_BOUND
    {
      assert(get());
      return *get();
    }
    KRYS_ALWAYS_INLINE T *operator->() const KRYS_LIFETIME_BOUND
    {
      return get();
    }

    bool operator!() const
    {
      return !_ptr || !_ptr->refCount();
    }

    explicit operator bool() const
    {
      return _ptr && _ptr->refCount();
    }

    InlineWeakPtr &operator=(T *);
    InlineWeakPtr &operator=(std::nullptr_t);
    InlineWeakPtr &operator=(const InlineWeakPtr &);
    InlineWeakPtr &operator=(InlineWeakPtr &&);

    template <typename X>
    void swap(InlineWeakPtr<X> &);

  private:
    template <typename X>
    friend class InlineWeakPtr;

    T *_ptr;
  };

  template <typename T>
  T *InlineWeakPtr<T>::get() const KRYS_LIFETIME_BOUND
  {
    if (!_ptr)
      return nullptr;
    if (!_ptr->refCount())
      return nullptr;
    return _ptr;
  }

  template <typename T>
  inline T *InlineWeakPtr<T>::leakWeak()
  {
    return std::exchange(_ptr, nullptr);
  }

  template <typename T>
  inline InlineWeakPtr<T> &InlineWeakPtr<T>::operator=(T *optr)
  {
    InlineWeakPtr ptr = optr;
    swap(ptr);
    return *this;
  }

  template <typename T>
  inline InlineWeakPtr<T> &InlineWeakPtr<T>::operator=(std::nullptr_t)
  {
    weakDerefIfNotNull(std::exchange(_ptr, nullptr));
    return *this;
  }

  template <typename T>
  inline InlineWeakPtr<T> &InlineWeakPtr<T>::operator=(const InlineWeakPtr &o)
  {
    InlineWeakPtr ptr = o;
    swap(ptr);
    return *this;
  }

  template <typename T>
  inline InlineWeakPtr<T> &InlineWeakPtr<T>::operator=(InlineWeakPtr &&o)
  {
    InlineWeakPtr ptr = Krys::Move(o);
    swap(ptr);
    return *this;
  }

  template <class T>
  template <typename X>
  inline void InlineWeakPtr<T>::swap(InlineWeakPtr<X> &o)
  {
    std::swap(_ptr, o._ptr);
  }

  template <typename T, typename U>
  inline bool operator==(const InlineWeakPtr<T> &a, const InlineWeakPtr<U> &b)
  {
    return a.get() == b.get();
  }

  template <typename T, typename U>
  inline bool operator==(const InlineWeakPtr<T> &a, U *b)
  {
    return a.get() == b;
  }

  template <typename T>
  struct GetPtrHelper<InlineWeakPtr<T>>
  {
    using pointer_type = RawPtr<T>;
    using underlying_type = T;

    static pointer_type GetPtr(const InlineWeakPtr<T> &p)
    {
      return const_cast<pointer_type>(p.get());
    }
  };

  template <typename T>
  struct IsSmartPtr<InlineWeakPtr<T>>
  {
    static constexpr bool value = true;
    static constexpr bool nullable = true;
  };

  template <typename P>
  struct InlineWeakPtrHashTraits : public SimpleClassHashTraits<InlineWeakPtr<P>>
  {
    static constexpr bool emptyValueIsZero = true;
    static RawPtr<P> emptyValue()
    {
      return nullptr;
    }

    template <typename>
    static void constructEmptyValue(InlineWeakPtr<P> &slot)
    {
      new (NotNull, std::addressof(slot)) InlineWeakPtr<P>();
    }

    static constexpr bool hasIsEmptyValueFunction = true;
    static bool isEmptyValue(const InlineWeakPtr<P> &value)
    {
      return value.isHashTableEmptyValue();
    }

    static constexpr bool hasIsWeakNullValueFunction = true;
    static bool isWeakNullValue(const InlineWeakPtr<P> &value)
    {
      return value.isWeakNullValue();
    }

    using PeekType = RawPtr<P>;
    static PeekType peek(const InlineWeakPtr<P> &value)
    {
      return const_cast<PeekType>(value.get());
    }
    static PeekType peek(P *value)
    {
      return value;
    }

    using TakeType = InlineWeakPtr<P>;
    static TakeType take(InlineWeakPtr<P> &&value)
    {
      return isEmptyValue(value) ? nullptr : InlineWeakPtr<P>(Krys::Move(value));
    }
  };
}
