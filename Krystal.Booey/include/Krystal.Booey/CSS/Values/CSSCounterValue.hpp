#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSCounterValue : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSCounterValue> Create(CSSOMStringAtom identifier, CSSOMStringAtom separator,
                                                      Ref<CSSValue> counterStyle) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSCounterValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSCounterValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()