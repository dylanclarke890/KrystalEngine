#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSRatio.hpp"

namespace Krys::HTML
{
  class CSSRatioValue : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSRatioValue> Create(Ratio ratio) noexcept;
  };
}