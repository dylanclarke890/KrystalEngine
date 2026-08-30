#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.HTML/CSS/Values/Grid/GridTemplateAreas.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSSubgridValue final : public CSSValueContainingVector
  {
  public:
    KRYS_NODISCARD static Ref<CSSSubgridValue> Create(CSSValueListBuilder builder) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSSubgridValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSSubgridValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()