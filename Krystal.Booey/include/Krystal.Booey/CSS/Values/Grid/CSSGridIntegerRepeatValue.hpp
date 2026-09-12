#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Grid/GridTemplateAreas.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSGridIntegerRepeatValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSGridIntegerRepeatValue> Create(Ref<CSSPrimitiveValue> repetitions,
                                                             CSSValueListBuilder builder) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSGridIntegerRepeatValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSGridIntegerRepeatValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()