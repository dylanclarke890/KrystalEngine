#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Motion/RayFunction.hpp"

namespace Krys::HTML
{
  class CSSRayValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSRayValue> Create(RayFunction ray) noexcept;
  };
}