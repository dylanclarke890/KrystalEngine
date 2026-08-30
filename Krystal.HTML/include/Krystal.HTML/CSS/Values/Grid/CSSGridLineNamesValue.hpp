#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Span.hpp"

namespace Krys::HTML
{
  class CSSGridLineNamesValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static RefPtr<CSSGridLineNamesValue> Create(Span<const CSSOMString> names) noexcept;

    KRYS_NODISCARD Span<const CSSOMString> Names() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSGridLineNamesValue)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSGridLineNamesValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()