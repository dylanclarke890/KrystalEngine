#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Quad.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class ReflectValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<ReflectValue> Create(css::ValueId direction, Ref<Value> offset,
                                                   RefPtr<Value> mask) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ReflectValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsReflectValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()