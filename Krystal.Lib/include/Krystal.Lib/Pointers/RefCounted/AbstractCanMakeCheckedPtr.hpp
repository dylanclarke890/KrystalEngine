#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
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
    KRYS_NODISCARD virtual uint32 CheckedPtrCount() const noexcept = 0;
    virtual void IncrementCheckedPtrCount() const noexcept = 0;
    virtual void DecrementCheckedPtrCount() const noexcept = 0;
    virtual void SetDidBeginCheckedPtrDeletion() noexcept = 0;
  };
}

#define KRYS_OVERRIDE_ABSTRACT_CAN_MAKE_CHECKEDPTR(BaseClass)                                                \
  KRYS_NODISCARD Krys::uint32 CheckedPtrCount() const noexcept final                                         \
  {                                                                                                          \
    return BaseClass::CheckedPtrCount();                                                                     \
  }                                                                                                          \
  void IncrementCheckedPtrCount() const noexcept final                                                       \
  {                                                                                                          \
    BaseClass::IncrementCheckedPtrCount();                                                                   \
  }                                                                                                          \
  void DecrementCheckedPtrCount() const noexcept final                                                       \
  {                                                                                                          \
    BaseClass::DecrementCheckedPtrCount();                                                                   \
  }                                                                                                          \
  void SetDidBeginCheckedPtrDeletion() noexcept final                                                        \
  {                                                                                                          \
    BaseClass::SetDidBeginCheckedPtrDeletion();                                                              \
  }                                                                                                          \
  using _forceSemicolonAbstractCanMakeCheckedPtr = int
