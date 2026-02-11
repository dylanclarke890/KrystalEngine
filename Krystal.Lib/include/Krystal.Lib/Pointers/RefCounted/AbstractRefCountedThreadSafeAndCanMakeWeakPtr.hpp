#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefCounted/ThreadSafeWeakPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys
{
  class AbstractRefCountedThreadSafeAndCanMakeWeakPtr : public AbstractRefCounted
  {
  public:
    KRYS_NODISCARD virtual ThreadSafeWeakPtrControlBlock &GetControlBlock() const noexcept = 0;
    KRYS_NODISCARD virtual uint32 GetRefCountWeak() const noexcept = 0;
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
  KRYS_NODISCARD ThreadSafeWeakPtrControlBlock &GetControlBlock() const noexcept final                       \
  {                                                                                                          \
    return ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr::GetControlBlock();                               \
  }                                                                                                          \
  KRYS_NODISCARD uint32 GetRefCountWeak() const noexcept final                                               \
  {                                                                                                          \
    return ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr::GetRefCountWeak();                               \
  }                                                                                                          \
  using _forceSemicolonAbstractThreadSafeRefCountedAndCanMakeWeakPtr = int
