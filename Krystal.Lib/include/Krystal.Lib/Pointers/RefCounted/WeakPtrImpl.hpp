#pragma once

#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/ThreadSafeRefCounted.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys
{
  template <typename Derived>
  class WeakPtrImplBase : public ThreadSafeRefCounted<Derived>, public NonCopyable<WeakPtrImplBase<Derived>>
  {
  private:
    RawPtr<void> _ptr;

  public:
    template <typename T>
    explicit WeakPtrImplBase(RawPtr<T> ptr) noexcept : _ptr(static_cast<RawPtr<typename T::TWeakValue>>(ptr))
    {
    }

    template <typename T>
    RawPtr<typename T::TWeakValue> get() noexcept
    {
      return static_cast<RawPtr<typename T::TWeakValue>>(_ptr);
    }

    void Clear() noexcept
    {
      _ptr = nullptr;
    }

    explicit operator bool() const noexcept
    {
      return _ptr;
    }
  };

  class DefaultWeakPtrImpl final : public WeakPtrImplBase<DefaultWeakPtrImpl>
  {
  public:
    template <typename T>
    explicit DefaultWeakPtrImpl(RawPtr<T> ptr) noexcept : WeakPtrImplBase<DefaultWeakPtrImpl>(ptr)
    {
    }
  };

  template <typename Derived>
  class WeakPtrImplBaseSingleThread : public NonCopyable<WeakPtrImplBaseSingleThread<Derived>>
  {
  private:
    mutable uint32 _refCount {1};
    RawPtr<void> _ptr;

  public:
    template <typename T>
    explicit WeakPtrImplBaseSingleThread(RawPtr<T> ptr) noexcept
        : _ptr(static_cast<RawPtr<typename T::TWeakValue>>(ptr))
    {
    }

    void AddRef() const noexcept
    {
      ++_refCount;
    }

    void SubRef() const noexcept
    {
      uint32 tempRefCount = _refCount - 1;
      if (!tempRefCount)
      {
        delete const_cast<Derived *>(static_cast<const Derived *>(this));
        return;
      }
      _refCount = tempRefCount;
    }

    uint32_t GetRefCount() const noexcept
    {
      return _refCount;
    }

    template <typename T>
    RawPtr<typename T::TWeakValue> get() noexcept
    { 
      return static_cast<RawPtr<typename T::TWeakValue>>(_ptr);
    }

    void Clear() noexcept
    {
      _ptr = nullptr;
    }

    explicit operator bool() const noexcept
    {
      return _ptr;
    }
  };

  class SingleThreadWeakPtrImpl final : public WeakPtrImplBaseSingleThread<SingleThreadWeakPtrImpl>
  {
  public:
    template <typename T>
    explicit SingleThreadWeakPtrImpl(RawPtr<T> ptr) noexcept
        : WeakPtrImplBaseSingleThread<SingleThreadWeakPtrImpl>(ptr)
    {
    }
  };
}
