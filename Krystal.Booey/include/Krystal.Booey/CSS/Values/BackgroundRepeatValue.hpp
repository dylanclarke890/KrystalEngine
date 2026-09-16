#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Quad.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class BackgroundRepeatValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<BackgroundRepeatValue> Create(css::ValueId repeatX,
                                                            css::ValueId repeatY) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::BackgroundRepeatValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsBackgroundRepeatValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()