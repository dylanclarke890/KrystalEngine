#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys::detail
{
  template <typename Derived>
  class WeakPtrImpl
  {
    KRYS_NON_COPYABLE(WeakPtrImpl);

  private:
    mutable uint32 _refCount {1};
    void *_ptr;

  public:
    template <typename T>
    constexpr explicit WeakPtrImpl(T *ptr) noexcept : _ptr(static_cast<typename T::weak_value *>(ptr))
    {
    }

    constexpr void AddRef() const noexcept
    {
      ++_refCount;
    }

    constexpr void SubRef() const noexcept
    {
      uint32 tempRefCount = _refCount - 1;
      if (!tempRefCount)
      {
        delete const_cast<Derived *>(static_cast<const Derived *>(this));
        return;
      }

      _refCount = tempRefCount;
    }

    KRYS_NODISCARD constexpr uint32 GetRefCount() const noexcept
    {
      return _refCount;
    }

    constexpr explicit operator bool() const noexcept
    {
      return _ptr;
    }

    template <typename T>
    KRYS_NODISCARD constexpr typename T::weak_value *get() noexcept
    {
      static_assert(IsTypeComplete<T>, "T is an incomplete type.");
      return static_cast<typename T::weak_value *>(_ptr);
    }

    constexpr void reset() noexcept
    {
      _ptr = nullptr;
    }
  };

  template <typename Derived>
  class ThreadSafeWeakPtrImpl : public ::krys::ThreadSafeRefCounted<Derived>
  {
    KRYS_NON_COPYABLE(ThreadSafeWeakPtrImpl);

  private:
    void *_ptr;

  public:
    template <typename T>
    constexpr explicit ThreadSafeWeakPtrImpl(T *ptr) noexcept
        : _ptr(static_cast<typename T::weak_value *>(ptr))
    {
    }

    constexpr explicit operator bool() const noexcept
    {
      return _ptr;
    }

    template <typename T>
    KRYS_NODISCARD constexpr typename T::weak_value *get() noexcept
    {
      return static_cast<typename T::weak_value *>(_ptr);
    }

    constexpr void reset() noexcept
    {
      _ptr = nullptr;
    }
  };
}

namespace krys
{
  class WeakPtrImpl final : public detail::WeakPtrImpl<WeakPtrImpl>
  {
  public:
    template <typename T>
    constexpr explicit WeakPtrImpl(T *ptr) noexcept : detail::WeakPtrImpl<WeakPtrImpl>(ptr)
    {
    }
  };

  class ThreadSafeWeakPtrImpl final : public detail::ThreadSafeWeakPtrImpl<ThreadSafeWeakPtrImpl>
  {
  public:
    template <typename T>
    constexpr explicit ThreadSafeWeakPtrImpl(T *ptr) noexcept
        : detail::ThreadSafeWeakPtrImpl<ThreadSafeWeakPtrImpl>(ptr)
    {
    }
  };
}