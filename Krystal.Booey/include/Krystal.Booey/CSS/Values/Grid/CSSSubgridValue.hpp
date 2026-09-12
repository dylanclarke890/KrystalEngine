#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.Booey/CSS/Values/Grid/GridTemplateAreas.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSSubgridValue final : public CSSValueContainingVector
  {
  public:
    KRYS_NODISCARD static Ref<CSSSubgridValue> Create(CSSValueListBuilder builder) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSSubgridValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSSubgridValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()