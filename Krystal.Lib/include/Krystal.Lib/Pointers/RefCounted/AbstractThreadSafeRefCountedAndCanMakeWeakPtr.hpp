#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefCounted/AbstractRefCounted.hpp"
#include "Krystal.Lib/Pointers/RefCounted/ThreadSafeWeakPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys
{
  class AbstractThreadSafeRefCountedAndCanMakeWeakPtr : public AbstractRefCounted
  {
  public:
    virtual ThreadSafeWeakPtrControlBlock &ControlBlock() const noexcept = 0;

  protected:
    KRYS_NODISCARD virtual uint32 WeakRefCount() const noexcept = 0;
  };
}

/// @brief Convenience macro that implements `AbstractThreadSafeRefCountedAndCanMakeWeakPtr` for you.
#define KRYS_ABSTRACT_THREAD_SAFE_REF_COUNTED_AND_CAN_MAKE_WEAK_PTR_IMPL                                     \
  void AddRef() const noexcept final                                                                         \
  {                                                                                                          \
    ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr::AddRef();                                               \
  }                                                                                                          \
  void SubRef() const noexcept final                                                                         \
  {                                                                                                          \
    ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr::SubRef();                                               \
  }                                                                                                          \
  ThreadSafeWeakPtrControlBlock &ControlBlock() const noexcept final                                         \
  {                                                                                                          \
    return ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr::ControlBlock();                                  \
  }                                                                                                          \
  KRYS_NODISCARD uint32 WeakRefCount() const noexcept final                                                  \
  {                                                                                                          \
    return ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr::WeakRefCount();                                  \
  }                                                                                                          \
  using _forceSemicolonAbstractThreadSafeRefCountedAndCanMakeWeakPtr = int
