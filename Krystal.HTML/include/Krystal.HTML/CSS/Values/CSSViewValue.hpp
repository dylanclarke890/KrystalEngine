#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSViewValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSViewValue> Create() noexcept;

    KRYS_NODISCARD static Ref<CSSViewValue> Create(Ref<CSSValue> axis, Ref<CSSValue> startInset,
                                                   Ref<CSSValue> endInset) noexcept;
  };
}