#pragma once

#include "Krystal.Core/Types/IntrusivePtr.hpp"

namespace krys
{
  template <typename T>
  struct InlineWeakRefPolicy
  {
    KRYS_ALWAYS_INLINE constexpr static T *AddRef(T *ptr) noexcept
    {
      static_assert(IsTypeComplete<T>, "T is an incomplete type.");

      if (ptr) KRYS_LIKELY
      {
        ptr->AddRefWeak();
      }
      return ptr;
    }

    KRYS_ALWAYS_INLINE constexpr static T &AddRef(T &ref) noexcept
    {
      static_assert(IsTypeComplete<T>, "T is an incomplete type.");

      ref.AddRefWeak();
      return ref;
    }

    KRYS_ALWAYS_INLINE constexpr static void SubRef(T *ptr) noexcept
    {
      static_assert(IsTypeComplete<T>, "T is an incomplete type.");

      if (ptr) KRYS_LIKELY
      {
        ptr->SubRefWeak();
      }
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static T *ValidateGetAccess(T *ptr) noexcept
    {
      static_assert(IsTypeComplete<T>, "T is an incomplete type.");

      if (!ptr || !ptr->GetRefCount())
      {
        return nullptr;
      }

      return ptr;
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static bool IsValid(T *ptr) noexcept
    {
      static_assert(IsTypeComplete<T>, "T is an incomplete type.");

      return ptr && ptr->GetRefCount();
    }
  };

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using InlineWeakRef = IntrusivePtr<T, PtrTraits, InlineWeakRefPolicy<T>, IsNullable(false)>;

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using InlineWeakPtr = IntrusivePtr<T, PtrTraits, InlineWeakRefPolicy<T>, IsNullable(true)>;
}
