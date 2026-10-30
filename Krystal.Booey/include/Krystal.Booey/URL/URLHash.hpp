#pragma once

#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Utils/Tags.hpp"
#include <memory>

namespace krys
{
  template <>
  struct DefaultHash<boo::url::URL>
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = false;

    KRYS_NODISCARD static size_t Hash(const boo::url::URL &key) noexcept
    {
      return std::hash<utf8_string> {}(key.String());
    }

    KRYS_NODISCARD static bool Equal(const boo::url::URL &a, const boo::url::URL &b) noexcept
    {
      return a.String() == b.String();
    }
  };

  template <>
  struct HashTraits<boo::url::URL> : public GenericHashTraits<boo::url::URL>
  {
    static void ConstructDeletedValue(boo::url::URL &slot) noexcept
    {
      new (NotNullTag {}, std::addressof(slot)) boo::url::URL(HashTableDeletedValueTag {});
    }

    KRYS_NODISCARD static bool IsDeletedValue(const boo::url::URL &value) noexcept
    {
      return value.IsHashTableDeletedValue();
    }
  };
}
