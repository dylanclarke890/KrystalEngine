#pragma once

#include "Krystal.Lib/Pointers/RefCounted/AbstractRefCounted.hpp"
#include "Krystal.Lib/Pointers/RefCounted/ThreadSafeWeakPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys
{
  class AbstractThreadSafeRefCountedAndCanMakeWeakPtr : public AbstractRefCounted
  {
    template <typename>
    friend class ThreadSafeWeakHashSet;

  public:
    virtual ThreadSafeWeakPtrControlBlock &controlBlock() const = 0;

  private:
    virtual uint32 weakRefCount() const = 0;
  };
}

// Convinience macro that implements AbstractThreadSafeRefCountedAndCanMakeWeakPtr for you.
#define WTF_ABSTRACT_THREAD_SAFE_REF_COUNTED_AND_CAN_MAKE_WEAK_PTR_IMPL                                      \
  void ref() const final                                                                                     \
  {                                                                                                          \
    ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr::ref();                                                  \
  }                                                                                                          \
  void deref() const final                                                                                   \
  {                                                                                                          \
    ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr::deref();                                                \
  }                                                                                                          \
  ThreadSafeWeakPtrControlBlock &controlBlock() const final                                                  \
  {                                                                                                          \
    return ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr::controlBlock();                                  \
  }                                                                                                          \
  uint32_t weakRefCount() const final                                                                        \
  {                                                                                                          \
    return ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr::weakRefCount();                                  \
  }                                                                                                          \
  using __Unused_type_for_semicolon = int
