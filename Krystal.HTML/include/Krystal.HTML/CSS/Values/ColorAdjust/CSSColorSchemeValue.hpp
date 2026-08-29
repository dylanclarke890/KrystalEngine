#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.HTML/CSS/Values/ColorAdjust/ColorScheme.hpp"

namespace Krys::HTML
{
  class CSSColorSchemeValue : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSColorSchemeValue> Create(ColorScheme scheme) noexcept;
  };
}