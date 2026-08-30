#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSAttrValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSAttrValue> Create(CSSOMStringAtom attr, RefPtr<CSSValue> fallback) noexcept;
  };
}