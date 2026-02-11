#pragma once

#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Pointers/RawPtrTraits.hpp"
#include "Krystal.Lib/Pointers/RefCounted/IntrusiveHandle.hpp"

namespace Krys
{
  /// @brief CheckedPtr is used to verify that the object being pointed to outlives the CheckedPtr.
  /// It does not affect the lifetime of the object being pointed to; it simply adds a runtime
  /// check (via assert) that when the object being pointed to is destroyed, there are
  /// no outstanding CheckedPtrs that reference it.
  /// @note Use is similar to WeakPtr, but CheckedPtr has less overhead and is used in cases where the target
  /// is never expected to become null.
  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using CheckedPtr = IntrusiveHandle<T, PtrTraits, CheckedPolicy<T>, CheckedAccess<T>, IsNullable(true)>;

  template <typename T, typename PtrTraits = RawPtrTraits<T>, typename... Args>
  KRYS_NODISCARD constexpr inline CheckedPtr<T, PtrTraits> CreateCheckedPtr(Args &&...args)
  {
    RawPtr<T> ptr = new T(std::forward<Args>(args)...);
    return CheckedPtr<T, PtrTraits>::NoRef(*ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedPtr<T, PtrTraits> AdoptCheckedPtr(T *ptr) noexcept
  {
    return CheckedPtr<T, PtrTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedPtr<T, PtrTraits> ShareCheckedPtr(T *ptr) noexcept
  {
    return CheckedPtr<T, PtrTraits>::WithRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using CheckedRef = IntrusiveHandle<T, PtrTraits, CheckedPolicy<T>, CheckedAccess<T>, IsNullable(false)>;

  template <typename T, typename PtrTraits = RawPtrTraits<T>, typename... Args>
  KRYS_NODISCARD constexpr inline CheckedRef<T, PtrTraits> CreateCheckedRef(Args &&...args)
  {
    RawPtr<T> ptr = new T(std::forward<Args>(args)...);
    return CheckedRef<T, PtrTraits>::NoRef(*ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedRef<T, PtrTraits> AdoptCheckedRef(T &ptr) noexcept
  {
    return CheckedRef<T, PtrTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedRef<T, PtrTraits> ShareCheckedRef(T &ptr) noexcept
  {
    return CheckedRef<T, PtrTraits>::WithRef(ptr);
  }
}
