#pragma once

#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Pointers/RefCounted/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/ThreadSafeRefCounted.hpp"

namespace Krys
{
  template <typename Derived>
  class WeakPtrImplBase : public ThreadSafeRefCounted<Derived>, public NonCopyable<WeakPtrImplBase<Derived>>
  {
  public:
    template <typename T>
    typename T::WeakValueType *get()
    {
      return static_cast<typename T::WeakValueType *>(_ptr);
    }

    explicit operator bool() const
    {
      return _ptr;
    }
    void clear()
    {
      _ptr = nullptr;
    }

#if KRYS_ENV(DEV)
    bool wasConstructedOnMainThread() const
    {
      return m_wasConstructedOnMainThread;
    }
#endif

    template <typename T>
    explicit WeakPtrImplBase(T *ptr)
        : _ptr(static_cast<typename T::WeakValueType *>(ptr))
#if KRYS_ENV(DEV)
          ,
          m_wasConstructedOnMainThread(isMainThread())
#endif
    {
    }

  private:
    void *_ptr;
#if KRYS_ENV(DEV)
    bool m_wasConstructedOnMainThread;
#endif
  };

  class DefaultWeakPtrImpl final : public WeakPtrImplBase<DefaultWeakPtrImpl>
  {
  public:
    template <typename T>
    explicit DefaultWeakPtrImpl(T *ptr) : WeakPtrImplBase<DefaultWeakPtrImpl>(ptr)
    {
    }
  };

  template <typename Derived>
  class WeakPtrImplBaseSingleThread : public NonCopyable<WeakPtrImplBaseSingleThread<Derived>>
  {
  public:
    template <typename T>
    typename T::WeakValueType *get()
    {
      return static_cast<typename T::WeakValueType *>(_ptr);
    }

    explicit operator bool() const
    {
      return _ptr;
    }
    void clear()
    {
      _ptr = nullptr;
    }

#if KRYS_ENV(DEV)
    bool wasConstructedOnMainThread() const
    {
      return m_wasConstructedOnMainThread;
    }
#endif

    template <typename T>
    explicit WeakPtrImplBaseSingleThread(T *ptr)
        : _ptr(static_cast<typename T::WeakValueType *>(ptr))
#if KRYS_ENV(DEV)
          ,
          m_wasConstructedOnMainThread(isMainThread())
#endif
    {
    }

    uint32_t refCount() const
    {
      return m_refCount;
    }
    void ref() const
    {
      ++m_refCount;
    }
    void deref() const
    {
      uint32_t tempRefCount = m_refCount - 1;
      if (!tempRefCount)
      {
        delete const_cast<Derived *>(static_cast<const Derived *>(this));
        return;
      }
      m_refCount = tempRefCount;
    }

  private:
    mutable uint32_t m_refCount {1};
    void *_ptr;
#if KRYS_ENV(DEV)
    bool m_wasConstructedOnMainThread;
#endif
  };

  class SingleThreadWeakPtrImpl final : public WeakPtrImplBaseSingleThread<SingleThreadWeakPtrImpl>
  {
  public:
    template <typename T>
    explicit SingleThreadWeakPtrImpl(T *ptr) : WeakPtrImplBaseSingleThread<SingleThreadWeakPtrImpl>(ptr)
    {
    }
  };
}
