#pragma once

#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys
{
  // Use this class when an abstract base class needs CheckedPtr/CheckedRef support, and the
  // CanMakeCheckedPtr implementation will be in a concrete subclass.
  class AbstractCanMakeCheckedPtr
  {
  protected:
    virtual ~AbstractCanMakeCheckedPtr() noexcept = default;

  public:
    virtual uint32 CheckedPtrCount() const noexcept = 0;
    virtual void IncrementCheckedPtrCount() const noexcept = 0;
    virtual void DecrementCheckedPtrCount() const noexcept = 0;
    virtual void SetDidBeginCheckedPtrDeletion() noexcept = 0;
  };
}

#define KRYS_OVERRIDE_ABSTRACT_CAN_MAKE_CHECKEDPTR(BaseClass)                                                \
  Krys::uint32 CheckedPtrCount() const final                                                                 \
  {                                                                                                          \
    return BaseClass::CheckedPtrCount();                                                                     \
  }                                                                                                          \
  void IncrementCheckedPtrCount() const final                                                                \
  {                                                                                                          \
    BaseClass::IncrementCheckedPtrCount();                                                                   \
  }                                                                                                          \
  void DecrementCheckedPtrCount() const final                                                                \
  {                                                                                                          \
    BaseClass::DecrementCheckedPtrCount();                                                                   \
  }                                                                                                          \
  void SetDidBeginCheckedPtrDeletion() final                                                                 \
  {                                                                                                          \
    BaseClass::SetDidBeginCheckedPtrDeletion();                                                              \
  }                                                                                                          \
  using _forceSemicolonAbstractCanMakeCheckedPtr = int
