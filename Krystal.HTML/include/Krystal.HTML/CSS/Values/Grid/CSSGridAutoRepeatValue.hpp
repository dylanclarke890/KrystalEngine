#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.HTML/CSS/Values/Grid/GridTemplateAreas.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSGridAutoRepeatValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSGridAutoRepeatValue> Create(CSSValueId id,
                                                             CSSValueListBuilder builder) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSGridAutoRepeatValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSGridAutoRepeatValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()