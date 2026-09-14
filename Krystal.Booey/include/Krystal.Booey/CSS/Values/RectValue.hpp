#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Rect.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  class RectValue : public Value
  {
  private:
    Rect _rect;

    explicit RectValue(Rect rect) noexcept;

  public:
    KRYS_NODISCARD static Ref<RectValue> Create(Rect rect) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::RectValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsRectValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()