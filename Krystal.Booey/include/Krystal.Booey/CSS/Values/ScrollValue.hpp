#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class ScrollValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<ScrollValue> Create(RefPtr<Value> scroller, RefPtr<Value> axis) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ScrollValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsScrollValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()