#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Detection/Compiler.hpp"
#include "Krystal.Lib/ForbidHeapAllocation.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include <algorithm>
#include <utility>

namespace Krys
{
  template <typename T>
  constexpr inline T &AddRefWeak(T &ref) noexcept
  {
    ref.AddRefWeak();
    return ref;
  }

  template <typename T>
  constexpr inline RawPtr<T> AddRefWeak(RawPtr<T> ptr) noexcept
  {
    if (ptr) KRYS_LIKELY
    {
      ptr->AddRefWeak();
    }

    return ptr;
  }

  template <typename T>
  inline void SubRefWeak(RawPtr<T> ptr) noexcept
  {
    if (ptr) KRYS_LIKELY
    {
      ptr->SubRefWeak();
    }
  }

  template <typename T>
  class InlineWeakRef
  {
    KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW;

    template <typename X>
    friend class InlineWeakRef;

  private:
    RawPtr<T> _ptr;

  public:
    KRYS_ALWAYS_INLINE constexpr InlineWeakRef(T &ptr) noexcept : _ptr(&AddRefWeak(ptr))
    {
    }

    KRYS_ALWAYS_INLINE constexpr InlineWeakRef(const InlineWeakRef &o) noexcept : _ptr(AddRefWeak(*o._ptr))
    {
    }

    template <typename X>
    constexpr InlineWeakRef(const InlineWeakRef<X> &o) noexcept : _ptr(AddRefWeak(*o._ptr))
    {
    }

    KRYS_ALWAYS_INLINE constexpr InlineWeakRef(InlineWeakRef &&o) noexcept : _ptr(&o.LeakWeak())
    {
    }

    template <typename X>
    constexpr InlineWeakRef(InlineWeakRef<X> &&o) noexcept : _ptr(&o.LeakWeak())
    {
    }

    KRYS_ALWAYS_INLINE constexpr ~InlineWeakRef() noexcept
    {
      SubRefWeak(_ptr);
    }

    constexpr T &get() const noexcept KRYS_LIFETIME_BOUND
    {
      assert(_ptr->GetRefCount());
      return *_ptr;
    }

    constexpr RawPtr<T> ptr() const noexcept KRYS_LIFETIME_BOUND
    {
      assert(_ptr->GetRefCount());
      return _ptr;
    }

    KRYS_ALWAYS_INLINE constexpr T *operator->() const noexcept KRYS_LIFETIME_BOUND
    {
      return ptr();
    }

    constexpr InlineWeakRef &operator=(T &optr) noexcept
    {
      InlineWeakRef ptr = optr;
      swap(ptr);
      return *this;
    }

    template <typename X>
    constexpr void swap(InlineWeakRef<X> &o) noexcept
    {
      std::swap(_ptr, o._ptr);
    }

  private:
    KRYS_NODISCARD constexpr T &LeakWeak() noexcept
    {
      assert(_ptr);
      return *std::exchange(_ptr, nullptr);
    }
  };
}
