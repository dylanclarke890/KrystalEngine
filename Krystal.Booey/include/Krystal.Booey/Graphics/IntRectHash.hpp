#pragma once

#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Core/Hash.hpp"

namespace krys
{
  template <>
  struct DefaultHash<boo::IntRect>
  {
    KRYS_NODISCARD static size_t Hash(const boo::IntRect &key) noexcept
    {
      return krys::Hash::Combine(key.X(), key.Y(), key.Width(), key.Height());
    }

    KRYS_NODISCARD static bool Equal(const boo::IntRect &a, const boo::IntRect &b) noexcept
    {
      return a == b;
    }

    constexpr static bool IsSafeToCompareToEmptyOrDeleted = true;
  };

  template <>
  struct HashTraits<boo::IntRect> : public GenericHashTraits<boo::IntRect>
  {
    constexpr static bool EmptyValueIsZero = true;

    constexpr static void ConstructDeletedValue(boo::IntRect &slot) noexcept
    {
      slot = boo::IntRect(-1, -1, -1, -1);
    }

    KRYS_NODISCARD constexpr static bool IsDeletedValue(const boo::IntRect &value) noexcept
    {
      return value.X() == -1 && value.Y() == -1 && value.Width() == -1 && value.Height() == -1;
    }
  };
}
