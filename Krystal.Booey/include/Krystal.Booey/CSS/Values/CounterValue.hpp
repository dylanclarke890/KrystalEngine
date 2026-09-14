#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CounterValue : public Value
  {
  public:
    KRYS_NODISCARD static Ref<CounterValue> Create(CSSOMStringAtom identifier, CSSOMStringAtom separator,
                                                   Ref<Value> counterStyle) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CounterValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsCounterValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()