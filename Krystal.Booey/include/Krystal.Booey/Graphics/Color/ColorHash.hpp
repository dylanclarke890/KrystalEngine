#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Utils/Tags.hpp"

namespace krys
{
  template <>
  struct HashTraits<krys::boo::Color> : public GenericHashTraits<krys::boo::Color>
  {
    constexpr static bool HasIsEmptyValueFunction = true;
    constexpr static bool EmptyValueIsZero = false;

    KRYS_NODISCARD static krys::boo::Color EmptyValue() noexcept
    {
      return krys::boo::Color(HashTableEmptyValueTag {});
    }

    KRYS_NODISCARD static bool IsEmptyValue(const krys::boo::Color &value) noexcept
    {
      return value.IsHashTableEmptyValue();
    }

    static void ConstructDeletedValue(krys::boo::Color &slot) noexcept
    {
      new (NotNullTag {}, &slot) krys::boo::Color(HashTableDeletedValueTag {});
    }

    KRYS_NODISCARD static bool IsDeletedValue(const krys::boo::Color &value) noexcept
    {
      return value.IsHashTableDeletedValue();
    }
  };
}
