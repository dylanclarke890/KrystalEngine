#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSScrollValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSScrollValue> Create(RefPtr<CSSValue> scroller, RefPtr<CSSValue> axis) noexcept;
  };
}