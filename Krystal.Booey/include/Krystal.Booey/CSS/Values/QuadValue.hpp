#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class QuadValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<QuadValue> Create(Ref<Value> value) noexcept;

    KRYS_NODISCARD static Ref<QuadValue> Create(Ref<Value> first, Ref<Value> second) noexcept;

    KRYS_NODISCARD static Ref<QuadValue> Create(Ref<Value> first, Ref<Value> second,
                                                Ref<Value> third) noexcept;

    KRYS_NODISCARD static Ref<QuadValue> Create(Ref<Value> first, Ref<Value> second, Ref<Value> third,
                                                Ref<Value> fourth) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::QuadValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsQuadValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()