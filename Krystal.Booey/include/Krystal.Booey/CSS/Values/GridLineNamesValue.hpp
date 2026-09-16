#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::css
{
  class GridLineNamesValue final : public Value
  {
  public:
    KRYS_NODISCARD static RefPtr<GridLineNamesValue> Create(Span<const CSSOMString> names) noexcept;

    KRYS_NODISCARD Span<const CSSOMString> Names() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::GridLineNamesValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsGridLineNamesValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()