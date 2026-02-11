#pragma once

#include "Krystal.Lib/Pointers/IntrusivePtr.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys
{
  template <typename T>
  struct InlineWeakRefPolicy
  {
    KRYS_ALWAYS_INLINE constexpr static RawPtr<T> AddRef(RawPtr<T> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->AddRefWeak();
      }
      return ptr;
    }

    KRYS_ALWAYS_INLINE constexpr static T &AddRef(T &ref) noexcept
    {
      ref.AddRefWeak();
      return ref;
    }

    KRYS_ALWAYS_INLINE constexpr static void SubRef(RawPtr<T> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->SubRefWeak();
      }
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static RawPtr<T> ValidateGetAccess(RawPtr<T> ptr) noexcept
    {
      if (!ptr || !ptr->GetRefCount())
      {
        return nullptr;
      }

      return ptr;
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static bool IsValid(RawPtr<T> ptr) noexcept
    {
      return ptr && ptr->GetRefCount();
    }
  };

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using InlineWeakRef = IntrusivePtr<T, PtrTraits, InlineWeakRefPolicy<T>, IsNullable(false)>;

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using InlineWeakPtr = IntrusivePtr<T, PtrTraits, InlineWeakRefPolicy<T>, IsNullable(true)>;
}
