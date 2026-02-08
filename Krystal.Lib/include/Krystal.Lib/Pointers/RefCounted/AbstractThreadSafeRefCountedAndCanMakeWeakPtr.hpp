#pragma once

#include "Krystal.Lib/Pointers/RefCounted/AbstractRefCounted.hpp"
#include "Krystal.Lib/Pointers/RefCounted/ThreadSafeWeakPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys
{
  class AbstractThreadSafeRefCountedAndCanMakeWeakPtr : public AbstractRefCounted
  {
  public:
    virtual ThreadSafeWeakPtrControlBlock &ControlBlock() const = 0;

  protected:
    virtual uint32 WeakRefCount() const = 0;
  };
}

/// @brief Convenience macro that implements `AbstractThreadSafeRefCountedAndCanMakeWeakPtr` for you.
#define KRYS_ABSTRACT_THREAD_SAFE_REF_COUNTED_AND_CAN_MAKE_WEAK_PTR_IMPL                                     \
  void AddRef() const final                                                                                  \
  {                                                                                                          \
    ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr::AddRef();                                               \
  }                                                                                                          \
  void SubRef() const final                                                                                  \
  {                                                                                                          \
    ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr::SubRef();                                               \
  }                                                                                                          \
  ThreadSafeWeakPtrControlBlock &ControlBlock() const final                                                  \
  {                                                                                                          \
    return ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr::ControlBlock();                                  \
  }                                                                                                          \
  uint32 WeakRefCount() const final                                                                          \
  {                                                                                                          \
    return ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr::WeakRefCount();                                  \
  }                                                                                                          \
  using _forceSemicolonAbstractThreadSafeRefCountedAndCanMakeWeakPtr = int
