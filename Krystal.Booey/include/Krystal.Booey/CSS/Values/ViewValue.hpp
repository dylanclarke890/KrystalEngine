#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class ViewValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<ViewValue> Create() noexcept;

    KRYS_NODISCARD static Ref<ViewValue> Create(Ref<Value> axis, Ref<Value> startInset,
                                                Ref<Value> endInset) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ViewValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsViewValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()