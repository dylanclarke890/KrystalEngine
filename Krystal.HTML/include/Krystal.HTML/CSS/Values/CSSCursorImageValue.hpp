#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSCursorImageValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSCursorImageValue> Create(Ref<CSSValue> &&image,
                                                          Ref<CSSValue> &&hotSpot) noexcept;
  };
}