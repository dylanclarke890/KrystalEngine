#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Debug.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include "Krystal.Core/Utils/ByteUtils.hpp"
#include <atomic>
#include <exception>

namespace krys::detail
{
  template <typename TCount, bool ThreadSafe>
  class CanMakeCheckedPtrBase
  {
  public:
    using deletion_flag_type = conditional_t<ThreadSafe, std::atomic<bool>, bool>;
    using storage_type = conditional_t<ThreadSafe, std::atomic<TCount>, TCount>;

  private:
    mutable storage_type _checkedPtrCount {0};
#if KRYS_DEBUG
    deletion_flag_type _didBeginDeletion {false};
#endif

  public:
    CanMakeCheckedPtrBase() noexcept = default;

    CanMakeCheckedPtrBase(const CanMakeCheckedPtrBase &) noexcept
    {
    }

    CanMakeCheckedPtrBase(CanMakeCheckedPtrBase &&) noexcept
    {
    }

    ~CanMakeCheckedPtrBase() noexcept
    {
      krys_assert(_didBeginDeletion);
    }

    CanMakeCheckedPtrBase &operator=(const CanMakeCheckedPtrBase &) noexcept
    {
      return *this;
    }

    CanMakeCheckedPtrBase &operator=(CanMakeCheckedPtrBase &&) noexcept
    {
      return *this;
    }

    KRYS_ALWAYS_INLINE void AddRefChecked() const noexcept
    {
      ++_checkedPtrCount;
    }

    KRYS_ALWAYS_INLINE void SubRefChecked() const noexcept
    {
      // In normal execution, a CheckedPtr always points to an object with a non-zero GetRefCountChecked().
      // When it detects a dangling pointer, KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR scribbles an object with
      // zeroes and then leaks it. When we check GetRefCountChecked() here, we're checking for a scribbled
      // object.
      if (!GetRefCountChecked()) KRYS_UNLIKELY
      {
        CrashDueToCheckedPtrToDeadObject();
      }
      --_checkedPtrCount;
    }

    KRYS_NODISCARD TCount GetRefCountChecked() const noexcept
    {
      return _checkedPtrCount;
    }

    void SetDidBeginCheckedPtrDeletion() noexcept
    {
#if KRYS_DEBUG
      _didBeginDeletion = true;
#endif
    }

  private:
    KRYS_NORETURN KRYS_NEVER_INLINE static void CrashDueToCheckedPtrToDeadObject() noexcept
    {
      std::terminate();
    }
  };

  template <typename T, bool ThreadSafe, typename TCount = uint32>
  class CanMakeCheckedPtr : public CanMakeCheckedPtrBase<uint32, ThreadSafe>
  {
  public:
    ~CanMakeCheckedPtr() noexcept
    {
      static_assert(T::_delete_overridden_for_checkedptr,
                    "Objects that use CanMakeCheckedPtr must use KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR");
    }
  };
}

/// @brief delete(T*, std::destroying_delete_t, size_t) is preferred over delete(void*)
/// in overload resolution, so we can use it to interpose before calling delete(void*).
/// @note KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR must be declared in every subclass.
#define KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR_IMPL(T)                                                         \
  void operator delete(T *object, std::destroying_delete_t, size_t size)                                     \
  {                                                                                                          \
    krys_assert("invalid size" && sizeof(T) == size);                                                        \
                                                                                                             \
    object->SetDidBeginCheckedPtrDeletion();                                                                 \
                                                                                                             \
    /* Run destructor manually */                                                                            \
    object->T::~T();                                                                                         \
                                                                                                             \
    /* If CheckedPtrs still exist, poison and keep memory */                                                 \
    if (object->GetRefCountChecked()) KRYS_UNLIKELY                                                          \
    {                                                                                                        \
      ::krys::ByteUtils::ZeroObject(*object);                                                                \
      return;                                                                                                \
    }                                                                                                        \
                                                                                                             \
    /* Free memory WITHOUT re-entering delete */                                                             \
    ::operator delete(static_cast<void *>(object));                                                          \
  }                                                                                                          \
  constexpr static bool _delete_overridden_for_checkedptr = true;

/// @note KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR must be declared in the most derived subclass.
#define KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ClassName)                                                      \
public:                                                                                                      \
  KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR_IMPL(ClassName)                                                       \
private:                                                                                                     \
  using _forceSemicolonAfterKrysOverrideDeleteCheckedPtr = int

#define KRYS_STRUCT_OVERRIDE_DELETE_FOR_CHECKED_PTR(ClassName)                                               \
  KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR_IMPL(ClassName)                                                       \
  using _forceSemicolonKrysOverrideDeleteCheckedPtr = int

namespace krys
{
  template <typename T>
  using CanMakeCheckedPtr = ::krys::detail::CanMakeCheckedPtr<T, false>;

  template <typename T>
  using CanMakeThreadSafeCheckedPtr = ::krys::detail::CanMakeCheckedPtr<T, true>;

  /// @brief Use this class when an abstract base class needs CheckedPtr/CheckedRef support, and the
  /// CanMakeCheckedPtr implementation will be in a concrete subclass.
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

#define KRYS_USING_CAN_MAKE_CHECKEDPTR(Base)                                                                 \
  using Base::AddRefChecked;                                                                                 \
  using Base::SubRefChecked;                                                                                 \
  using Base::GetRefCountChecked;

#define KRYS_ABSTRACT_CAN_MAKE_CHECKEDPTR_OVERRIDE(Base)                                                     \
  void AddRefChecked() const noexcept final                                                                  \
  {                                                                                                          \
    Base::AddRefChecked();                                                                                   \
  }                                                                                                          \
  void SubRefChecked() const noexcept final                                                                  \
  {                                                                                                          \
    Base::SubRefChecked();                                                                                   \
  }                                                                                                          \
  KRYS_NODISCARD krys::uint32 GetRefCountChecked() const noexcept final                                      \
  {                                                                                                          \
    return Base::GetRefCountChecked();                                                                       \
  }