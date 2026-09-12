#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::css
{
  class CSSGridLineNamesValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static RefPtr<CSSGridLineNamesValue> Create(Span<const CSSOMString> names) noexcept;

    KRYS_NODISCARD Span<const CSSOMString> Names() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSGridLineNamesValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSGridLineNamesValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()