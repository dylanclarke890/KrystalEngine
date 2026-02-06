#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Detection/Compiler.hpp"
#include "Krystal.Lib/ForbidHeapAllocation.hpp"
#include <algorithm>
#include <utility>

namespace Krys
{
  template <typename T>
  inline T &weakRef(T &ref)
  {
    ref.weakRef();
    return ref;
  }

  template <typename T>
  inline T *weakRefIfNotNull(T *ptr)
  {
    if (ptr) [[likely]]
      ptr->weakRef();
    return ptr;
  }

  template <typename T>
  inline void weakDerefIfNotNull(T *ptr)
  {
    if (ptr) [[likely]]
      ptr->weakDeref();
  }

  template <typename T>
  class InlineWeakRef
  {
    KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW;

  public:
    KRYS_ALWAYS_INLINE InlineWeakRef(T &ptr) : _ptr(&weakRef(ptr))
    {
    }
    KRYS_ALWAYS_INLINE InlineWeakRef(const InlineWeakRef &o) : _ptr(weakRef(*o._ptr))
    {
    }
    template <typename X>
    InlineWeakRef(const InlineWeakRef<X> &o) : _ptr(weakRef(*o._ptr))
    {
    }

    KRYS_ALWAYS_INLINE InlineWeakRef(InlineWeakRef &&o) : _ptr(&o.leakWeak())
    {
    }
    template <typename X>
    InlineWeakRef(InlineWeakRef<X> &&o) : _ptr(&o.leakWeak())
    {
    }

    KRYS_ALWAYS_INLINE ~InlineWeakRef()
    {
      weakDerefIfNotNull(_ptr);
    }

    T &get() const KRYS_LIFETIME_BOUND;
    T *ptr() const KRYS_LIFETIME_BOUND;

    KRYS_ALWAYS_INLINE T *operator->() const KRYS_LIFETIME_BOUND
    {
      return ptr();
    }

    InlineWeakRef &operator=(T &);

    template <typename X>
    void swap(InlineWeakRef<X> &);

  private:
    template <typename X>
    friend class InlineWeakRef;

    KRYS_NODISCARD T &leakWeak()
    {
      assert(_ptr);
      return *std::exchange(_ptr, nullptr);
    }

    T *_ptr;
  };

  template <typename T>
  T &InlineWeakRef<T>::get() const KRYS_LIFETIME_BOUND
  {
    assert(_ptr->refCount());
    return *_ptr;
  }

  template <typename T>
  T *InlineWeakRef<T>::ptr() const KRYS_LIFETIME_BOUND
  {
    assert(_ptr->refCount());
    return _ptr;
  }

  template <typename T>
  inline InlineWeakRef<T> &InlineWeakRef<T>::operator=(T &optr)
  {
    InlineWeakRef ptr = optr;
    swap(ptr);
    return *this;
  }

  template <class T>
  template <typename X>
  inline void InlineWeakRef<T>::swap(InlineWeakRef<X> &o)
  {
    std::swap(_ptr, o._ptr);
  }
}
