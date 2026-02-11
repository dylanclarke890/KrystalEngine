#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Pointers/IntrusivePtr.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys
{
  template <typename T>
  concept SupportsCheckedPtr = requires(T &value) {
    { value.AddRefChecked() } -> SameType<void>;
    { value.SubRefChecked() } -> SameType<void>;
    { value.CheckedPtrCount() } -> ConvertibleTo<uint32>;
  };

  /// @brief CheckedPtr is used to verify that the object being pointed to outlives the CheckedPtr.
  /// It does not affect the lifetime of the object being pointed to; it simply adds a runtime
  /// check (via assert) that when the object being pointed to is destroyed, there are
  /// no outstanding CheckedPtrs that reference it.
  /// @note Use is similar to WeakPtr, but CheckedPtr has less overhead and is used in cases where the
  /// target is never expected to become null.
  template <typename T>
  struct CheckedRefPolicy
  {
    KRYS_ALWAYS_INLINE constexpr static RawPtr<T> AddRef(RawPtr<T> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->AddRefChecked();
      }
      return ptr;
    }

    KRYS_ALWAYS_INLINE constexpr static T &AddRef(T &ref) noexcept
    {
      ref.AddRefChecked();
      return ref;
    }

    KRYS_ALWAYS_INLINE constexpr static void SubRef(RawPtr<T> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->SubRefChecked();
      }
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static RawPtr<T> ValidateGetAccess(RawPtr<T> ptr) noexcept
    {
      if (ptr && !ptr->CheckedPtrCount())
      {
        assert(false && "Attempting to access an object through a CheckedPtr that has been deleted");
        std::terminate();
      }

      return ptr;
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static bool IsValid(RawPtr<T> ptr) noexcept
    {
      // Note that we're considering a CheckedPtr to be valid as long as it points to an object, even if that
      // object has been deleted. The runtime checks in ValidateGetAccess will catch attempts to access
      // deleted objects.
      return ptr != nullptr;
    }
  };

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using CheckedRef = IntrusivePtr<T, PtrTraits, CheckedRefPolicy<T>, IsNullable(false)>;

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using CheckedPtr = IntrusivePtr<T, PtrTraits, CheckedRefPolicy<T>, IsNullable(true)>;

  template <typename T, typename PtrTraits = RawPtrTraits<T>, typename... Args>
  KRYS_NODISCARD constexpr inline CheckedRef<T, PtrTraits>
    CreateCheckedRef(Args &&...args) noexcept(NoThrowConstructible<T, Args...>)
  {
    RawPtr<T> ptr = new T(std::forward<Args>(args)...);
    return CheckedRef<T, PtrTraits>::NoRef(*ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>, typename... Args>
  KRYS_NODISCARD constexpr inline CheckedPtr<T, PtrTraits>
    CreateCheckedPtr(Args &&...args) noexcept(NoThrowConstructible<T, Args...>)
  {
    RawPtr<T> ptr = new T(std::forward<Args>(args)...);
    return CheckedPtr<T, PtrTraits>::NoRef(*ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedRef<T, PtrTraits> AdoptCheckedRef(T &ptr) noexcept
  {
    return CheckedRef<T, PtrTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedPtr<T, PtrTraits> AdoptCheckedPtr(RawPtr<T> ptr) noexcept
  {
    return CheckedPtr<T, PtrTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedRef<T, PtrTraits> ShareCheckedRef(T &ptr) noexcept
  {
    return CheckedRef<T, PtrTraits>::WithRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedPtr<T, PtrTraits> ShareCheckedPtr(RawPtr<T> ptr) noexcept
  {
    return CheckedPtr<T, PtrTraits>::WithRef(ptr);
  }
}
