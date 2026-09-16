#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class AttrValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<AttrValue> Create(CSSOMStringAtom attr, RefPtr<Value> fallback) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::AttrValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsAttrValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()