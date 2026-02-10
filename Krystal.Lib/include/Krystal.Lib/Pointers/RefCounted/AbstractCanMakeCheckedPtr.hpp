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
    virtual void AddRefChecked() const noexcept = 0;
    virtual void SubRefChecked() const noexcept = 0;
    KRYS_NODISCARD virtual uint32 GetRefCountChecked() const noexcept = 0;
  };
}

#define KRYS_OVERRIDE_ABSTRACT_CAN_MAKE_CHECKEDPTR(Base)                                                     \
  void AddRefChecked() const noexcept final                                                                  \
  {                                                                                                          \
    Base::AddRefChecked();                                                                                   \
  }                                                                                                          \
  void SubRefChecked() const noexcept final                                                                  \
  {                                                                                                          \
    Base::SubRefChecked();                                                                                   \
  }                                                                                                          \
  KRYS_NODISCARD Krys::uint32 GetRefCountChecked() const noexcept final                                      \
  {                                                                                                          \
    return Base::GetRefCountChecked();                                                                       \
  }

#define KRYS_USING_CAN_MAKE_CHECKEDPTR(Base)                                                                 \
  using Base::AddRefChecked;                                                                                 \
  using Base::SubRefChecked;                                                                                 \
  using Base::GetRefCountChecked;
