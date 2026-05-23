#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::detail
{
  template <typename Derived>
  class WeakPtrImpl : public NonCopyable<WeakPtrImpl<Derived>>
  {
  private:
    mutable uint32 _refCount {1};
    RawPtr<void> _ptr;

  public:
    template <typename T>
    constexpr explicit WeakPtrImpl(RawPtr<T> ptr) noexcept
        : _ptr(static_cast<RawPtr<typename T::weak_value>>(ptr))
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
    KRYS_NODISCARD constexpr RawPtr<typename T::weak_value> get() noexcept
    {
      return static_cast<RawPtr<typename T::weak_value>>(_ptr);
    }

    constexpr void reset() noexcept
    {
      _ptr = nullptr;
    }
  };

  template <typename Derived>
  class ThreadSafeWeakPtrImpl : public ::Krys::ThreadSafeRefCounted<Derived>,
                                public NonCopyable<ThreadSafeWeakPtrImpl<Derived>>
  {
  private:
    RawPtr<void> _ptr;

  public:
    template <typename T>
    constexpr explicit ThreadSafeWeakPtrImpl(RawPtr<T> ptr) noexcept
        : _ptr(static_cast<RawPtr<typename T::weak_value>>(ptr))
    {
    }

    constexpr explicit operator bool() const noexcept
    {
      return _ptr;
    }

    template <typename T>
    KRYS_NODISCARD constexpr RawPtr<typename T::weak_value> get() noexcept
    {
      return static_cast<RawPtr<typename T::weak_value>>(_ptr);
    }

    constexpr void reset() noexcept
    {
      _ptr = nullptr;
    }
  };
}

namespace Krys
{
  class WeakPtrImpl final : public detail::WeakPtrImpl<WeakPtrImpl>
  {
  public:
    template <typename T>
    constexpr explicit WeakPtrImpl(RawPtr<T> ptr) noexcept : detail::WeakPtrImpl<WeakPtrImpl>(ptr)
    {
    }
  };

  class ThreadSafeWeakPtrImpl final : public detail::ThreadSafeWeakPtrImpl<ThreadSafeWeakPtrImpl>
  {
  public:
    template <typename T>
    constexpr explicit ThreadSafeWeakPtrImpl(RawPtr<T> ptr) noexcept
        : detail::ThreadSafeWeakPtrImpl<ThreadSafeWeakPtrImpl>(ptr)
    {
    }
  };
}