#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CursorImageValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<CursorImageValue> Create(Ref<Value> &&image, Ref<Value> &&hotSpot) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CursorImageValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsCursorImageValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()