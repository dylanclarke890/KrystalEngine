#pragma once

#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Types/IntrusivePtr.hpp"

namespace krys
{
  /// @brief CheckedPtr is used to verify that the object being pointed to outlives the CheckedPtr.
  /// It does not affect the lifetime of the object being pointed to; it simply adds a runtime
  /// check (via krys_assert) that when the object being pointed to is destroyed, there are
  /// no outstanding CheckedPtrs that reference it.
  /// @note Use is similar to WeakPtr, but CheckedPtr has less overhead and is used in cases where the
  /// target is never expected to become null.
  template <typename T>
  struct CheckedRefPolicy
  {
    KRYS_ALWAYS_INLINE constexpr static T *AddRef(T *ptr) noexcept
    {
      static_assert(IsTypeComplete<T>, "T is an incomplete type.");

      if (ptr) KRYS_LIKELY
      {
        ptr->AddRefChecked();
      }
      return ptr;
    }

    KRYS_ALWAYS_INLINE constexpr static T &AddRef(T &ref) noexcept
    {
      static_assert(IsTypeComplete<T>, "T is an incomplete type.");

      ref.AddRefChecked();
      return ref;
    }

    KRYS_ALWAYS_INLINE constexpr static void SubRef(T *ptr) noexcept
    {
      static_assert(IsTypeComplete<T>, "T is an incomplete type.");

      if (ptr) KRYS_LIKELY
      {
        ptr->SubRefChecked();
      }
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static T *ValidateGetAccess(T *ptr) noexcept
    {
      static_assert(IsTypeComplete<T>, "T is an incomplete type.");

      if (ptr && !ptr->GetRefCountChecked())
      {
        krys_assert_msg(false, "Attempting to access an object through a CheckedPtr that has been deleted");
        std::terminate();
      }

      return ptr;
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static bool IsValid(T *ptr) noexcept
    {
      static_assert(IsTypeComplete<T>, "T is an incomplete type.");

      // Note that we only check for nullptr here and not ptr->GetRefCountChecked() == 0 because we want to
      // allow nullptr.
      return ptr != nullptr;
    }
  };

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using CheckedRef = IntrusivePtr<T, PtrTraits, CheckedRefPolicy<T>, IsNullable(false)>;

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using CheckedPtr = IntrusivePtr<T, PtrTraits, CheckedRefPolicy<T>, IsNullable(true)>;

  template <typename T, typename PtrTraits = RawPtrTraits<T>, typename... Args>
  KRYS_NODISCARD constexpr CheckedRef<T, PtrTraits>
    CreateCheckedRef(Args &&...args) noexcept(NoThrowConstructible<T, Args...>)
  {
    T *ptr = new T(std::forward<Args>(args)...);
    return CheckedRef<T, PtrTraits>::NoRef(*ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>, typename... Args>
  KRYS_NODISCARD constexpr CheckedPtr<T, PtrTraits>
    CreateCheckedPtr(Args &&...args) noexcept(NoThrowConstructible<T, Args...>)
  {
    T *ptr = new T(std::forward<Args>(args)...);
    return CheckedPtr<T, PtrTraits>::NoRef(*ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr CheckedRef<T, PtrTraits> AdoptCheckedRef(T &ptr) noexcept
  {
    return CheckedRef<T, PtrTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr CheckedPtr<T, PtrTraits> AdoptCheckedPtr(T *ptr) noexcept
  {
    return CheckedPtr<T, PtrTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr CheckedRef<T, PtrTraits> ShareCheckedRef(T &ptr) noexcept
  {
    return CheckedRef<T, PtrTraits>::WithRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr CheckedPtr<T, PtrTraits> ShareCheckedPtr(T *ptr) noexcept
  {
    return CheckedPtr<T, PtrTraits>::WithRef(ptr);
  }
}
