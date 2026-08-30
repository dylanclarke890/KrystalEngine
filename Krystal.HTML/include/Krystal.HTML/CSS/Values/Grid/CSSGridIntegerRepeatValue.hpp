#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/Grid/GridTemplateAreas.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSGridIntegerRepeatValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSGridIntegerRepeatValue> Create(Ref<CSSPrimitiveValue> repetitions,
                                                             CSSValueListBuilder builder) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSGridIntegerRepeatValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSGridIntegerRepeatValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()