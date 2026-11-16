#pragma once

#include "Krystal.Core/Types/IntrusivePtr.hpp"

namespace krys
{
  template <typename T>
  struct InlineWeakRefPolicy
  {
    KRYS_ALWAYS_INLINE static T *AddRef(T *ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->AddRefWeak();
      }
      return ptr;
    }

    KRYS_ALWAYS_INLINE static T &AddRef(T &ref) noexcept
    {
      ref.AddRefWeak();
      return ref;
    }

    KRYS_ALWAYS_INLINE static void SubRef(T *ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->SubRefWeak();
      }
    }

    KRYS_NODISCARD KRYS_ALWAYS_INLINE static T *ValidateGetAccess(T *ptr) noexcept
    {
      if (!ptr || !ptr->GetRefCount())
      {
        return nullptr;
      }

      return ptr;
    }

    KRYS_NODISCARD KRYS_ALWAYS_INLINE static bool IsValid(T *ptr) noexcept
    {
      return ptr && ptr->GetRefCount();
    }
  };

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using InlineWeakRef = IntrusivePtr<T, PtrTraits, InlineWeakRefPolicy<T>, IsNullable::No>;

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using InlineWeakPtr = IntrusivePtr<T, PtrTraits, InlineWeakRefPolicy<T>, IsNullable::Yes>;
}
