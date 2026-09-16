#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class UnicodeRangeValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<UnicodeRangeValue> Create(char32 start, char32 end) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::UnicodeRangeValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsUnicodeRangeValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()