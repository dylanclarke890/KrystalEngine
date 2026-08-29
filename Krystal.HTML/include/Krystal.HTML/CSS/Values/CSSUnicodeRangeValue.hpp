#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSUnicodeRangeValue final : public CSSValue
  {
  public:
    KRYS_NODISCARD static Ref<CSSUnicodeRangeValue> Create(char32 start, char32 end) noexcept;
  };
}