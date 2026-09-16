#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class PrimitiveValue;

  class GridLineValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<GridLineValue> Create(RefPtr<PrimitiveValue> spanValue,
                                                    RefPtr<PrimitiveValue> numericValue,
                                                    RefPtr<PrimitiveValue> gridLineName) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::GridLineValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsGridLineValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()