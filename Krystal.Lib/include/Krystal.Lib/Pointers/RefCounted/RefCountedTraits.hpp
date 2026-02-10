#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/Concepts.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys
{
  /// @brief Default traits for reference counting.
  template <IntrusiveRefCounted T>
  struct DefaultRefCountedTraits
  {
    KRYS_ALWAYS_INLINE constexpr static RawPtr<T> AddRef(RawPtr<T> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->AddRef();
      }
      return ptr;
    }

    KRYS_ALWAYS_INLINE constexpr static T &AddRef(T &ref) noexcept
    {
      ref.AddRef();
      return ref;
    }

    KRYS_ALWAYS_INLINE constexpr static void SubRef(RawPtr<T> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->SubRef();
      }
    }
  };
}