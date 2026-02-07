#pragma once

#include <cstdint>

namespace Krys
{
  // Use this class when an abstract base class needs CheckedPtr/CheckedRef support, and the
  // CanMakeCheckedPtr implementation will be in a concrete subclass.
  class AbstractCanMakeCheckedPtr
  {
  protected:
    virtual ~AbstractCanMakeCheckedPtr() = default;

  public:
    virtual uint32_t checkedPtrCount() const = 0;
    virtual uint32_t checkedPtrCountWithoutThreadCheck() const = 0;
    virtual void incrementCheckedPtrCount() const = 0;
    virtual void decrementCheckedPtrCount() const = 0;
    virtual void setDidBeginCheckedPtrDeletion() = 0;
  };
}

#define OVERRIDE_ABSTRACT_CAN_MAKE_CHECKEDPTR(BaseClass)                                                     \
  uint32_t checkedPtrCount() const final                                                                     \
  {                                                                                                          \
    return BaseClass::checkedPtrCount();                                                                     \
  }                                                                                                          \
  uint32_t checkedPtrCountWithoutThreadCheck() const final                                                   \
  {                                                                                                          \
    return BaseClass::checkedPtrCountWithoutThreadCheck();                                                   \
  }                                                                                                          \
  void incrementCheckedPtrCount() const final                                                                \
  {                                                                                                          \
    BaseClass::incrementCheckedPtrCount();                                                                   \
  }                                                                                                          \
  void decrementCheckedPtrCount() const final                                                                \
  {                                                                                                          \
    BaseClass::decrementCheckedPtrCount();                                                                   \
  }                                                                                                          \
  void setDidBeginCheckedPtrDeletion() final                                                                 \
  {                                                                                                          \
    BaseClass::setDidBeginCheckedPtrDeletion();                                                              \
  }                                                                                                          \
  using __unused_for_semicolon_canmakecheckedptr = int
